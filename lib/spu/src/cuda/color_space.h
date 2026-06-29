//
//
//
#pragma once

#include <cuda_runtime.h>
#include <stdint.h>

namespace spu::libspu::video {

typedef enum ColorSpaceStandard {
	ColorSpaceStandard_BT709 = 0,
	ColorSpaceStandard_BT601 = 2,
	ColorSpaceStandard_BT2020 = 4
} ColorSpaceStandard;

union BGRA32 {
	uint32_t d;
	uchar4 v;
	struct {
		uint8_t b, g, r, a;
	} c;
};

union RGBA32 {
	uint32_t d;
	uchar4 v;
	struct {
		uint8_t r, g, b, a;
	} c;
};

union BGRA64 {
	uint64_t d;
	ushort4 v;
	struct {
		uint16_t b, g, r, a;
	} c;
};

union RGBA64 {
	uint64_t d;
	ushort4 v;
	struct {
		uint16_t r, g, b, a;
	} c;
};

// ColorSpace.cu
template<class COLOR32>
void Nv12ToColor32(
        uint8_t *dpNv12, int nNv12Pitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight,
        int iMatrix = 0);

template<class COLOR64>
void Nv12ToColor64(
        uint8_t *dpNv12, int nNv12Pitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight,
        int iMatrix = 0);

template<class COLOR32>
void P016ToColor32(
        uint8_t *dpP016, int nP016Pitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight,
        int iMatrix = 4);
template<class COLOR64>
void P016ToColor64(
        uint8_t *dpP016, int nP016Pitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight,
        int iMatrix = 4);

template<class COLOR32>
void YUV444ToColor32(
        uint8_t *dpYUV444, int nPitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight,
        int iMatrix = 0);
template<class COLOR64>
void YUV444ToColor64(
        uint8_t *dpYUV444, int nPitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight,
        int iMatrix = 0);

template<class COLOR32>
void YUV444P16ToColor32(
        uint8_t *dpYUV444, int nPitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight,
        int iMatrix = 4);
template<class COLOR64>
void YUV444P16ToColor64(
        uint8_t *dpYUV444, int nPitch, uint8_t *dpBgra, int nBgraPitch, int nWidth, int nHeight,
        int iMatrix = 4);

template<class COLOR32>
void Nv12ToColorPlanar(
        uint8_t *dpNv12, int nNv12Pitch, uint8_t *dpBgrp, int nBgrpPitch, int nWidth, int nHeight,
        int iMatrix = 0);
template<class COLOR32>
void P016ToColorPlanar(
        uint8_t *dpP016, int nP016Pitch, uint8_t *dpBgrp, int nBgrpPitch, int nWidth, int nHeight,
        int iMatrix = 4);

template<class COLOR32>
void YUV444ToColorPlanar(
        uint8_t *dpYUV444, int nPitch, uint8_t *dpBgrp, int nBgrpPitch, int nWidth, int nHeight,
        int iMatrix = 0);
template<class COLOR32>
void YUV444P16ToColorPlanar(
        uint8_t *dpYUV444, int nPitch, uint8_t *dpBgrp, int nBgrpPitch, int nWidth, int nHeight,
        int iMatrix = 4);

void Bgra64ToP016(
        uint8_t *dpBgra, int nBgraPitch, uint8_t *dpP016, int nP016Pitch, int nWidth, int nHeight,
        int iMatrix = 4);

void ConvertUInt8ToUInt16(
        uint8_t *dpUInt8, uint16_t *dpUInt16, int nSrcPitch, int nDestPitch, int nWidth, int nHeight);
void ConvertUInt16ToUInt8(
        uint16_t *dpUInt16, uint8_t *dpUInt8, int nSrcPitch, int nDestPitch, int nWidth, int nHeight);

void ResizeNv12(
        unsigned char *dpDstNv12, int nDstPitch, int nDstWidth, int nDstHeight, unsigned char *dpSrcNv12,
        int nSrcPitch, int nSrcWidth, int nSrcHeight, unsigned char *dpDstNv12UV = nullptr);
void ResizeP016(
        unsigned char *dpDstP016, int nDstPitch, int nDstWidth, int nDstHeight, unsigned char *dpSrcP016,
        int nSrcPitch, int nSrcWidth, int nSrcHeight, unsigned char *dpDstP016UV = nullptr);

void ScaleYUV420(
        unsigned char *dpDstY, unsigned char *dpDstU, unsigned char *dpDstV, int nDstPitch, int nDstChromaPitch,
        int nDstWidth, int nDstHeight, unsigned char *dpSrcY, unsigned char *dpSrcU, unsigned char *dpSrcV,
        int nSrcPitch, int nSrcChromaPitch, int nSrcWidth, int nSrcHeight, bool bSemiplanar);
}  // namespace spu::libspu::video
