//
//
//
#include "sb6ktx.h"
#include "spu++/spu++.h"

namespace spu::sb6::ktx {
static const uint8_t c_identifier[] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
static uint32_t swap32(uint32_t u32)
{
	union {
		uint32_t u32;
		uint8_t u8[4];
	} a, b;
	a.u32 = u32;
	b.u8[0] = a.u8[3];
	b.u8[1] = a.u8[2];
	b.u8[2] = a.u8[1];
	b.u8[3] = a.u8[0];
	return b.u32;
}

static uint32_t calculate_stride(const header &h, uint32_t width, uint32_t pad = 4)
{
	auto channels = 0u;
	switch (h.glbaseinternalformat) {
	case GL_RED: channels = 1; break;
	case GL_RG: channels = 2; break;
	case GL_BGR:
	case GL_RGB: channels = 3; break;
	case GL_BGRA:
	case GL_RGBA: channels = 4; break;
	}
	auto stride = h.gltypesize * channels * width;
	stride = (stride + (pad - 1)) & ~(pad - 1);
	return stride;
}

uint32_t load(const char *filename, const Attrs &aux_attrs)
{
	File file(filename, "rb");
	header h;
	GLenum target = GL_NONE;
	auto total_size = file.size();
	auto read_size = 0u;
	file.read(&h, sizeof(h));
	read_size += sizeof(h);

	if (memcmp(h.identifier, c_identifier, sizeof(c_identifier)) != 0) {
		aux_error(true, "%s: invalid header\n", filename);
	}
	if (h.endianness == 0x04030201) {
		// No swap needed
	}
	else if (h.endianness == 0x01020304) {
		// Swap needed
		h.endianness = swap32(h.endianness);
		h.gltype = swap32(h.gltype);
		h.gltypesize = swap32(h.gltypesize);
		h.glformat = swap32(h.glformat);
		h.glinternalformat = swap32(h.glinternalformat);
		h.glbaseinternalformat = swap32(h.glbaseinternalformat);
		h.pixelwidth = swap32(h.pixelwidth);
		h.pixelheight = swap32(h.pixelheight);
		h.pixeldepth = swap32(h.pixeldepth);
		h.arrayelements = swap32(h.arrayelements);
		h.faces = swap32(h.faces);
		h.miplevels = swap32(h.miplevels);
		h.keypairbytes = swap32(h.keypairbytes);
	}
	else {
		aux_error(true, "%s: invalid header\n", filename);
	}
	file.read(0, h.keypairbytes);  // skip
	read_size += h.keypairbytes;

	auto data_size = total_size - read_size;
	auto image_size = calculate_stride(h, h.pixelwidth);

	// Guess target (texture type)
	if (h.pixelheight == 0) {
		if (h.arrayelements == 0) {
			target = GL_TEXTURE_1D;
		}
		else {
			target = GL_TEXTURE_1D_ARRAY;
			image_size *= h.arrayelements;
		}
	}
	else if (h.pixeldepth == 0) {
		image_size *= h.pixelheight;
		if (h.arrayelements == 0) {
			if (h.faces == 0) {
				target = GL_TEXTURE_2D;
			}
			else {
				target = GL_TEXTURE_CUBE_MAP;
				image_size *= h.faces;
			}
		}
		else {
			image_size *= h.arrayelements;
			if (h.faces == 0) {
				target = GL_TEXTURE_2D_ARRAY;
			}
			else {
				target = GL_TEXTURE_CUBE_MAP_ARRAY;
			}
		}
	}
	else {
		image_size *= h.pixelheight * h.pixeldepth;
		target = GL_TEXTURE_3D;
	}
	// Check for insanity...
	if (target == GL_NONE ||                        // Couldn't figure out target
	    (h.pixelwidth == 0) ||                      // Texture has no width???
	    (h.pixelheight == 0 && h.pixeldepth != 0))  // Texture has depth but no height???
	{
		aux_error(true, "%s: invalid header\n", filename);
	}
	if (data_size != image_size) {
		printf("sb6ktx: warning: data size mismatch!. data_size=%ld image_size=%d\n", data_size,
		       image_size);
	}
	std::vector<uint8_t> data(data_size);
	file.read(data.data(), data.size());
	if (h.miplevels == 0) {
		h.miplevels = 1;
	}
	// template
	// const void *pixv[] = { &data[0], 0 };
	auto format = h.glformat;
	if (h.gltype == GL_FLOAT) {
		switch (format) {
		case GL_RED: format = GL_R32F; break;
		case GL_RGB: format = GL_RGB32F; break;
		case GL_RGBA: format = GL_RGBA32F; break;
		default: assert(0);
		}
	}
	else if (h.gltype == GL_UNSIGNED_SHORT) {
		switch (format) {
		case GL_RED: format = GL_R16; break;
		default: assert(0);
		}
	}
	else {
		switch (format) {
		case GL_RED: format = GL_R8; break;
		case GL_RGB: format = GL_RGB8; break;
		case GL_RGBA: format = GL_RGBA8; break;
		case GL_BGR:
		case GL_BGRA: break;
		default: printf("type = %s\n", opengl_const(format)); assert(0);
		}
	}
	Attrs attrs = {
	        {"target",     GL_TEXTURE_2D            },
	        {"iformat",    format                   },
	        {"width",      h.pixelwidth             },
	        {"height",     h.pixelheight            },
	        {"min_filter", GL_LINEAR_MIPMAP_LINEAR  },
	        {"mag_filter", GL_LINEAR                },
	        {"data",       (const void*)data.data()},
	};
	attrs = attrs + aux_attrs;
	switch (target) {
	case GL_TEXTURE_1D: {
		attrs.replace("target", GL_TEXTURE_1D);
		return spu_texture_new(attrs);
	}
	case GL_TEXTURE_2D: {
		attrs.replace("target", GL_TEXTURE_2D);
		return spu_texture_new(attrs);
	}
	case GL_TEXTURE_3D: {
		attrs.replace("target", GL_TEXTURE_3D);
		attrs.push_back({"depth", h.pixeldepth});
		return spu_texture_new(attrs);
	}
	case GL_TEXTURE_1D_ARRAY: {
		attrs.replace("target", GL_TEXTURE_1D_ARRAY);
		attrs.push_back({"depth", h.arrayelements});
		return spu_texture_new(attrs);
	}
	case GL_TEXTURE_2D_ARRAY: {
		attrs.replace("target", GL_TEXTURE_2D_ARRAY);
		attrs.push_back({"depth", h.arrayelements});
		return spu_texture_new(attrs);
	}
	case GL_TEXTURE_CUBE_MAP: {
		const int32_t cube_target[] = {
		        GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1,
		        GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3,
		        GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5,
		};
		attrs.replace("target", GL_TEXTURE_CUBE_MAP);
		attrs.push_back({"cube_target", cube_target});
		return spu_texture_new(attrs);
	}
	default: {
		aux_error(true, "%s: format %s not supported\n", filename, opengl_const(target));
	}
	}
}
}  // namespace spu::sb6::ktx
