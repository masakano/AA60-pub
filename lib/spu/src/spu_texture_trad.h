//
// Unpacker :
//
#pragma once

#include "spu_texture_object.h"
#include "spu_texture_trad_lib.h"

namespace spu::libspu::spu_texture {

struct Unpacker {
	RectLoc rloc;
	uint32_t target;
	uint32_t w0, h0, d0;
	uint32_t w, h, d;
	int32_t sx, sy, sz;
	int32_t dx, dy, dz;
	uint32_t rw, rh;
	bool is_skip;

	Unpacker(uint32_t target, uint32_t width, uint32_t height, uint32_t depth, const RectLoc &rloc)
	        : rloc(rloc), target(target), is_skip(false)

	{
		w0 = std::max(1u, std::min(rloc.x, width - std::max(0, rloc.dx)));
		h0 = std::max(1u, std::min(rloc.y, height - std::max(0, rloc.dy)));
		d0 = std::max(1u, std::min(rloc.z, depth - std::max(0, rloc.dz)));
	}

	void shift(uint32_t level)
	{
		sx = std::max(0, -rloc.dx) >> level;
		sy = std::max(0, -rloc.dy) >> level;
		sz = std::max(0, -rloc.dz) >> level;

		dx = std::max(0, +rloc.dx) >> level;
		dy = std::max(0, +rloc.dy) >> level;
		dz = std::max(0, +rloc.dz) >> level;

		w = std::max(1u, w0 >> level);
		h = std::max(1u, h0 >> level);
		d = std::max(1u, d0 >> level);

		// depth is not mip-mapped?
		if (target != GL_TEXTURE_3D) {
			d = d0;
			dz = rloc.dz;
		}

		rw = std::max(1u, rloc.x >> level);
		rh = std::max(1u, rloc.y >> level);

		if (rw > w || rh > h || sx > 0 || sy > 0 || sz > 0) {
			is_skip = true;
			F(glPixelStorei, GL_UNPACK_ROW_LENGTH, rw);
			F(glPixelStorei, GL_UNPACK_IMAGE_HEIGHT, rh);
			F(glPixelStorei, GL_UNPACK_SKIP_PIXELS, sx);
			F(glPixelStorei, GL_UNPACK_SKIP_ROWS, sy);
			F(glPixelStorei, GL_UNPACK_SKIP_IMAGES, sz);
		}
	}

	~Unpacker()
	{
		if (is_skip) {
			F(glPixelStorei, GL_UNPACK_ROW_LENGTH, 0);
			F(glPixelStorei, GL_UNPACK_IMAGE_HEIGHT, 0);
			F(glPixelStorei, GL_UNPACK_SKIP_PIXELS, 0);
			F(glPixelStorei, GL_UNPACK_SKIP_ROWS, 0);
			F(glPixelStorei, GL_UNPACK_SKIP_IMAGES, 0);
		}
	}
};

class TextureTrad : public TextureObject {
public:
	TextureTrad(const Attrs &attrs) : TextureObject(attrs)
	{
		auto *pix = attrs.get<void *>("data", nullptr);

		if (m_handle.ui == 0) {
			F(glGenTextures, 1, &m_handle.ui);
			m_handle.target = attrs.get("target", GL_TEXTURE_2D);
			auto is_alias = attrs.get("alias", 0);
			if (is_alias) {
				// m_iformat = attrs.get("iformat", GL_RGBA8);  // set format only
			}
			else {
				set(attrs);  // make empty slot (gl-430-texture-view.cpp)
			}
		}
		else {
			set(attrs);
		}

		if (pix) {
			send(pix, m_iformat);
		}
	}

	~TextureTrad() override
	{
		if (glIsTexture(m_handle.id) == false) {
			warnNotExist();
		}
		if (m_bindlessId != 0u) {
			F(glMakeTextureHandleNonResidentNV, m_bindlessId);
		}
		uint32_t id_ui = m_handle.id;
		if (id_ui != 0u) {
			F(glDeleteTextures, 1, &id_ui);
		}
	}

	void bind() override
	{
		F(glBindTexture, m_handle.target, m_handle.id);
		if (glIsTexture(m_handle.id) == false) {
			warnNotExist();
		}
	}

	void update() override
	{
		if (m_isAutoMipmap) {
			F(glBindTexture, m_handle.target, m_handle.id);
			generateMipmap();
			F(glBindTexture, m_handle.target, 0);  // for sync
		}
	}

private:
	bool doUse(uint32_t slot) override
	{
		F(glActiveTexture, GL_TEXTURE0 + slot);  // necessary
		if (m_width == 0) {
			F(glBindTexture, m_handle.target, 0);
		}
		else {
			F(glBindTexture, m_handle.target, m_handle.id);
		}
		return true;
	}

	void doCopy(const TextureObject *src, const RectLoc &rloc) override
	{
		auto s_handle = src->handle();

		if (s_handle.target == GL_TEXTURE_BUFFER) {
			checkFullSize(rloc);
			spu_gl_texinfo_t texinfo = spu_gl_texinfo(m_iformat);

			// shortcut
			F(glBindBuffer, GL_PIXEL_UNPACK_BUFFER, src->m_bufferId);
			F(glBindTexture, m_handle.target, m_handle.id);
			texImageMutable_raw(
			        m_handle.target, 0, 0, texinfo.pformat, texinfo.ptype, m_width, m_height,
			        m_depth, 0, 0);

			F(glBindBuffer, GL_PIXEL_UNPACK_BUFFER, 0);
		}
		else {
			auto sx = rloc.sx;
			auto sy = rloc.sy;
			auto sz = rloc.sz;
			auto sl = uint32_t(rloc.sl);
			auto dx = rloc.dx;
			auto dy = rloc.dy;
			auto dz = rloc.dz;
			auto dl = uint32_t(rloc.dl);
			auto x = rloc.x;
			auto y = rloc.y;
			auto z = rloc.z;

			for (auto level = 0u; level <= m_maxLevel; level++) {
				if (sl <= src->m_maxLevel && dl <= m_maxLevel) {
					texCopyImage_raw(
					        s_handle, sl, sx, sy, sz, m_handle, dl, dx, dy, dz, x, y, z);

					sl++, dl++;

					sx /= 2, dx /= 2, x /= 2;
					sy /= 2, dy /= 2, y /= 2;

					if (m_handle.target == GL_TEXTURE_3D) {
						sz /= 2, dz /= 2, z /= 2;
					}
				}
			}
		}
	}

	void doSend(const void *pix, const RectLoc &rloc, uint32_t pformat, uint32_t ptype) override
	{
		if (m_width == 0) {
			return;  // do nothing
		}
		textureError(rloc.sl != 0 || rloc.l != 0, "mipmap level except destination must be zero");

		auto iformat = m_iformat;

		// BGR patch
		if (iformat == GL_BGR || iformat == GL_RGB) {
			iformat = GL_RGB8;
		}
		if (iformat == GL_BGRA || iformat == GL_RGBA) {
			iformat = GL_RGBA8;
		}

		F(glBindTexture, m_handle.target, m_handle.id);

		if (!m_isAllocated) {
			texStorage(m_handle.target, iformat, pformat, ptype, rloc);
		}

		if (m_handle.target == GL_TEXTURE_CUBE_MAP) {
			checkFullSize(rloc);
			const auto *cpix = static_cast<const uint8_t *>(pix);
			auto csize = cpix == nullptr ? 0 : spu_gl_texinfo(pformat, ptype).bit_per_pixel / 8;
			auto crloc = rloc;

			for (auto i = 0; i < 6; i++) {
				if (crloc.is_clear) {
					crloc.dz = i;
					texImage(m_handle.id, iformat, pformat, ptype, cpix, crloc);
					cpix += csize;
				}
				else {
					texImage(m_cubeTarget.ui[i], iformat, pformat, ptype, cpix, rloc);
					cpix += csize * m_width * m_height;
				}
			}
		}
		else {
			texImage(m_handle.target, iformat, pformat, ptype, pix, rloc);
		}

		if (m_isAutoMipmap) {
			generateMipmap();
		}
	}

	void doRecv(void *pix, const RectLoc &rloc, uint32_t pformat, uint32_t ptype) override
	{
		textureError(rloc.dl != 0 || rloc.l != 0, "cannot copy multiple levels\n");

		if (m_width != 0) {
			auto target = m_handle.target;
			auto id = m_handle.id;
			auto *cube_target = m_cubeTarget.ui;
			auto level = rloc.sl;
			auto w = rloc.x >> level;
			auto h = rloc.y >> level;
			auto d = rloc.z;
			auto bpp = spu_gl_texinfo(pformat, ptype).bit_per_pixel / 8;

			if (target == GL_RENDERBUFFER) {
				spu_message(0, "recv from renderbuffer (ignored)\n");
			}
			else if (target == GL_TEXTURE_CUBE_MAP) {
				checkFullSize(rloc);
				F(glBindTexture, target, id);
				auto *pix8 = static_cast<uint8_t *>(pix);
				for (auto i = 0; i < 6; i++) {
					F(glGetTexImage, cube_target[i], level, pformat, ptype, pix8);
					pix8 += w * h * d * bpp;
				}
			}
			else if (rloc.sl > 0) {
				checkFullSize(rloc);
				F(glBindTexture, target, id);
				F(glGetTexImage, target, level, pformat, ptype, pix);
			}
			else {
				auto sx = rloc.sx >> level;
				auto sy = rloc.sy >> level;
				auto sz = rloc.sz;
				F(glGetTextureSubImage, m_handle.id, level, sx, sy, sz, w, h, d, pformat, ptype,
				  w * h * d * bpp, pix);
			}
		}
	}

	void doReport() override
	{
		aux_printf("texture [ %d %s ]\n", m_handle.id, opengl_const(m_handle.target));
		if (m_iformat == 0) {
			uint32_t iformat = 0;
			getLevelParameter(GL_TEXTURE_INTERNAL_FORMAT, &iformat);
			aux_printf("\tformat       [%s]\n", opengl_const(iformat));
			aux_printf("\t     iformat %s\n", opengl_const(iformat));
		}
		else {
			spu_gl_texinfo_t texinfo = spu_gl_texinfo(m_iformat);
			aux_printf("\tformat label       %s\n", opengl_const(m_iformat));
			aux_printf("\t     iformat       %s\n", opengl_const(texinfo.iformat));
			aux_printf("\t     pformat       %s\n", opengl_const(texinfo.pformat));
			aux_printf("\t     ptype         %s\n", opengl_const(texinfo.ptype));
			aux_printf("\t     bit per_pixel %d\n", texinfo.bit_per_pixel);
		}

		aux_printf("\twidth        %d\n", m_width);
		aux_printf("\theight       %d\n", m_height);
		aux_printf("\tdepth        %d\n", m_depth);
		aux_printf("\tbase_level   %d\n", m_baseLevel);
		aux_printf("\tmax_level    %d\n", m_maxLevel);
		aux_printf("\tmultisample  %d\n", m_multisample);
		aux_printf("\tswizzle_r    %s\n", opengl_const(m_swizzleR));
		aux_printf("\tswizzle_g    %s\n", opengl_const(m_swizzleG));
		aux_printf("\tswizzle_b    %s\n", opengl_const(m_swizzleB));
		aux_printf("\tswizzle_a    %s\n", opengl_const(m_swizzleA));
		aux_printf("\twrap_s       %s\n", opengl_const(m_wrapS));
		aux_printf("\twrap_t       %s\n", opengl_const(m_wrapT));
		aux_printf("\twrap_r       %s\n", opengl_const(m_wrapR));
		aux_printf("\tmag_filter   %s\n", opengl_const(m_magFilter));
		aux_printf("\tmin_filter   %s\n", opengl_const(m_minFilter));
		aux_printf("\tdepth_mode   %s\n", opengl_const(m_depthMode));
		aux_printf("\tcompare_mode %s\n", opengl_const(m_compMode));
		aux_printf("\tcompare_func %s\n", opengl_const(m_compFunc));
		aux_printf("\tmin_lod      %f\n", m_minLod);
		aux_printf("\tmax_lod      %f\n", m_maxLod);
		aux_printf("\tlod_bias     %f\n", m_lodBias);
		aux_printf("\tmax_aniso    %f\n", m_maxAniso);
		aux_printf("\tborder       %f,%f,%f,%f\n", m_border.x, m_border.y, m_border.z, m_border.w);
		aux_printf("\tauto_mipmap  %d\n", m_isAutoMipmap);
		aux_printf("\tsrgb_decode  %d\n", m_isSrgbDecode);
	}

	void doSet() override
	{
		if (!m_isAllocated) {
			send(nullptr, m_iformat);  // realloc
		}

		// gl-400-fbo-multisample
		if (m_handle.target == GL_TEXTURE_2D_MULTISAMPLE) {
			return;
		}

		setParameter(GL_TEXTURE_MIN_FILTER, &m_minFilter);
		setParameter(GL_TEXTURE_MAG_FILTER, &m_magFilter);
		setParameter(GL_TEXTURE_WRAP_S, &m_wrapS);
		setParameter(GL_TEXTURE_WRAP_T, &m_wrapT);
		setParameter(GL_TEXTURE_WRAP_R, &m_wrapR);
		setParameter(GL_DEPTH_TEXTURE_MODE, &m_depthMode);
		setParameter(GL_TEXTURE_COMPARE_MODE, &m_compMode);
		setParameter(GL_TEXTURE_COMPARE_FUNC, &m_compFunc);
		setParameter(GL_TEXTURE_SWIZZLE_R, &m_swizzleR);
		setParameter(GL_TEXTURE_SWIZZLE_G, &m_swizzleG);
		setParameter(GL_TEXTURE_SWIZZLE_B, &m_swizzleB);
		setParameter(GL_TEXTURE_SWIZZLE_A, &m_swizzleA);
		setParameter(GL_TEXTURE_BORDER_COLOR, &m_border.x);

#if 1
		uint32_t srgb_decode = m_isSrgbDecode ? GL_DECODE_EXT : GL_SKIP_DECODE_EXT;
		setParameter(GL_TEXTURE_SRGB_DECODE_EXT, &srgb_decode);

#else
		{
			bool use_srgb;
			spu_graphics_get("use_srgb"_h32, &use_srgb);
			uint32_t srgb_decode
			        = (use_srgb && m_isSrgbDecode) ? GL_DECODE_EXT : GL_SKIP_DECODE_EXT;
			setParameter(GL_TEXTURE_SRGB_DECODE_EXT, &srgb_decode);
		}
#endif
		// 027_depth_of_field
		if (m_handle.target == GL_TEXTURE_RECTANGLE) {
			return;
		}
		setParameter(GL_TEXTURE_MIN_LOD, &m_minLod);
		setParameter(GL_TEXTURE_MAX_LOD, &m_maxLod);
		setParameter(GL_TEXTURE_LOD_BIAS, &m_lodBias);
		setParameter(GL_TEXTURE_MAX_ANISOTROPY, &m_maxAniso);
		setParameter(GL_TEXTURE_BASE_LEVEL, &m_baseLevel);
		setParameter(GL_TEXTURE_MAX_LEVEL, &m_maxLevel);
	}

	void doGet() override
	{
		getLevelParameter(GL_TEXTURE_WIDTH, &m_width);
		getLevelParameter(GL_TEXTURE_HEIGHT, &m_height);
		getLevelParameter(GL_TEXTURE_DEPTH, &m_depth);
		getLevelParameter(GL_TEXTURE_ALPHA_SIZE, &m_alphaSize);

		if (m_handle.target == GL_TEXTURE_2D_MULTISAMPLE) {
			return;
		}

		getParameter(GL_TEXTURE_MIN_FILTER, &m_minFilter);
		getParameter(GL_TEXTURE_MAG_FILTER, &m_magFilter);
		getParameter(GL_TEXTURE_WRAP_S, &m_wrapS);
		getParameter(GL_TEXTURE_WRAP_T, &m_wrapT);
		getParameter(GL_TEXTURE_WRAP_R, &m_wrapR);
		getParameter(GL_DEPTH_TEXTURE_MODE, &m_depthMode);
		getParameter(GL_TEXTURE_COMPARE_MODE, &m_compMode);
		getParameter(GL_TEXTURE_COMPARE_FUNC, &m_compFunc);
		getParameter(GL_TEXTURE_SWIZZLE_R, &m_swizzleR);
		getParameter(GL_TEXTURE_SWIZZLE_G, &m_swizzleG);
		getParameter(GL_TEXTURE_SWIZZLE_B, &m_swizzleB);
		getParameter(GL_TEXTURE_SWIZZLE_A, &m_swizzleA);
		getParameter(GL_TEXTURE_BORDER_COLOR, &m_border.x);

		uint32_t srgb_decode;
		getParameter(GL_TEXTURE_SRGB_DECODE_EXT, &srgb_decode);
		m_isSrgbDecode = srgb_decode == GL_DECODE_EXT;

		if (m_handle.target == GL_TEXTURE_RECTANGLE) {
			return;
		}
		getParameter(GL_TEXTURE_MIN_LOD, &m_minLod);
		getParameter(GL_TEXTURE_MAX_LOD, &m_maxLod);
		getParameter(GL_TEXTURE_LOD_BIAS, &m_lodBias);
		getParameter(GL_TEXTURE_MAX_ANISOTROPY, &m_maxAniso);
		getParameter(GL_TEXTURE_BASE_LEVEL, &m_baseLevel);
		getParameter(GL_TEXTURE_MAX_LEVEL, &m_maxLevel);
	}

	void doGetBindless() override
	{
		if (m_bindlessId == 0) {
			m_bindlessId = glGetTextureHandleNV(m_handle.id);
			F(glMakeTextureHandleResidentNV, m_bindlessId);
		}
	}

	uint32_t castPixelFormat(uint32_t pformat)
	{
		if (m_iformat == GL_STENCIL_INDEX8) {
			return GL_STENCIL_INDEX;
		}
		if (spu_gl_is_depth_stencil(m_iformat)) {
			return GL_DEPTH_STENCIL;
		}
		if (spu_gl_is_depth_component(m_iformat)) {  // must be last
			return GL_DEPTH_COMPONENT;
		}
		return pformat;
	}

	void generateMipmap()
	{
		if (m_iformat == GL_STENCIL_INDEX8) {
			spu_message(1, "skip generateMipmap for %s\n", opengl_const(m_iformat));
		}
		else if (
		        m_handle.target == GL_TEXTURE_RECTANGLE
		        || m_handle.target == GL_TEXTURE_2D_MULTISAMPLE) {
			spu_message(1, "skip generateMipmap for %s\n", opengl_const(m_handle.target));
		}
		else if (m_maxLevel < m_baseLevel) {
			spu_message(0, "mipmap (base_level==max_level (%d,%d)\n", m_baseLevel, m_maxLevel);
		}
		else {
			F(glGenerateMipmap, m_handle.target);
		}
	}

	void setParameter(uint32_t key, uint32_t *val)
	{
		if (val) {
			F(glTexParameteriv, m_handle.target, key, reinterpret_cast<int32_t *>(val));
		}
	}

	void setParameter(uint32_t key, int32_t *val)
	{
		if (val) {
			F(glTexParameteriv, m_handle.target, key, val);
		}
	}

	void setParameter(uint32_t key, float *val)
	{
		if (val) {
			F(glTexParameterfv, m_handle.target, key, val);
		}
	}

	void getParameter(uint32_t key, uint32_t *val)
	{
		F(glGetTexParameteriv, m_handle.target, key, reinterpret_cast<int32_t *>(val));
	}

	void getParameter(uint32_t key, int32_t *val) { F(glGetTexParameteriv, m_handle.target, key, val); }

	void getParameter(uint32_t key, float *val) { F(glGetTexParameterfv, m_handle.target, key, val); }

	static bool isMutable(uint32_t target, uint32_t iformat, uint32_t pformat)
	{
		if (spu_gl_compress_ratio(iformat) != 0) {
			return true;
		}
		if (spu_gl_compress_ratio(pformat) != 0) {
			return true;
		}

		switch (target) {
		case GL_TEXTURE_1D:
		case GL_TEXTURE_1D_ARRAY:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_CUBE_MAP:
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_RECTANGLE:
		case GL_TEXTURE_3D:
		case GL_TEXTURE_CUBE_MAP_ARRAY: return false;
		default: return true;
		}
	}

	void texStorage(
	        uint32_t local_target, uint32_t iformat, uint32_t pformat, uint32_t ptype, const RectLoc &rloc)
	{
		bool is_mutable = isMutable(local_target, iformat, pformat);

		aux_error(
		        m_isAllocated, "cannot change storage %dx%dx%d. Already allocated\n", m_width, m_height,
		        m_depth);

		m_isAllocated = true;
		m_maxLevel = std::min(m_maxLevel, getLevel());

		if (is_mutable) {  // alocate anyway
			Unpacker unpacker(m_handle.target, m_width, m_height, m_depth, rloc);
			for (auto level = 0u; level <= m_maxLevel; level++) {
				unpacker.shift(level);
				auto w = unpacker.w;
				auto h = unpacker.h;
				auto d = unpacker.d;
				texImageMutable_raw(
				        local_target, level, iformat, pformat, ptype, w, h, d, m_multisample,
				        nullptr);
			}
		}
		else {
			texStorage_raw(m_handle.target, m_maxLevel + 1, iformat, m_width, m_height, m_depth);
		}
	}

	void texImage(
	        uint32_t local_target, uint32_t iformat, uint32_t pformat, uint32_t ptype, const void *pix,
	        const RectLoc &rloc)
	{
		bool is_mutable = isMutable(m_handle.target, iformat, pformat);

		auto level = uint32_t(rloc.dl);
		if (level > m_maxLevel) {
			return;
		}

		Unpacker unpacker(m_handle.target, m_width, m_height, m_depth, rloc);
		unpacker.shift(level);

		auto dx = unpacker.dx;
		auto dy = unpacker.dy;
		auto dz = unpacker.dz;

		auto w = unpacker.w;
		auto h = unpacker.h;
		auto d = unpacker.d;

		if (rloc.is_clear) {
			F(glClearTexSubImage, m_handle.id, level, dx, dy, dz, w, h, d, pformat, ptype, pix);
		}
		else if (is_mutable) {
			textureError(rloc.dx != 0 || rloc.dy != 0 || rloc.dz != 0, "offset must be zero");
			texImageMutable_raw(
			        local_target, level, iformat, pformat, ptype, w, h, d, m_multisample, pix);
		}
		else if (pix) {
			texImageImutable_raw(local_target, level, pformat, ptype, w, h, d, dx, dy, dz, pix);
		}
	}

	uint32_t getLevel()
	{
		if (m_handle.target == GL_TEXTURE_2D_MULTISAMPLE || m_handle.target == GL_TEXTURE_RECTANGLE) {
			return 0;
		}
		uint32_t level;
		for (level = 0; level <= m_maxLevel; level++) {
			int32_t next_level = level + 1;
			if ((m_width >> next_level) == 0 && (m_height >> next_level) == 0) {
				break;
			}
		}
		return level;
	}

	void getLevelParameter(uint32_t key, uint32_t *val) const
	{
		int32_t val2 = 0;
		if (m_handle.target == GL_TEXTURE_CUBE_MAP) {
			F(glGetTexLevelParameteriv, GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, key, &val2);
		}
		else {
			F(glGetTexLevelParameteriv, m_handle.target, 0, key, &val2);
		}
		if ((val) && (val2 != 0)) {
			*val = val2;  // ignore if return value is zero
		}
	}
};
}  // namespace spu::libspu::spu_texture
