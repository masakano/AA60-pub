//
//
//
#pragma once

#define GL_GLEXT_PROTOTYPES
#include <ssys/ssys.h>
#include <GL/gl.h>
#include <GL/glext.h>

#define GL_TEXTURE_SAMPLER 0xffff
#define GL_TEXTURE_HOST_IMAGE 0x0001

namespace spu {

struct spu_gl_texinfo_t {
	uint32_t iformat;
	uint32_t bit_per_pixel;
	uint32_t pformat;
	uint32_t ptype;
	bool is_generic;
};

const spu_gl_texinfo_t &spu_gl_texinfo(uint32_t iformat);
const spu_gl_texinfo_t &spu_gl_texinfo(uint32_t pformat, uint32_t ptype);

bool spu_gl_is_texture(uint32_t type);
bool spu_gl_is_image(uint32_t type);
bool spu_gl_is_depth_component(uint32_t type);
bool spu_gl_is_depth_stencil(uint32_t type);
uint32_t spu_gl_sizeof(uint32_t type);
int32_t spu_gl_compress_ratio(uint32_t type);

template<class T> inline uint32_t spu_gl_typeof() = delete;
template<> inline uint32_t spu_gl_typeof<uint8_t>() { return GL_UNSIGNED_BYTE; }
template<> inline uint32_t spu_gl_typeof<uint16_t>() { return GL_UNSIGNED_SHORT; }
template<> inline uint32_t spu_gl_typeof<uint32_t>() { return GL_UNSIGNED_INT; }
template<> inline uint32_t spu_gl_typeof<float>() { return GL_FLOAT; }
}  // namespace spu
