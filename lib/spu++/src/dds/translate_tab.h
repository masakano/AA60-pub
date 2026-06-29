//
//
//
#pragma once

namespace spu::dds {

namespace {
struct {
	uint32_t fourcc;
	uint32_t iformat;
	uint32_t pformat;
	uint32_t type;
	int32_t size;
} c_format_tab[] = {
        {FOURCC_R8G8B8,                    GL_RGB8,                       GL_BGR,                        GL_UNSIGNED_BYTE,                3 },
        {FOURCC_A8R8G8B8,                  GL_RGBA8,                      GL_BGRA,                       GL_UNSIGNED_BYTE,                4 },
        {FOURCC_X8R8G8B8,                  GL_RGB8,                       GL_BGRA,                       GL_UNSIGNED_INT_8_8_8_8,         4 },
        {FOURCC_R5G6B5,                    GL_RGB5,                       GL_BGR,                        GL_UNSIGNED_SHORT_5_6_5,         2 },
        {FOURCC_A8,                        GL_ALPHA8,                     GL_ALPHA,                      GL_UNSIGNED_BYTE,                1 },
        {FOURCC_A2B10G10R10,               GL_RGB10_A2,                   GL_RGBA,                       GL_UNSIGNED_INT_10_10_10_2,      4 },
        {FOURCC_A8B8G8R8,                  GL_RGBA8,                      GL_RGBA,                       GL_UNSIGNED_BYTE,                4 },
        {FOURCC_X8B8G8R8,                  GL_RGB8,                       GL_RGBA,                       GL_UNSIGNED_INT_8_8_8_8,         4 },
        {FOURCC_A2R10G10B10,               GL_RGB10_A2,                   GL_BGRA,                       GL_UNSIGNED_INT_10_10_10_2,      4 },
        {FOURCC_G16R16,                    GL_RG16,                       GL_RG,                         GL_UNSIGNED_SHORT,               4 },
        {FOURCC_A16B16G16R16,              GL_RGBA16,                     GL_RGBA,                       GL_UNSIGNED_SHORT,               8 },
        {FOURCC_L8,                        GL_LUMINANCE8,                 GL_LUMINANCE,                  GL_UNSIGNED_BYTE,                1 },
        {FOURCC_A8L8,                      GL_LUMINANCE8_ALPHA8,          GL_LUMINANCE_ALPHA,            GL_UNSIGNED_BYTE,                2 },
        {FOURCC_L16,                       GL_UNSIGNED_SHORT,             GL_LUMINANCE,                  GL_LUMINANCE16,                  2 },
        {FOURCC_Q16W16V16U16,              GL_RGBA16_SNORM,               GL_RGBA,                       GL_SHORT,                        8 },
        {FOURCC_R16F,                      GL_R16F,                       GL_RED,                        GL_HALF_FLOAT,                   2 },
        {FOURCC_G16R16F,                   GL_RG16F,                      GL_RG,                         GL_HALF_FLOAT,                   4 },
        {FOURCC_A16B16G16R16F,             GL_RGBA16F,                    GL_RGBA,                       GL_HALF_FLOAT,                   8 },
        {FOURCC_R32F,                      GL_R32F,                       GL_RED,                        GL_FLOAT,                        4 },
        {FOURCC_G32R32F,                   GL_RG32F,                      GL_RG,                         GL_FLOAT,                        8 },
        {FOURCC_A32B32G32R32F,             GL_RGBA32F,                    GL_RGBA,                       GL_FLOAT,                        16},

        {DDS10_FORMAT_R32G32B32A32_FLOAT,  GL_RGBA32F,                    GL_RGBA,                       GL_FLOAT,                        16},
        {DDS10_FORMAT_R32G32B32A32_UINT,   GL_RGBA32UI,                   GL_RGBA_INTEGER,               GL_UNSIGNED_INT,                 16},
        {DDS10_FORMAT_R32G32B32A32_SINT,   GL_RGBA32I,                    GL_RGBA_INTEGER,               GL_INT,                          16},
        {DDS10_FORMAT_R32G32B32_FLOAT,     GL_RGBA32F,                    GL_RGB,                        GL_FLOAT,                        12},
        {DDS10_FORMAT_R32G32B32_UINT,      GL_RGB32UI,                    GL_RGB_INTEGER,                GL_UNSIGNED_INT,                 12},
        {DDS10_FORMAT_R32G32B32_SINT,      GL_RGB32I,                     GL_RGB_INTEGER,                GL_INT,                          12},
        {DDS10_FORMAT_R16G16B16A16_FLOAT,  GL_RGBA16F,                    GL_RGBA,                       GL_HALF_FLOAT,                   8 },
        {DDS10_FORMAT_R16G16B16A16_UNORM,  GL_RGBA16,                     GL_RGBA,                       GL_UNSIGNED_SHORT,               8 },
        {DDS10_FORMAT_R16G16B16A16_UINT,   GL_RGBA16UI,                   GL_RGBA_INTEGER,               GL_UNSIGNED_SHORT,               8 },
        {DDS10_FORMAT_R16G16B16A16_SNORM,  GL_RGBA16_SNORM,               GL_RGBA,                       GL_SHORT,                        8 },
        {DDS10_FORMAT_R16G16B16A16_SINT,   GL_RGBA16I,                    GL_RGBA_INTEGER,               GL_SHORT,                        8 },
        {DDS10_FORMAT_R32G32_FLOAT,        GL_RG32F,                      GL_RG,                         GL_FLOAT,                        8 },
        {DDS10_FORMAT_R32G32_UINT,         GL_RG32UI,                     GL_RG_INTEGER,                 GL_UNSIGNED_INT,                 8 },
        {DDS10_FORMAT_R32G32_SINT,         GL_RG32I,                      GL_RG_INTEGER,                 GL_INT,                          8 },
        {DDS10_FORMAT_R10G10B10A2_UNORM,   GL_RGB10_A2,                   GL_RGBA,                       GL_UNSIGNED_INT_2_10_10_10_REV,  4 },
        {DDS10_FORMAT_R11G11B10_FLOAT,     GL_R11F_G11F_B10F,             GL_RGB,                        GL_UNSIGNED_INT_10F_11F_11F_REV, 4 },
        {DDS10_FORMAT_R8G8B8A8_UNORM,      GL_RGBA8,                      GL_RGBA,                       GL_UNSIGNED_BYTE,                4 },
        {DDS10_FORMAT_R8G8B8A8_UNORM_SRGB, GL_SRGB8_ALPHA8,               GL_RGBA,                       GL_UNSIGNED_BYTE,                4 },
        {DDS10_FORMAT_R8G8B8A8_UINT,       GL_RGBA8UI,                    GL_RGBA_INTEGER,               GL_UNSIGNED_BYTE,                4 },
        {DDS10_FORMAT_R8G8B8A8_SNORM,      GL_RGBA8_SNORM,                GL_RGBA,                       GL_BYTE,                         4 },
        {DDS10_FORMAT_R8G8B8A8_SINT,       GL_RGBA8UI,                    GL_RGBA_INTEGER,               GL_BYTE,                         4 },
        {DDS10_FORMAT_R16G16_FLOAT,        GL_RG16F,                      GL_RG,                         GL_HALF_FLOAT,                   4 },
        {DDS10_FORMAT_R16G16_UNORM,        GL_RG16,                       GL_RG,                         GL_UNSIGNED_SHORT,               4 },
        {DDS10_FORMAT_R16G16_UINT,         GL_RG16UI,                     GL_RG_INTEGER,                 GL_UNSIGNED_SHORT,               4 },
        {DDS10_FORMAT_R16G16_SNORM,        GL_RG16_SNORM,                 GL_RG,                         GL_SHORT,                        4 },
        {DDS10_FORMAT_R16G16_SINT,         GL_RG16I,                      GL_RG_INTEGER,                 GL_SHORT,                        4 },
        {DDS10_FORMAT_D32_FLOAT,           GL_DEPTH_COMPONENT32F,         GL_DEPTH,                      GL_FLOAT,                        4 },
        {DDS10_FORMAT_R32_FLOAT,           GL_R32F,                       GL_RED,                        GL_FLOAT,                        4 },
        {DDS10_FORMAT_R32_UINT,            GL_R32UI,                      GL_RED_INTEGER,                GL_UNSIGNED_INT,                 4 },
        {DDS10_FORMAT_R32_SINT,            GL_R32I,                       GL_RED_INTEGER,                GL_INT,                          4 },
        {DDS10_FORMAT_R8G8_UNORM,          GL_RG8,                        GL_RG,                         GL_UNSIGNED_BYTE,                2 },
        {DDS10_FORMAT_R8G8_UINT,           GL_RG8UI,                      GL_RG_INTEGER,                 GL_UNSIGNED_BYTE,                2 },
        {DDS10_FORMAT_R8G8_SNORM,          GL_RG8_SNORM,                  GL_RG,                         GL_BYTE,                         2 },
        {DDS10_FORMAT_R8G8_SINT,           GL_RG8I,                       GL_RG_INTEGER,                 GL_BYTE,                         2 },
        {DDS10_FORMAT_R16_FLOAT,           GL_R16F,                       GL_RED,                        GL_HALF_FLOAT,                   2 },
        {DDS10_FORMAT_D16_UNORM,           GL_DEPTH_COMPONENT16,          GL_DEPTH,                      GL_UNSIGNED_SHORT,               2 },
        {DDS10_FORMAT_R16_UNORM,           GL_R16,                        GL_RED,                        GL_UNSIGNED_SHORT,               2 },
        {DDS10_FORMAT_R16_UINT,            GL_R16UI,                      GL_RED_INTEGER,                GL_UNSIGNED_SHORT,               2 },
        {DDS10_FORMAT_R16_SNORM,           GL_R16_SNORM,                  GL_RED,                        GL_SHORT,                        2 },
        {DDS10_FORMAT_R16_SINT,            GL_R16I,                       GL_RED_INTEGER,                GL_SHORT,                        2 },
        {DDS10_FORMAT_R8_UNORM,            GL_R8,                         GL_RED,                        GL_UNSIGNED_BYTE,                1 },
        {DDS10_FORMAT_R8_UINT,             GL_R8UI,                       GL_RED_INTEGER,                GL_UNSIGNED_BYTE,                1 },
        {DDS10_FORMAT_R8_SNORM,            GL_R8_SNORM,                   GL_RED,                        GL_BYTE,                         1 },
        {DDS10_FORMAT_R8_SINT,             GL_R8I,                        GL_RED_INTEGER,                GL_BYTE,                         1 },
        {DDS10_FORMAT_A8_UNORM,            GL_ALPHA8,                     GL_ALPHA,                      GL_UNSIGNED_BYTE,                1 },
        {DDS10_FORMAT_R9G9B9E5_SHAREDEXP,  GL_RGB9_E5,                    GL_RGB,                        GL_UNSIGNED_INT_5_9_9_9_REV,     4 },
        {DDS10_FORMAT_B5G6R5_UNORM,        GL_RGB5,                       GL_BGR,                        GL_UNSIGNED_SHORT_5_6_5,         2 },
        {DDS10_FORMAT_B5G5R5A1_UNORM,      GL_RGB5_A1,                    GL_BGRA,                       GL_UNSIGNED_SHORT_5_5_5_1,       2 },
        {DDS10_FORMAT_B8G8R8A8_UNORM,      GL_RGBA8,                      GL_BGRA,                       GL_UNSIGNED_BYTE,                2 },
        {DDS10_FORMAT_B8G8R8X8_UNORM,      GL_RGB8,                       GL_BGRA,                       GL_UNSIGNED_BYTE,                4 },
        {DDS10_FORMAT_B8G8R8A8_UNORM_SRGB, GL_SRGB8_ALPHA8,               GL_BGRA,                       GL_UNSIGNED_BYTE,                4 },
        {DDS10_FORMAT_B8G8R8X8_UNORM_SRGB, GL_SRGB8,                      GL_BGRA,                       GL_UNSIGNED_BYTE,                4 },

        {DDS10_FORMAT_BC7_UNORM,           GL_COMPRESSED_RGBA_BPTC_UNORM, GL_COMPRESSED_RGBA_BPTC_UNORM, GL_UNSIGNED_BYTE,
         1                                                                                                                                  },
};

struct {
	uint32_t fourcc;
	uint32_t iformat;
	int32_t size;
} c_compressed_format_tab[] = {
        {FOURCC_DXT1,                 GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   8 },
        {FOURCC_DXT3,                 GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,   16},
        {FOURCC_DXT5,                 GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,   16},
        {FOURCC_ATI1,                 GL_COMPRESSED_RED_RGTC1_EXT,        8 },

        {FOURCC_ATI2,                 GL_COMPRESSED_RG_RGTC2,             16},

        {FOURCC_BC4U,                 GL_COMPRESSED_RED_RGTC1_EXT,        8 },
        {FOURCC_BC4S,                 GL_COMPRESSED_SIGNED_RED_RGTC1_EXT, 8 },

        {FOURCC_BC5S,                 GL_COMPRESSED_SIGNED_RG_RGTC2,      16},

        {DDS10_FORMAT_BC1_UNORM,      GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   8 },
        {DDS10_FORMAT_BC1_UNORM_SRGB, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   8 },
        {DDS10_FORMAT_BC2_UNORM,      GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,   16},
        {DDS10_FORMAT_BC2_UNORM_SRGB, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   16},
        {DDS10_FORMAT_BC3_UNORM,      GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,   16},
        {DDS10_FORMAT_BC3_UNORM_SRGB, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,   16},
        {DDS10_FORMAT_BC4_UNORM,      GL_COMPRESSED_RED_RGTC1_EXT,        8 },
        {DDS10_FORMAT_BC4_SNORM,      GL_COMPRESSED_SIGNED_RED_RGTC1_EXT, 8 },

        {DDS10_FORMAT_BC5_UNORM,      GL_COMPRESSED_RG_RGTC2,             16},

        {DDS10_FORMAT_BC5_SNORM,      GL_COMPRESSED_SIGNED_RG_RGTC2,      16},

        // { DDS10_FORMAT_BC7_UNORM, GL_COMPRESSED_RGBA_BPTC_UNORM, 1}, // not work

        // fallback temporary
        {0,                           GL_COMPRESSED_RGB8_ETC2,            8 },
};

struct {
	uint32_t flags;
	uint32_t bitcount;
	uint32_t maskR;
	uint32_t maskG;
	uint32_t maskB;
	uint32_t maskA;
	uint32_t iformat;
	uint32_t pformat;
	uint32_t type;
	int32_t size;

} c_color_mask_tab[] = {
        {DDSF_RGBA, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000, GL_RGBA8,             GL_RGBA,            GL_UNSIGNED_BYTE,        4},
        {DDSF_RGBA, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000, GL_RGBA8,             GL_BGRA,            GL_UNSIGNED_BYTE,        4},
        {DDSF_RGBA, 32, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000, GL_RGB10_A2,          GL_RGBA,
         GL_UNSIGNED_INT_2_10_10_10_REV,                                                                                                   4},
        {DDSF_RGBA, 32, 0x000003ff, 0x000ffc00, 0x3ff00000, 0xff000000, GL_RGB10_A2,          GL_RGBA,
         GL_UNSIGNED_INT_10_10_10_2,                                                                                                       4},

        {DDSF_RGB,  32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000, GL_RG16,              GL_RG,              GL_UNSIGNED_SHORT,       4},
        {DDSF_RGB,  32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000, GL_RGBA8,             GL_RGB,
         GL_UNSIGNED_INT_8_8_8_8,                                                                                                          4},
        {DDSF_RGB,  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000, GL_RGBA8,             GL_BGR,
         GL_UNSIGNED_INT_8_8_8_8,                                                                                                          4},

        // We support D3D's R5G6B5, which is actually RGB in linear memory. It is equivalent to
        // GL's GL_UNSIGNED_SHORT_5_6_5
        {0,         16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000, GL_RGB5,              GL_RGB,             GL_UNSIGNED_SHORT_5_6_5, 2},

        // We support D3D's A8L8 (flagged as 8 bits of red and 8 bits of alpha)
        {0,         16, 0x000000ff, 0x00000000, 0x00000000, 0x0000ff00, GL_LUMINANCE8_ALPHA8, GL_LUMINANCE_ALPHA,
         GL_UNSIGNED_BYTE,                                                                                                                 2},

        // GIMP header for L8A8
        {0,         16, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, GL_LUMINANCE8_ALPHA8, GL_LUMINANCE_ALPHA,
         GL_UNSIGNED_BYTE,                                                                                                                 2},

        // D3D's A8
        {0,         8,  0x00000000, 0x00000000, 0x00000000, 0x000000ff, GL_ALPHA8,            GL_ALPHA,           GL_UNSIGNED_BYTE,        1},

        // D3D's L8
        {0,         8,  0x00000000, 0x00000000, 0x00000000, 0x00000000, GL_LUMINANCE8,        GL_LUMINANCE,       GL_UNSIGNED_BYTE,
         1                                                                                                                                  },

        // fallbacks
        // we'll just guess BGRA8, because that is the common legacy format for improperly
        // labeled files
        {DDSF_RGBA, 32, 0,          0,          0,          0,          GL_RGBA8,             GL_BGRA,            GL_UNSIGNED_BYTE,        4},
        {DDSF_RGB,  32, 0,          0,          0,          0,          GL_RGB8,              GL_BGR,             GL_UNSIGNED_INT_8_8_8_8, 4},
        {DDSF_RGB,  24, 0,          0,          0,          0,          GL_RGB8,              GL_BGR,             GL_UNSIGNED_BYTE,        3},
};
}  // namespace
}  // namespace spu::dds
