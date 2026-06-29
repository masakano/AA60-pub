# streamer test bench

This directory contains the C++ streaming test bench, the Python WebRTC sender, and the Vite/React monitor.

## System
```text

     +--------------+
     |  sender_app  +----------+
     +------+-------+          |
            | udp (add delay)  |
     +------+-------+          |
     | receiver_app |          | http
     +------+-------+          |
            | shm              |
+------+-------------------+   |
| vite + uvicorn + python  +---+
+-----------+--------------+ 
            | wetrtc + http 
     +------+-------+
     | web monitor  |
     +--------------+

```

## Layout

```text
streamer/
  bin/          installed C++ executable
  data/         h264 sample inputs
  sys/          C++ sender / receiver / viewer sources
  wan21/        Python WebRTC sender
  web/          Vite + React browser monitor
```

## Build C++ streamer

```sh
cd streamer/sys/build
cmake ..
make -sj8 install
```

The installed executable is:

```text
streamer/bin/streamer
```

## C++ shm mode

Start the UDP sender. For localhost:

```sh
cd streamer/bin/
streamer -c sender -sender.path 127.0.0.1:12345 -escape 255 -iconic 1 &
```

Start the UDP receiver. It decodes the stream and writes RGBA frames to shared memory.

```sh
cd streamer/bin/
streamer -c receiver -receiver.path 127.0.0.1:12345
```

The shared memory path is currently defined in `sys/common.h` as `reciever.dat`, which maps to:

```text
/dev/shm/reciever.dat
```

The frame size is currently defined in `sys/common.h`:

```cpp
c_width  = 1280
c_height = 720
```

## Python WebRTC sender

Run commands from the project root unless noted otherwise.

```sh
cd ~/progs/AA60
streamer/wan21/.venv/bin/python streamer/wan21/client.py \
  --host 0.0.0.0 \
  --port 8000 \
  --width 1280 \
  --height 720 \
  --ai-width 320 \
  --ai-height 180
```

### Python options

```text
--shm-path /dev/shm/reciever.dat
--width 1280
--height 720
--ai-width 320
--ai-height 180
--poll-interval 0.001
--no-flip-y
--host 0.0.0.0
--port 8000
```

`client.py` uses a singleton shared-memory source pipeline. Multiple browser clients share one internal `SharedMemoryTrack` through `MediaRelay`. AI processing is implemented in `wan21/inference.py` as `inference_process(input_image, count, input_width, input_height, output_image, output_width, output_height)`, and runs once per source frame before the relay fan-out. The WebRTC output is a side-by-side RGBA frame: the full input on the left and the AI preview on the right. With the defaults this is `1600x720`.

Health check:

```sh
curl http://127.0.0.1:8000/health
```

## Web monitor

```sh
cd streamer/web
npm run dev -- --host 0.0.0.0
```

Open one of:

```text
http://localhost:5173/
http://<server-ip>:5173/
```

The browser connects to the Python WebRTC sender on port `8000` using the same host name as the page URL.
