//
// DDS_PIXELFORMAT :
//
#pragma once
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <sys/types.h>
#include "block.h"

namespace spu::dds {

// surface description flags
const uint32_t DDSF_CAPS = 0x00000001L;
const uint32_t DDSF_HEIGHT = 0x00000002L;
const uint32_t DDSF_WIDTH = 0x00000004L;
const uint32_t DDSF_PITCH = 0x00000008L;
const uint32_t DDSF_PIXELFORMAT = 0x00001000L;
const uint32_t DDSF_MIPMAPCOUNT = 0x00020000L;
const uint32_t DDSF_LINEARSIZE = 0x00080000L;
const uint32_t DDSF_DEPTH = 0x00800000L;

// pixel format flags
const uint32_t DDSF_ALPHAPIXELS = 0x00000001L;
const uint32_t DDSF_FOURCC = 0x00000004L;
const uint32_t DDSF_RGB = 0x00000040L;
const uint32_t DDSF_RGBA = 0x00000041L;

// dwCaps1 flags
const uint32_t DDSF_COMPLEX = 0x00000008L;
const uint32_t DDSF_TEXTURE = 0x00001000L;
const uint32_t DDSF_MIPMAP = 0x00400000L;

// dwCaps2 flags
const uint32_t DDSF_CUBEMAP = 0x00000200L;
const uint32_t DDSF_CUBEMAP_POSITIVEX = 0x00000400L;
const uint32_t DDSF_CUBEMAP_NEGATIVEX = 0x00000800L;
const uint32_t DDSF_CUBEMAP_POSITIVEY = 0x00001000L;
const uint32_t DDSF_CUBEMAP_NEGATIVEY = 0x00002000L;
const uint32_t DDSF_CUBEMAP_POSITIVEZ = 0x00004000L;
const uint32_t DDSF_CUBEMAP_NEGATIVEZ = 0x00008000L;
const uint32_t DDSF_CUBEMAP_ALL_FACES = 0x0000FC00L;
const uint32_t DDSF_VOLUME = 0x00200000L;

// compressed texture types
const uint32_t FOURCC_UNKNOWN = 0;

#ifndef MAKEFOURCC
#define MAKEFOURCC(c0, c1, c2, c3)                                                               \
	((uint32_t)(u_char)(c0) | ((uint32_t)(u_char)(c1) << 8) | ((uint32_t)(u_char)(c2) << 16) \
	 | ((uint32_t)(u_char)(c3) << 24))
#endif

const uint32_t FOURCC_R8G8B8 = 20;
const uint32_t FOURCC_A8R8G8B8 = 21;
const uint32_t FOURCC_X8R8G8B8 = 22;
const uint32_t FOURCC_R5G6B5 = 23;
const uint32_t FOURCC_X1R5G5B5 = 24;
const uint32_t FOURCC_A1R5G5B5 = 25;
const uint32_t FOURCC_A4R4G4B4 = 26;
const uint32_t FOURCC_R3G3B2 = 27;
const uint32_t FOURCC_A8 = 28;
const uint32_t FOURCC_A8R3G3B2 = 29;
const uint32_t FOURCC_X4R4G4B4 = 30;
const uint32_t FOURCC_A2B10G10R10 = 31;
const uint32_t FOURCC_A8B8G8R8 = 32;
const uint32_t FOURCC_X8B8G8R8 = 33;
const uint32_t FOURCC_G16R16 = 34;
const uint32_t FOURCC_A2R10G10B10 = 35;
const uint32_t FOURCC_A16B16G16R16 = 36;

const uint32_t FOURCC_L8 = 50;
const uint32_t FOURCC_A8L8 = 51;
const uint32_t FOURCC_A4L4 = 52;

const uint32_t FOURCC_DXT1 = MAKEFOURCC('D', 'X', 'T', '1');
const uint32_t FOURCC_DXT2 = MAKEFOURCC('D', 'X', 'T', '2');
const uint32_t FOURCC_DXT3 = MAKEFOURCC('D', 'X', 'T', '3');
const uint32_t FOURCC_DXT4 = MAKEFOURCC('D', 'X', 'T', '4');
const uint32_t FOURCC_DXT5 = MAKEFOURCC('D', 'X', 'T', '5');
const uint32_t FOURCC_ATI1 = MAKEFOURCC('A', 'T', 'I', '1');
const uint32_t FOURCC_ATI2 = MAKEFOURCC('A', 'T', 'I', '2');
const uint32_t FOURCC_BC4U = MAKEFOURCC('B', 'C', '4', 'U');
const uint32_t FOURCC_BC4S = MAKEFOURCC('B', 'C', '4', 'S');
const uint32_t FOURCC_BC5S = MAKEFOURCC('B', 'C', '5', 'S');

const uint32_t FOURCC_D16_LOCKABLE = 70;
const uint32_t FOURCC_D32 = 71;
const uint32_t FOURCC_D24X8 = 77;
const uint32_t FOURCC_D16 = 80;
const uint32_t FOURCC_D32F_LOCKABLE = 82;
const uint32_t FOURCC_L16 = 81;
const uint32_t FOURCC_DX10 = MAKEFOURCC('D', 'X', '1', '0');

// signed normalized formats
const uint32_t FOURCC_Q16W16V16U16 = 110;

// Floating point surface formats

// s10e5 formats (16-bits per channel)
const uint32_t FOURCC_R16F = 111;
const uint32_t FOURCC_G16R16F = 112;
const uint32_t FOURCC_A16B16G16R16F = 113;

// IEEE s23e8 formats (32-bits per channel)
const uint32_t FOURCC_R32F = 114;
const uint32_t FOURCC_G32R32F = 115;
const uint32_t FOURCC_A32B32G32R32F = 116;

// DXGI enums
const uint32_t DDS10_FORMAT_UNKNOWN = 0;
const uint32_t DDS10_FORMAT_R32G32B32A32_TYPELESS = 1;
const uint32_t DDS10_FORMAT_R32G32B32A32_FLOAT = 2;
const uint32_t DDS10_FORMAT_R32G32B32A32_UINT = 3;
const uint32_t DDS10_FORMAT_R32G32B32A32_SINT = 4;
const uint32_t DDS10_FORMAT_R32G32B32_TYPELESS = 5;
const uint32_t DDS10_FORMAT_R32G32B32_FLOAT = 6;
const uint32_t DDS10_FORMAT_R32G32B32_UINT = 7;
const uint32_t DDS10_FORMAT_R32G32B32_SINT = 8;
const uint32_t DDS10_FORMAT_R16G16B16A16_TYPELESS = 9;
const uint32_t DDS10_FORMAT_R16G16B16A16_FLOAT = 10;
const uint32_t DDS10_FORMAT_R16G16B16A16_UNORM = 11;
const uint32_t DDS10_FORMAT_R16G16B16A16_UINT = 12;
const uint32_t DDS10_FORMAT_R16G16B16A16_SNORM = 13;
const uint32_t DDS10_FORMAT_R16G16B16A16_SINT = 14;
const uint32_t DDS10_FORMAT_R32G32_TYPELESS = 15;
const uint32_t DDS10_FORMAT_R32G32_FLOAT = 16;
const uint32_t DDS10_FORMAT_R32G32_UINT = 17;
const uint32_t DDS10_FORMAT_R32G32_SINT = 18;
const uint32_t DDS10_FORMAT_R32G8X24_TYPELESS = 19;
const uint32_t DDS10_FORMAT_D32_FLOAT_S8X24_UINT = 20;
const uint32_t DDS10_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21;
const uint32_t DDS10_FORMAT_X32_TYPELESS_G8X24_UINT = 22;
const uint32_t DDS10_FORMAT_R10G10B10A2_TYPELESS = 23;
const uint32_t DDS10_FORMAT_R10G10B10A2_UNORM = 24;
const uint32_t DDS10_FORMAT_R10G10B10A2_UINT = 25;
const uint32_t DDS10_FORMAT_R11G11B10_FLOAT = 26;
const uint32_t DDS10_FORMAT_R8G8B8A8_TYPELESS = 27;
const uint32_t DDS10_FORMAT_R8G8B8A8_UNORM = 28;
const uint32_t DDS10_FORMAT_R8G8B8A8_UNORM_SRGB = 29;
const uint32_t DDS10_FORMAT_R8G8B8A8_UINT = 30;
const uint32_t DDS10_FORMAT_R8G8B8A8_SNORM = 31;
const uint32_t DDS10_FORMAT_R8G8B8A8_SINT = 32;
const uint32_t DDS10_FORMAT_R16G16_TYPELESS = 33;
const uint32_t DDS10_FORMAT_R16G16_FLOAT = 34;
const uint32_t DDS10_FORMAT_R16G16_UNORM = 35;
const uint32_t DDS10_FORMAT_R16G16_UINT = 36;
const uint32_t DDS10_FORMAT_R16G16_SNORM = 37;
const uint32_t DDS10_FORMAT_R16G16_SINT = 38;
const uint32_t DDS10_FORMAT_R32_TYPELESS = 39;
const uint32_t DDS10_FORMAT_D32_FLOAT = 40;
const uint32_t DDS10_FORMAT_R32_FLOAT = 41;
const uint32_t DDS10_FORMAT_R32_UINT = 42;
const uint32_t DDS10_FORMAT_R32_SINT = 43;
const uint32_t DDS10_FORMAT_R24G8_TYPELESS = 44;
const uint32_t DDS10_FORMAT_D24_UNORM_S8_UINT = 45;
const uint32_t DDS10_FORMAT_R24_UNORM_X8_TYPELESS = 46;
const uint32_t DDS10_FORMAT_X24_TYPELESS_G8_UINT = 47;
const uint32_t DDS10_FORMAT_R8G8_TYPELESS = 48;
const uint32_t DDS10_FORMAT_R8G8_UNORM = 49;
const uint32_t DDS10_FORMAT_R8G8_UINT = 50;
const uint32_t DDS10_FORMAT_R8G8_SNORM = 51;
const uint32_t DDS10_FORMAT_R8G8_SINT = 52;
const uint32_t DDS10_FORMAT_R16_TYPELESS = 53;
const uint32_t DDS10_FORMAT_R16_FLOAT = 54;
const uint32_t DDS10_FORMAT_D16_UNORM = 55;
const uint32_t DDS10_FORMAT_R16_UNORM = 56;
const uint32_t DDS10_FORMAT_R16_UINT = 57;
const uint32_t DDS10_FORMAT_R16_SNORM = 58;
const uint32_t DDS10_FORMAT_R16_SINT = 59;
const uint32_t DDS10_FORMAT_R8_TYPELESS = 60;
const uint32_t DDS10_FORMAT_R8_UNORM = 61;
const uint32_t DDS10_FORMAT_R8_UINT = 62;
const uint32_t DDS10_FORMAT_R8_SNORM = 63;
const uint32_t DDS10_FORMAT_R8_SINT = 64;
const uint32_t DDS10_FORMAT_A8_UNORM = 65;
const uint32_t DDS10_FORMAT_R1_UNORM = 66;
const uint32_t DDS10_FORMAT_R9G9B9E5_SHAREDEXP = 67;
const uint32_t DDS10_FORMAT_R8G8_B8G8_UNORM = 68;
const uint32_t DDS10_FORMAT_G8R8_G8B8_UNORM = 69;
const uint32_t DDS10_FORMAT_BC1_TYPELESS = 70;
const uint32_t DDS10_FORMAT_BC1_UNORM = 71;
const uint32_t DDS10_FORMAT_BC1_UNORM_SRGB = 72;
const uint32_t DDS10_FORMAT_BC2_TYPELESS = 73;
const uint32_t DDS10_FORMAT_BC2_UNORM = 74;
const uint32_t DDS10_FORMAT_BC2_UNORM_SRGB = 75;
const uint32_t DDS10_FORMAT_BC3_TYPELESS = 76;
const uint32_t DDS10_FORMAT_BC3_UNORM = 77;
const uint32_t DDS10_FORMAT_BC3_UNORM_SRGB = 78;
const uint32_t DDS10_FORMAT_BC4_TYPELESS = 79;
const uint32_t DDS10_FORMAT_BC4_UNORM = 80;
const uint32_t DDS10_FORMAT_BC4_SNORM = 81;
const uint32_t DDS10_FORMAT_BC5_TYPELESS = 82;
const uint32_t DDS10_FORMAT_BC5_UNORM = 83;
const uint32_t DDS10_FORMAT_BC5_SNORM = 84;
const uint32_t DDS10_FORMAT_B5G6R5_UNORM = 85;
const uint32_t DDS10_FORMAT_B5G5R5A1_UNORM = 86;
const uint32_t DDS10_FORMAT_B8G8R8A8_UNORM = 87;
const uint32_t DDS10_FORMAT_B8G8R8X8_UNORM = 88;
const uint32_t DDS10_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89;
const uint32_t DDS10_FORMAT_B8G8R8A8_TYPELESS = 90;
const uint32_t DDS10_FORMAT_B8G8R8A8_UNORM_SRGB = 91;
const uint32_t DDS10_FORMAT_B8G8R8X8_TYPELESS = 92;
const uint32_t DDS10_FORMAT_B8G8R8X8_UNORM_SRGB = 93;
const uint32_t DDS10_FORMAT_BC6H_TYPELESS = 94;
const uint32_t DDS10_FORMAT_BC6H_UF16 = 95;
const uint32_t DDS10_FORMAT_BC6H_SF16 = 96;
const uint32_t DDS10_FORMAT_BC7_TYPELESS = 97;
const uint32_t DDS10_FORMAT_BC7_UNORM = 98;
const uint32_t DDS10_FORMAT_BC7_UNORM_SRGB = 99;
const uint32_t DDS10_FORMAT_FORCE_UINT = 0xffffffffUL;

// DDS 10 resource dimension enums
const uint32_t DDS10_RESOURCE_DIMENSION_UNKNOWN = 0;
const uint32_t DDS10_RESOURCE_DIMENSION_BUFFER = 1;
const uint32_t DDS10_RESOURCE_DIMENSION_TEXTURE1D = 2;
const uint32_t DDS10_RESOURCE_DIMENSION_TEXTURE2D = 3;
const uint32_t DDS10_RESOURCE_DIMENSION_TEXTURE3D = 4;

struct DDS_PIXELFORMAT {
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwFourCC;
	uint32_t dwRGBBitCount;
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwABitMask;
};

struct DDS_HEADER {
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwHeight;
	uint32_t dwWidth;
	uint32_t dwPitchOrLine3farSize;
	uint32_t dwDepth;
	uint32_t dwMipMapCount;
	uint32_t dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t dwCaps1;
	uint32_t dwCaps2;
	uint32_t dwReserved2[3];
};

struct DDS_HEADER_10 {
	uint32_t dxgiFormat;         // check type
	uint32_t resourceDimension;  // check type
	uint32_t miscFlag;
	uint32_t arraySize;
	uint32_t reserved;
};

}  // namespace spu::dds

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
