//
// Image :
//
#include "flip.h"
#include "translate.h"
#include <spu++/dds/dds.h>

namespace spu::dds {

const void *Image::pixels(int32_t level, uint32_t face) const
{
	assert(level < m_levels);
	assert(face >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	assert(face == GL_TEXTURE_CUBE_MAP_POSITIVE_X || m_isCubemap);

	face = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	return m_pixv[face * m_levels + level].data();
}

int32_t Image::size(int32_t level, uint32_t face) const
{
	assert(level < m_levels);
	assert(face >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	assert(face == GL_TEXTURE_CUBE_MAP_POSITIVE_X || m_isCubemap);

	face = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	return m_pixv[face * m_levels + level].size();
}

uint32_t Image::upload(const Attrs &aux_attrs)
{
	auto iformat = aux_attrs.get("iformat", m_iformat);
	auto pformat = aux_attrs.get("pformat", m_pformat);
	auto ptype = aux_attrs.get("ptype", m_type);
	auto max_level = m_levels - 1;

	uint32_t target;
	std::vector<const void *> pixv;
	std::vector<uint8_t> cube_pix;

// #define MAINTENANCE
#ifdef MAINTENANCE
	aux_printf("UploadTexture:\n");
	aux_printf("    iformat      : %s\n", opengl_const(iformat));
	aux_printf("    pformat      : %s\n", opengl_const(pformat));
	aux_printf("    ptype        : %s\n", opengl_const(ptype));
#endif

	if (m_isCubemap) {
		target = GL_TEXTURE_CUBE_MAP;

		const uint32_t cube_targets[] = {
		        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		        GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		};

		for (const auto &cube_target: cube_targets) {
			const auto *pixels = (const uint8_t *)this->pixels(0, cube_target);
			auto size = this->size(0, cube_target);
			auto base = cube_pix.size();
			cube_pix.resize(base + size);
			memcpy(&cube_pix[base], pixels, size);
		}
		pixv.push_back(cube_pix.data());
	}
	else {
		target = GL_TEXTURE_2D;
		for (auto i = 0; i <= max_level; i++) {
			pixv.push_back(pixels(i));
		}
	}

	Attrs attrs = {
	        {"target",      target                },
	        {"iformat",     iformat               },
	        {"pformat",     pformat               },
	        {"width",       m_width               },
	        {"height",      m_height              },
	        {"max_level",   max_level             },
	        {"auto_mipmap", int32_t(max_level > 0)},
	};

	auto texture_id = spu_texture_new(attrs + aux_attrs);

	for (auto &pix: pixv) {
		uint32_t i = &pix - &pixv[0];
		int32_t locs[4] = {0, 0, 0, int32_t(i)};
		spu_texture_send(texture_id, pix, iformat, locs, nullptr, false, pformat, ptype);
	}
	return texture_id;
}

int32_t Image::get(const hash32_t &key, void *value) const
{
	if (key == "alpha") {
		*(int32_t *)value = hasAlpha();
	}
	else if (key == "levels") {
		*(int32_t *)value = m_levels;
	}
	else if (key == "layers") {
		*(int32_t *)value = m_layers;
	}
	else if (key == "pformat") {
		*(int32_t *)value = m_pformat;
	}
	else if (key == "iformat") {
		*(int32_t *)value = m_iformat;
	}
	else if (key == "cubemap") {
		*(int32_t *)value = m_isCubemap;
	}
	else {
		aux_error(1, "%s: unknown key\n", key.c_str());
	}
	return sizeof(int32_t);
}

bool Image::hasAlpha() const
{
	switch (m_pformat) {
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
	case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
	case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
	case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
	case GL_ALPHA:  // legacy
	case GL_LUMINANCE_ALPHA:
	case GL_RGBA:
	case GL_RGBA_INTEGER:
	case GL_BGRA:

	// safety
	case GL_ALPHA8:
	case GL_R8:
	case GL_RED: return true;
	}
	return false;
}

void Image::read(const char *filename, bool is_flip)
{
	File file(filename, "rb");

	char filecode[4];  // magic number

	file.read(filecode, sizeof(filecode));

	if (strncmp(filecode, "DDS ", 4) != 0) {
		aux_error(1, "%s: not DDS format\n", filename);
	}

	// read in DDS header
	DDS_HEADER ddsh;
	DDS_HEADER_10 ddsh10;

	file.read(&ddsh, sizeof(ddsh));

	// check if image is a volume texture
	if (((ddsh.dwCaps2 & DDSF_VOLUME) != 0u) && (ddsh.dwDepth > 0)) {
		m_depth = ddsh.dwDepth;
	}
	else {
		m_depth = 1;
	}

	if (((ddsh.ddspf.dwFlags & DDSF_FOURCC) != 0u) && (ddsh.ddspf.dwFourCC == FOURCC_DX10)) {
		file.read(&ddsh10, sizeof(ddsh10));
	}

	m_width = ddsh.dwWidth;
	m_height = ddsh.dwHeight;

	if ((ddsh.dwFlags & DDSF_MIPMAPCOUNT) != 0u) {
		m_levels = ddsh.dwMipMapCount;
	}
	else {
		m_levels = 1;
	}

	if (((ddsh.dwCaps2 & DDSF_CUBEMAP) != 0u)
	    && !(((ddsh.ddspf.dwFlags & DDSF_FOURCC) != 0u) && ddsh.ddspf.dwFourCC == FOURCC_DX10)) {
		m_layers = 0;
		m_layers += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEX) != 0u ? 1 : 0;
		m_layers += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEX) != 0u ? 1 : 0;
		m_layers += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEY) != 0u ? 1 : 0;
		m_layers += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEY) != 0u ? 1 : 0;
		m_layers += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEZ) != 0u ? 1 : 0;
		m_layers += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEZ) != 0u ? 1 : 0;

		if ((m_layers != 6) || (m_width != m_height)) {
			aux_error(1, "%s: not complete cubemap\n", filename);
		}

		m_isCubemap = true;
	}
	else {
		m_layers = 1;
		m_isCubemap = false;
	}

	m_isCompressed = false;
	m_elementSize = 0;

	parseDDS(&ddsh, &ddsh10);

	m_pixv.clear();

	for (auto face = 0; face < m_layers; face++) {
		auto w = m_width;
		auto h = m_height;
		auto d = m_depth;
		for (auto level = 0; level < m_levels; level++) {
			auto bw = (m_isCompressed) ? (w + 3) / 4 : w;
			auto bh = (m_isCompressed) ? (h + 3) / 4 : h;
			auto size = bw * bh * d * m_elementSize;

			std::vector<uint8_t> pixels(size);
			file.read(pixels.data(), size);
			if (is_flip && !m_isCubemap) {
				flip(pixels.data(), w, h, d);
			}
			m_pixv.emplace_back(pixels);

			// reduce mip sizes
			w = (w > 1) ? w >> 1 : 1;
			h = (h > 1) ? h >> 1 : 1;
			d = (d > 1) ? d >> 1 : 1;
		}
	}
}
}  // namespace spu::dds
