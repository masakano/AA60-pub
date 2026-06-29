#!/usr/bin/env python3
from __future__ import annotations

import argparse
import asyncio
import fractions
import mmap
import os
import struct
import time
from contextlib import asynccontextmanager
from pathlib import Path
from typing import Any

import av
import numpy as np
from aiortc import RTCPeerConnection, RTCSessionDescription, VideoStreamTrack
from aiortc.contrib.media import MediaRelay
from aiortc.mediastreams import MediaStreamError
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

from inference import inference_process


class Offer(BaseModel):
    sdp: str
    type: str


class ControlRequest(BaseModel):
    enable: bool | None = None
    fps: int | None = None
    flags: int | None = None


class RgbaFrameMixin:
    m_width: int
    m_height: int
    m_flipY: bool

    def makeVideoFrame(self, pixels: bytes):
        frame = av.VideoFrame(self.m_width, self.m_height, "rgba")
        plane = frame.planes[0]
        row_size = self.m_width * 4
        if not self.m_flipY and plane.line_size == row_size:
            plane.update(pixels)
        else:
            padded = bytearray(plane.buffer_size)
            for y in range(self.m_height):
                src_y = self.m_height - 1 - y if self.m_flipY else y
                src = src_y * row_size
                dst = y * plane.line_size
                padded[dst:dst + row_size] = pixels[src:src + row_size]
            plane.update(padded)
        return frame.reformat(format="yuv420p")


class SharedMemoryTrack(RgbaFrameMixin, VideoStreamTrack):
    kind = "video"

    c_count_offset = 0
    c_control_offset = 16
    c_header_size = 32
    c_time_base = fractions.Fraction(1, 90000)

    def __init__(
        self,
        path: Path,
        width: int,
        height: int,
        ai_width: int,
        ai_height: int,
        poll_interval: float = 0.001,
        flip_y: bool = True,
    ):
        super().__init__()
        self.m_path = path
        self.m_inputWidth = width
        self.m_inputHeight = height
        self.m_aiWidth = ai_width
        self.m_aiHeight = ai_height
        self.m_width = width + ai_width
        self.m_height = height
        self.m_flipY = flip_y
        self.m_pollInterval = poll_interval
        self.m_frameSize = width * height * 4
        self.m_aiFrameSize = ai_width * ai_height * 4
        self.m_outputFrameSize = self.m_width * self.m_height * 4
        self.m_mapSize = self.c_header_size + self.m_frameSize
        self.m_file = path.open("r+b", buffering=0)
        file_size = os.fstat(self.m_file.fileno()).st_size
        if file_size < self.m_mapSize:
            self.m_file.close()
            raise ValueError(f"{path} is too small: {file_size} < {self.m_mapSize}")
        self.m_map = mmap.mmap(self.m_file.fileno(), self.m_mapSize, access=mmap.ACCESS_WRITE)
        self.m_lastCount: int | None = None
        self.m_startTime = time.monotonic()
        self.m_lastPts = -1
        self.m_aiPixels = bytearray(self.m_aiFrameSize)
        self.m_outputPixels = bytearray(self.m_outputFrameSize)
        self.m_aiArray = np.frombuffer(self.m_aiPixels, dtype=np.uint8).reshape(ai_height, ai_width, 4)
        self.m_outputArray = np.frombuffer(self.m_outputPixels, dtype=np.uint8).reshape(self.m_height, self.m_width, 4)
        self.m_aiScale = self._makeAiScale(width, height, ai_width, ai_height)
        if self.m_aiScale is None:
            self.m_aiYIndices = (np.arange(ai_height) * height // ai_height).astype(np.intp)
            self.m_aiXIndices = (np.arange(ai_width) * width // ai_width).astype(np.intp)
        else:
            self.m_aiYIndices = None
            self.m_aiXIndices = None

    async def recv(self):
        while self.readyState == "live":
            count = self._readCount()
            if self.m_lastCount is None or count != self.m_lastCount:
                pixels, stable_count = self._readStablePixels(count)
                self.m_lastCount = stable_count
                frame = await asyncio.to_thread(self._processFrame, pixels, stable_count)
                pts = int((time.monotonic() - self.m_startTime) * 90000)
                if pts <= self.m_lastPts:
                    pts = self.m_lastPts + 1
                self.m_lastPts = pts
                frame.pts = pts
                frame.time_base = self.c_time_base
                return frame
            await asyncio.sleep(self.m_pollInterval)
        raise MediaStreamError

    def stop(self) -> None:
        if self.readyState == "ended":
            return
        super().stop()
        self.m_map.close()
        self.m_file.close()

    def _processFrame(self, pixels: bytes, count: int):
        input_array = np.frombuffer(pixels, dtype=np.uint8).reshape(self.m_inputHeight, self.m_inputWidth, 4)
        inference_process(input_array, count, self.m_aiArray, self.m_aiScale, self.m_aiYIndices, self.m_aiXIndices)
        self._composeOutput(input_array, self.m_aiArray)
        return self.makeVideoFrame(self.m_outputPixels)

    def _composeOutput(self, input_pixels: np.ndarray, ai_pixels: np.ndarray) -> None:
        self.m_outputArray[:, :self.m_inputWidth, :] = input_pixels
        self.m_outputArray[:self.m_aiHeight, self.m_inputWidth:self.m_inputWidth + self.m_aiWidth, :] = ai_pixels

    @staticmethod
    def _makeAiScale(width: int, height: int, ai_width: int, ai_height: int) -> int | None:
        if ai_width <= 0 or ai_height <= 0:
            return None
        if width % ai_width != 0 or height % ai_height != 0:
            return None
        x_scale = width // ai_width
        y_scale = height // ai_height
        if x_scale != y_scale:
            return None
        return x_scale

    def readControl(self) -> dict[str, int | bool]:
        serial, enable, fps, flags = struct.unpack_from("<IIII", self.m_map, self.c_control_offset)
        return {"serial": serial, "enable": bool(enable), "fps": fps, "flags": flags}

    def updateControl(self, request: ControlRequest) -> dict[str, int | bool]:
        serial, enable, fps, flags = struct.unpack_from("<IIII", self.m_map, self.c_control_offset)
        if request.enable is not None:
            enable = 1 if request.enable else 0
        if request.fps is not None:
            fps = max(1, min(240, int(request.fps)))
        if request.flags is not None:
            flags = int(request.flags) & 0xFFFFFFFF
        next_serial = (serial + 1) & 0xFFFFFFFF
        struct.pack_into("<III", self.m_map, self.c_control_offset + 4, enable, fps, flags)
        struct.pack_into("<I", self.m_map, self.c_control_offset, next_serial)
        self.m_map.flush()
        return {"serial": next_serial, "enable": bool(enable), "fps": fps, "flags": flags}

    def _readCount(self) -> int:
        return struct.unpack_from("<I", self.m_map, self.c_count_offset)[0]

    def _readStablePixels(self, count: int) -> tuple[bytes, int]:
        pixel_offset = self.c_header_size
        for _ in range(4):
            pixels = bytes(self.m_map[pixel_offset:pixel_offset + self.m_frameSize])
            next_count = self._readCount()
            if next_count == count:
                return pixels, count
            count = next_count
        return bytes(self.m_map[pixel_offset:pixel_offset + self.m_frameSize]), self._readCount()


class SourceHub:
    def __init__(self, args: argparse.Namespace):
        self.m_args = args
        self.m_relay = MediaRelay()
        self.m_track: SharedMemoryTrack | None = None

    def readControl(self) -> dict[str, int | bool]:
        if self.m_track is not None and self.m_track.readyState == "live":
            return self.m_track.readControl()
        return self._withControlMap(lambda control_map: self._readControlMap(control_map))

    def updateControl(self, request: ControlRequest) -> dict[str, int | bool]:
        if self.m_track is not None and self.m_track.readyState == "live":
            return self.m_track.updateControl(request)
        return self._withControlMap(lambda control_map: self._updateControlMap(control_map, request))

    def subscribe(self) -> VideoStreamTrack:
        track = self._ensureTrack()
        return self.m_relay.subscribe(track, buffered=False)

    def close(self) -> None:
        if self.m_track is not None:
            self.m_track.stop()
            self.m_track = None
            self.m_relay = MediaRelay()

    def _withControlMap(self, callback):
        with self.m_args.shm_path.open("r+b", buffering=0) as file:
            file_size = os.fstat(file.fileno()).st_size
            if file_size < SharedMemoryTrack.c_header_size:
                raise ValueError(f"{self.m_args.shm_path} is too small: {file_size} < {SharedMemoryTrack.c_header_size}")
            with mmap.mmap(file.fileno(), SharedMemoryTrack.c_header_size, access=mmap.ACCESS_WRITE) as control_map:
                return callback(control_map)

    @staticmethod
    def _readControlMap(control_map: mmap.mmap) -> dict[str, int | bool]:
        serial, enable, fps, flags = struct.unpack_from("<IIII", control_map, SharedMemoryTrack.c_control_offset)
        return {"serial": serial, "enable": bool(enable), "fps": fps, "flags": flags}

    @staticmethod
    def _updateControlMap(control_map: mmap.mmap, request: ControlRequest) -> dict[str, int | bool]:
        serial, enable, fps, flags = struct.unpack_from("<IIII", control_map, SharedMemoryTrack.c_control_offset)
        if request.enable is not None:
            enable = 1 if request.enable else 0
        if request.fps is not None:
            fps = max(1, min(240, int(request.fps)))
        if request.flags is not None:
            flags = int(request.flags) & 0xFFFFFFFF
        next_serial = (serial + 1) & 0xFFFFFFFF
        struct.pack_into("<III", control_map, SharedMemoryTrack.c_control_offset + 4, enable, fps, flags)
        struct.pack_into("<I", control_map, SharedMemoryTrack.c_control_offset, next_serial)
        control_map.flush()
        return {"serial": next_serial, "enable": bool(enable), "fps": fps, "flags": flags}

    def _ensureTrack(self) -> SharedMemoryTrack:
        if self.m_track is None or self.m_track.readyState != "live":
            args = self.m_args
            self.m_track = SharedMemoryTrack(
                args.shm_path,
                args.width,
                args.height,
                args.ai_width,
                args.ai_height,
                args.poll_interval,
                flip_y=not args.no_flip_y,
            )
        return self.m_track


def make_app(args: argparse.Namespace) -> FastAPI:
    pcs: set[RTCPeerConnection] = set()
    closing_pcs: set[RTCPeerConnection] = set()
    source_hub = SourceHub(args)

    @asynccontextmanager
    async def lifespan(app: FastAPI):
        try:
            yield
        finally:
            await asyncio.gather(*(pc.close() for pc in pcs), return_exceptions=True)
            pcs.clear()
            source_hub.close()

    app = FastAPI(lifespan=lifespan)

    app.add_middleware(
        CORSMiddleware,
        allow_origins=["*"],
        allow_credentials=True,
        allow_methods=["*"],
        allow_headers=["*"],
    )

    @app.get("/health")
    async def health() -> dict[str, Any]:
        shm_size = args.shm_path.stat().st_size if args.shm_path.exists() else 0
        try:
            control = source_hub.readControl() if args.shm_path.exists() else None
        except (OSError, ValueError):
            control = None
        return {
            "source": "shm",
            "width": args.width,
            "height": args.height,
            "ai_width": args.ai_width,
            "ai_height": args.ai_height,
            "output_width": args.width + args.ai_width,
            "output_height": args.height,
            "flip_y": not args.no_flip_y,
            "shm_path": str(args.shm_path),
            "shm_exists": args.shm_path.exists(),
            "shm_size": shm_size,
            "peers": len(pcs),
            "singleton_active": source_hub.m_track is not None and source_hub.m_track.readyState == "live",
            "control": control,
        }

    @app.post("/control")
    async def control(request: ControlRequest) -> dict[str, Any]:
        try:
            control_state = source_hub.updateControl(request)
        except (FileNotFoundError, ValueError) as error:
            raise HTTPException(status_code=404, detail=str(error)) from error
        return {"control": control_state}

    async def cleanup_peer(pc: RTCPeerConnection) -> None:
        if pc in closing_pcs:
            return
        closing_pcs.add(pc)
        pcs.discard(pc)
        try:
            await pc.close()
        finally:
            closing_pcs.discard(pc)
        if not pcs:
            source_hub.close()

    @app.post("/offer")
    async def offer(offer: Offer) -> dict[str, str]:
        pc = RTCPeerConnection()
        pcs.add(pc)

        @pc.on("connectionstatechange")
        async def on_connectionstatechange() -> None:
            if pc.connectionState in {"failed", "closed", "disconnected"}:
                await cleanup_peer(pc)

        await pc.setRemoteDescription(RTCSessionDescription(sdp=offer.sdp, type=offer.type))
        try:
            track = source_hub.subscribe()
        except (FileNotFoundError, ValueError) as error:
            await cleanup_peer(pc)
            raise HTTPException(status_code=404, detail=str(error)) from error

        pc.addTrack(track)
        answer = await pc.createAnswer()
        await pc.setLocalDescription(answer)
        return {"sdp": pc.localDescription.sdp, "type": pc.localDescription.type}

    return app


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--shm-path", type=Path, default=Path("/dev/shm/reciever.dat"))
    parser.add_argument("--width", type=int, default=1280)
    parser.add_argument("--height", type=int, default=720)
    parser.add_argument("--ai-width", type=int, default=320)
    parser.add_argument("--ai-height", type=int, default=180)
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8000)
    parser.add_argument("--poll-interval", type=float, default=0.001)
    parser.add_argument("--no-flip-y", action="store_true")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    import uvicorn

    app = make_app(args)
    uvicorn.run(app, host=args.host, port=args.port)


if __name__ == "__main__":
    main()
