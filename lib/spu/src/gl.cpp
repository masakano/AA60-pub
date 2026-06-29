//
//
//
#include "spu_object.h"
#include <map>

using namespace spu::libspu;  // spu_error
namespace spu {

bool spu_gl_is_texture(uint32_t type)
{
	// clang-format off
	return (type == GL_SAMPLER_1D ||
		type == GL_SAMPLER_2D ||
		type == GL_SAMPLER_3D ||
	        type == GL_SAMPLER_2D_ARRAY ||
		type == GL_SAMPLER_2D_MULTISAMPLE ||
	        type == GL_SAMPLER_2D_SHADOW ||
		type == GL_SAMPLER_2D_ARRAY_SHADOW ||
	        type == GL_SAMPLER_2D_RECT ||
	        type == GL_SAMPLER_CUBE ||
		type == GL_SAMPLER_CUBE_SHADOW ||
	        type == GL_SAMPLER_CUBE_MAP_ARRAY ||
		type == GL_SAMPLER_BUFFER ||
	        type == GL_INT_SAMPLER_1D ||
		type == GL_INT_SAMPLER_2D ||
		type == GL_INT_SAMPLER_2D_RECT ||
	        type == GL_INT_SAMPLER_2D_ARRAY ||
		type == GL_INT_SAMPLER_BUFFER ||
		type == GL_UNSIGNED_INT_SAMPLER_1D ||
	        type == GL_UNSIGNED_INT_SAMPLER_2D ||
		type == GL_UNSIGNED_INT_SAMPLER_2D_RECT ||
	        type == GL_UNSIGNED_INT_SAMPLER_2D_ARRAY ||
		type == GL_UNSIGNED_INT_SAMPLER_BUFFER ||
		type == GL_SAMPLER_RENDERBUFFER_NV);
	// clang-format on
}

bool spu_gl_is_image(uint32_t type)
{
	// clang-format off
	return (type == GL_IMAGE_1D ||
		type == GL_IMAGE_1D_ARRAY ||
		type == GL_IMAGE_2D ||
		type == GL_IMAGE_2D_ARRAY ||
		type == GL_IMAGE_3D ||
	        type == GL_IMAGE_BUFFER ||
		type == GL_UNSIGNED_INT_IMAGE_1D ||
	        type == GL_UNSIGNED_INT_IMAGE_1D_ARRAY ||
		type == GL_UNSIGNED_INT_IMAGE_2D ||
	        type == GL_UNSIGNED_INT_IMAGE_2D_ARRAY ||
		type == GL_UNSIGNED_INT_IMAGE_3D ||
	        type == GL_UNSIGNED_INT_IMAGE_BUFFER);
	// clang-format on
}

bool spu_gl_is_depth_component(uint32_t type)
{
	// clang-format off
	return (type == GL_DEPTH_COMPONENT ||
		type == GL_DEPTH_COMPONENT32F ||
	        type == GL_DEPTH_COMPONENT32 ||
		type == GL_DEPTH_COMPONENT24 ||
	        type == GL_DEPTH_COMPONENT16 ||
		type == GL_DEPTH24_STENCIL8  ||
	        type == GL_DEPTH32F_STENCIL8 ||
		type == GL_STENCIL_INDEX8);
	// clang-format on
}

bool spu_gl_is_depth_stencil(uint32_t type)
{
	return (type == GL_DEPTH24_STENCIL8 || type == GL_DEPTH32F_STENCIL8);
}

uint32_t spu_gl_sizeof(uint32_t type)
{
	switch (type) {
	case GL_FLOAT_MAT4: return 4 * 16;
	case GL_FLOAT_MAT3: return 4 * 9;
	case GL_FLOAT_MAT2: return 4 * 4;
	case GL_FLOAT_VEC4: return 4 * 4;
	case GL_FLOAT_VEC3: return 4 * 3;
	case GL_FLOAT_VEC2: return 4 * 2;
	case GL_FLOAT: return 4 * 1;
	case GL_INT_VEC4: return 4 * 4;
	case GL_INT_VEC3: return 4 * 3;
	case GL_INT_VEC2: return 4 * 2;
	case GL_INT: return 4 * 1;
	case GL_UNSIGNED_INT: return 4 * 1;
	case GL_BOOL_VEC4: return 4 * 4;
	case GL_BOOL_VEC3: return 4 * 3;
	case GL_BOOL_VEC2: return 4 * 2;
	case GL_BOOL: return 4 * 1;
	case GL_HALF_FLOAT: return 2;
	case GL_SHORT: return 2;
	case GL_UNSIGNED_SHORT: return 2;
	case GL_BYTE: return 1;
	case GL_UNSIGNED_BYTE: return 1;
	case GL_DOUBLE_MAT4: return 8 * 16;
	case GL_DOUBLE_MAT3: return 8 * 9;
	case GL_DOUBLE_MAT2: return 8 * 4;
	case GL_DOUBLE_VEC4: return 8 * 4;
	case GL_DOUBLE_VEC3: return 8 * 3;
	case GL_DOUBLE_VEC2: return 8 * 2;
	case GL_DOUBLE: return 8 * 1;
	case GL_INT_2_10_10_10_REV: return 1;            // gl-330-buffer-type 4byte/4element
	case GL_UNSIGNED_INT_10F_11F_11F_REV: return 1;  // gl-330-buffer-type
	case GL_GPU_ADDRESS_NV: return 8;                // bindless

	default:  // type must be texture or image
		if (spu_gl_is_texture(type)) {
			return 4;
		}
		if (spu_gl_is_image(type)) {
			return 4;
		}
	}
	return 0;
	// clang-format on
}

int32_t spu_gl_compress_ratio(uint32_t type)
{
	switch (type) {
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_RG_RGTC2:
	case GL_COMPRESSED_RGBA_BPTC_UNORM: return 16;

	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RED_RGTC1:
	case GL_COMPRESSED_RGB8_ETC2:
	case GL_COMPRESSED_SRGB8_ETC2: return 8;
	default: return 0;  // straight
	}
}

namespace {

constexpr spu_gl_texinfo_t c_texinfo_tab[] = {
        // sized internal formats
        {GL_R8,                            8,   GL_RED,                           GL_UNSIGNED_BYTE,           true },
        {GL_R8_SNORM,                      8,   GL_RED,                           GL_BYTE,                    true },
        {GL_R16,                           16,  GL_RED,                           GL_UNSIGNED_SHORT,          true },
        {GL_R16_SNORM,                     16,  GL_RED,                           GL_SHORT,                   true },
        {GL_RG8,                           16,  GL_RG,                            GL_UNSIGNED_BYTE,           true },
        {GL_RG8_SNORM,                     16,  GL_RG,                            GL_BYTE,                    true },
        {GL_RG16,                          32,  GL_RG,                            GL_UNSIGNED_SHORT,          true },
        {GL_RG16_SNORM,                    32,  GL_RG,                            GL_SHORT,                   true },
        {GL_R3_G3_B2,                      8,   GL_RGB,                           GL_UNSIGNED_BYTE,           false},
        {GL_RGB4,                          12,  GL_RGB,                           GL_UNSIGNED_BYTE,           false},
        {GL_RGB5,                          15,  GL_RGB,                           GL_UNSIGNED_BYTE,           false},
        {GL_RGB8,                          24,  GL_RGB,                           GL_UNSIGNED_BYTE,           true },
        {GL_RGB8_SNORM,                    24,  GL_RGB,                           GL_BYTE,                    true },
        {GL_RGB10,                         30,  GL_RGB,                           GL_UNSIGNED_SHORT,          false},
        {GL_RGB12,                         48,  GL_RGB,                           GL_UNSIGNED_BYTE,           false},
        {GL_RGB16,                         48,  GL_RGB,                           GL_UNSIGNED_SHORT,          true },
        {GL_RGB16_SNORM,                   48,  GL_RGB,                           GL_SHORT,                   true },
        {GL_RGBA2,                         8,   GL_RGB,                           GL_UNSIGNED_BYTE,           false},
        {GL_RGBA4,                         16,  GL_RGB,                           GL_UNSIGNED_BYTE,           false},
        {GL_RGB5_A1,                       16,  GL_RGBA,                          GL_UNSIGNED_BYTE,           false},
        {GL_RGBA8,                         32,  GL_RGBA,                          GL_UNSIGNED_BYTE,           true },
        {GL_RGBA8_SNORM,                   32,  GL_RGBA,                          GL_BYTE,                    true },
        {GL_RGBA16,                        64,  GL_RGBA,                          GL_UNSIGNED_SHORT,          true },
        {GL_RGBA16_SNORM,                  64,  GL_RGBA,                          GL_SHORT,                   true },
        {GL_RGB10_A2UI,                    32,  GL_RGBA_INTEGER,                  GL_UNSIGNED_INT_10_10_10_2, true },
        {GL_RGBA12,                        48,  GL_RGBA,                          GL_UNSIGNED_SHORT,          false},
        {GL_RGBA16,                        64,  GL_RGBA,                          GL_UNSIGNED_SHORT,          true },
        {GL_SRGB8,                         32,  GL_RGB,                           GL_UNSIGNED_BYTE,           true },
        {GL_SRGB8_ALPHA8,                  32,  GL_RGBA,                          GL_UNSIGNED_BYTE,           true },
        {GL_R16F,                          16,  GL_RED,                           GL_HALF_FLOAT,              true },
        {GL_RG16F,                         32,  GL_RG,                            GL_HALF_FLOAT,              true },
        {GL_RGB16F,                        48,  GL_RGB,                           GL_HALF_FLOAT,              true },
        {GL_RGBA16F,                       64,  GL_RGBA,                          GL_HALF_FLOAT,              true },
        {GL_R32F,                          32,  GL_RED,                           GL_FLOAT,                   true },
        {GL_RG32F,                         64,  GL_RG,                            GL_FLOAT,                   true },
        {GL_RGB32F,                        96,  GL_RGB,                           GL_FLOAT,                   true },
        {GL_RGBA32F,                       128, GL_RGBA,                          GL_FLOAT,                   true },
        {GL_R11F_G11F_B10F,                32,  GL_RGB,                           GL_FLOAT,                   false},
        {GL_R8I,                           8,   GL_RED_INTEGER,                   GL_BYTE,                    true },
        {GL_R8UI,                          8,   GL_RED_INTEGER,                   GL_UNSIGNED_BYTE,           true },
        {GL_R16I,                          16,  GL_RED_INTEGER,                   GL_SHORT,                   true },
        {GL_R16UI,                         16,  GL_RED_INTEGER,                   GL_UNSIGNED_SHORT,          true },
        {GL_R32I,                          32,  GL_RED_INTEGER,                   GL_INT,                     true },
        {GL_R32UI,                         32,  GL_RED_INTEGER,                   GL_UNSIGNED_INT,            true },
        {GL_RG8I,                          16,  GL_RG_INTEGER,                    GL_BYTE,                    true },
        {GL_RG8UI,                         16,  GL_RG_INTEGER,                    GL_UNSIGNED_BYTE,           true },
        {GL_RG16I,                         32,  GL_RG_INTEGER,                    GL_SHORT,                   true },
        {GL_RG16UI,                        32,  GL_RG_INTEGER,                    GL_UNSIGNED_SHORT,          true },
        {GL_RG32I,                         64,  GL_RG_INTEGER,                    GL_INT,                     true },
        {GL_RG32UI,                        64,  GL_RG_INTEGER,                    GL_UNSIGNED_INT,            true },
        {GL_RGB8I,                         24,  GL_RGB_INTEGER,                   GL_BYTE,                    true },
        {GL_RGB8UI,                        24,  GL_RGB_INTEGER,                   GL_UNSIGNED_BYTE,           true },
        {GL_RGB16I,                        48,  GL_RGB_INTEGER,                   GL_SHORT,                   true },
        {GL_RGB16UI,                       48,  GL_RGB_INTEGER,                   GL_UNSIGNED_SHORT,          true },
        {GL_RGB32I,                        96,  GL_RGB_INTEGER,                   GL_INT,                     true },
        {GL_RGB32UI,                       96,  GL_RGB_INTEGER,                   GL_UNSIGNED_INT,            true },
        {GL_RGBA8I,                        32,  GL_RGBA_INTEGER,                  GL_BYTE,                    true },
        {GL_RGBA8UI,                       32,  GL_RGBA_INTEGER,                  GL_UNSIGNED_BYTE,           true },
        {GL_RGBA16I,                       64,  GL_RGBA_INTEGER,                  GL_SHORT,                   true },
        {GL_RGBA16UI,                      64,  GL_RGBA_INTEGER,                  GL_UNSIGNED_SHORT,          true },
        {GL_RGBA32I,                       128, GL_RGBA_INTEGER,                  GL_INT,                     true },
        {GL_RGBA32UI,                      128, GL_RGBA_INTEGER,                  GL_UNSIGNED_INT,            true },

        // sized depth-component formats
        {GL_DEPTH_COMPONENT32F,            32,  GL_DEPTH_COMPONENT,               GL_FLOAT,                   true },
        {GL_DEPTH_COMPONENT24,             24,  GL_DEPTH_COMPONENT,               GL_FLOAT,                   true },
        {GL_DEPTH_COMPONENT16,             16,  GL_DEPTH_COMPONENT,               GL_HALF_FLOAT,              true },

        // combined depth-stencil formats
        {GL_DEPTH32F_STENCIL8,             40,  GL_DEPTH_STENCIL,                 0,                          true },
        {GL_DEPTH24_STENCIL8,              32,  GL_DEPTH_STENCIL,                 0,                          true },

        // stencil-only format
        {GL_STENCIL_INDEX8,                8,   GL_STENCIL_INDEX,                 GL_UNSIGNED_BYTE,           true },

        // compressed: classic (subset)
        {GL_COMPRESSED_RG_RGTC2,           8,   GL_COMPRESSED_RG_RGTC2,           0,                          false},
        {GL_COMPRESSED_RGBA_BPTC_UNORM,    8,   GL_COMPRESSED_RGBA_BPTC_UNORM,    0,                          false},
        {GL_COMPRESSED_RED_RGTC1,          4,   GL_COMPRESSED_RED_RGTC1,          0,                          false},

        // compressed: S3TC
        {GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 4,   GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0,                          false},
        {GL_COMPRESSED_RGB_S3TC_DXT1_EXT,  4,   GL_COMPRESSED_RGB_S3TC_DXT1_EXT,  0,                          false},
        {GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 8,   GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,                          false},
        {GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 8,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0,                          false},

        // compressed: ETC
        {GL_COMPRESSED_RGB8_ETC2,          4,   GL_COMPRESSED_RGB8_ETC2,          0,                          false},
        {GL_COMPRESSED_SRGB8_ETC2,         4,   GL_COMPRESSED_SRGB8_ETC2,         0,                          false},

        // BGR & legacy
        {GL_RGB,                           24,  GL_RGB,                           GL_UNSIGNED_BYTE,           true },
        {GL_BGR,                           24,  GL_BGR,                           GL_UNSIGNED_BYTE,           true },
        {GL_RGBA,                          32,  GL_RGBA,                          GL_UNSIGNED_BYTE,           true },
        {GL_BGRA,                          32,  GL_BGRA,                          GL_UNSIGNED_BYTE,           true },
        {GL_ALPHA8,                        8,   GL_ALPHA,                         GL_UNSIGNED_BYTE,           true }, // tricky

        // other
        {GL_RGB9_E5,                       8,   GL_RGB,                           GL_FLOAT,                   true },
        {GL_RGB10_A2,                      32,  GL_RGBA,                          GL_UNSIGNED_SHORT,          false},
};
}  // namespace

const spu_gl_texinfo_t &spu_gl_texinfo(uint32_t iformat)
{
	assert(iformat);
	for (const auto &table: c_texinfo_tab) {
		if (iformat == table.iformat) {
			return table;
		}
	}
	aux_error(true, "spu_gl_texinfo: invalid format [%s]\n", opengl_const(iformat));
}

const spu_gl_texinfo_t &spu_gl_texinfo(uint32_t pformat, uint32_t ptype)
{
	assert(pformat);
	assert(ptype);

	for (const auto &table: c_texinfo_tab) {
		if (table.is_generic && pformat == table.pformat && ptype == table.ptype) {
			return table;
		}
	}
	aux_error(true, "spu_gl_texinfo: invalid format [%s]\n", opengl_const(pformat));
}
}  // namespace spu
