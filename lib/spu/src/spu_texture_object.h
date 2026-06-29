//
// RectLoc :
//
#pragma once
#include "spu_object.h"

namespace spu::libspu::spu_texture {

struct RectLoc {
	int32_t dx, dy, dz, dl;
	int32_t sx, sy, sz, sl;
	uint32_t x, y, z, l;
	bool is_clear;

	void report(const char *str) const
	{
		aux_printf(
		        "rectloc: %s: src=(%d,%d,%d,%d) dst=(%d,%d,%d,%d) size=(%d,%d,%d,%d)  clear=%d\n", str,
		        sx, sy, sz, sl, dx, dy, dz, dl, x, y, z, l, is_clear);
	}
};

class TextureObject : public Object {
public:
	uint64_t m_bindlessId = 0;

	uint32_t m_size = 0;
	uint32_t m_width = 0;
	uint32_t m_height = 1;
	uint32_t m_depth = 1;
	uint32_t m_alphaSize = 0;
	uint32_t m_baseLevel = 0;
	uint32_t m_maxLevel = 1000;
	uint32_t m_multisample = 0;

	uint32_t m_bufferId = 0;
	uint32_t m_iformat = GL_RGBA8;
	uint32_t m_wrapS = GL_REPEAT;
	uint32_t m_wrapT = GL_REPEAT;
	uint32_t m_wrapR = GL_REPEAT;
	uint32_t m_magFilter = GL_LINEAR;
	uint32_t m_minFilter = GL_NEAREST_MIPMAP_LINEAR;  // this is the default
	uint32_t m_depthMode = GL_LUMINANCE;
	uint32_t m_compMode = GL_NONE;
	uint32_t m_compFunc = GL_LEQUAL;
	uint32_t m_swizzleR = GL_RED;
	uint32_t m_swizzleG = GL_GREEN;
	uint32_t m_swizzleB = GL_BLUE;
	uint32_t m_swizzleA = GL_ALPHA;

	float m_minLod = -1000;
	float m_maxLod = +1000;
	float m_lodBias = 0;
	float m_maxAniso = 1.0;

	bool m_isSrgbDecode = true;
	bool m_isAutoMipmap = true;
	vec4f_t m_border = {0, 0, 0, 0};

	struct {
		uint32_t ui[6];
	} m_cubeTarget = {
	        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	};

	bool m_isAllocated = 0;

	TextureObject(const Attrs &attrs)
	{
		attrs.peek("pix", "use 'data' instead");
		attrs.peek("format", "use 'iformat' instead");
		uint32_t iformat = attrs.get("iformat", 0);
		aux_error(
		        iformat == GL_DEPTH_COMPONENT32,
		        "'GL_DEPTH_COMPONENT' not allowed. Use 'GL_DEPTH_COMPONENT32F' instead\n");

		uint32_t texture_id = attrs.get("texture_id", ~0u);
		if (texture_id != ~0u) {
			m_handle.ui = texture_id;
		}
	}

	void set(const Attrs &attrs)
	{
		bind();
		doGet();

		uint32_t new_width = attrs.get("width", m_width);
		if (m_width > 0 && new_width != m_width) {
			this->report();
			attrs.peek("size", "use 'imutable allocation' instead");
			attrs.peek("width", "use 'imutable allocation' instead");
			attrs.peek("height", "use 'imutable allocation' instead");
			attrs.peek("depth", "use 'imutable allocation' instead");
			attrs.peek("iformat", "use 'imutable allocation' instead");
		}

		attrs.apply("size", m_size);
		attrs.apply("width", m_width);
		attrs.apply("height", m_height);
		attrs.apply("depth", m_depth);
		attrs.apply("base_level", m_baseLevel);
		attrs.apply("max_level", m_maxLevel);
		attrs.apply("multisample", m_multisample);
		attrs.apply("buffer_id", m_bufferId);
		attrs.apply("iformat", m_iformat);
		attrs.apply("wrap_s", m_wrapS);
		attrs.apply("wrap_t", m_wrapT);
		attrs.apply("wrap_r", m_wrapR);
		attrs.apply("mag_filter", m_magFilter);
		attrs.apply("min_filter", m_minFilter);
		attrs.apply("depth_mode", m_depthMode);
		attrs.apply("compare_mode", m_compMode);
		attrs.apply("compare_func", m_compFunc);
		attrs.apply("swizzle_r", m_swizzleR);
		attrs.apply("swizzle_g", m_swizzleG);
		attrs.apply("swizzle_b", m_swizzleB);
		attrs.apply("swizzle_a", m_swizzleA);

		attrs.apply("min_lod", m_minLod);
		attrs.apply("max_lod", m_maxLod);
		attrs.apply("lod_bias", m_lodBias);
		attrs.apply("max_aniso", m_maxAniso);
		attrs.apply("border", m_border);
		attrs.apply("auto_mipmap", m_isAutoMipmap);
		attrs.apply("srgb_decode", m_isSrgbDecode);

		// for attrs.trace
		memcpy(m_cubeTarget.ui, attrs.get("cube_target", m_cubeTarget.ui), sizeof(m_cubeTarget));
		textureError(m_height <= 0 || m_depth <= 0, "invalid size");
		doSet();
	}

	void set(const TextureObject *texture)  // texture-view
	{
		bind();
		doGet();

		m_baseLevel = texture->m_baseLevel;
		m_maxLevel = texture->m_maxLevel;
		m_multisample = texture->m_multisample;

		m_isAllocated = texture->m_isAllocated;
		m_wrapS = texture->m_wrapS;
		m_wrapT = texture->m_wrapT;
		m_wrapR = texture->m_wrapR;
		m_magFilter = texture->m_magFilter;
		m_minFilter = texture->m_minFilter;
		m_depthMode = texture->m_depthMode;
		m_compMode = texture->m_compMode;
		m_compFunc = texture->m_compFunc;
		m_swizzleR = texture->m_swizzleR;
		m_swizzleG = texture->m_swizzleG;
		m_swizzleB = texture->m_swizzleB;
		m_swizzleA = texture->m_swizzleA;

		m_minLod = texture->m_minLod;
		m_maxLod = texture->m_maxLod;
		m_lodBias = texture->m_lodBias;
		m_maxAniso = texture->m_maxAniso;

		m_border = texture->m_border;
		m_cubeTarget = texture->m_cubeTarget;

		m_isAutoMipmap = texture->m_isAutoMipmap;
		m_isSrgbDecode = texture->m_isSrgbDecode;

		doSet();
	}

	int32_t get(const hash32_t &k, void *v)
	{
		if (k == "bindless_id") {
			doGetBindless();
			*static_cast<uint64_t *>(v) = m_bindlessId;
			return 8;
		}

		auto n = 0;

		// clang-format off
		if ((n = getvalue(k, v, "target"_h32,	   m_handle.target))) { return n;}
		if ((n = getvalue(k, v, "size"_h32,	   m_size	  ))) { return n;}
		if ((n = getvalue(k, v, "width"_h32,	   m_width	  ))) { return n;}
		if ((n = getvalue(k, v, "height"_h32,	   m_height	  ))) { return n;}
		if ((n = getvalue(k, v, "depth"_h32,	   m_depth	  ))) { return n;}
		if ((n = getvalue(k, v, "alpha_size"_h32,  m_alphaSize	  ))) { return n;}
		if ((n = getvalue(k, v, "base_level"_h32,  m_baseLevel	  ))) { return n;}
		if ((n = getvalue(k, v, "max_level"_h32,   m_maxLevel	  ))) { return n;}
		if ((n = getvalue(k, v, "multisample"_h32, m_multisample  ))) { return n;}
		if ((n = getvalue(k, v, "buffer_id"_h32,   m_bufferId	  ))) { return n;}
		if ((n = getvalue(k, v, "iformat"_h32,	   m_iformat	  ))) { return n;}
		if ((n = getvalue(k, v, "wrap_s"_h32,	   m_wrapS	  ))) { return n;}
		if ((n = getvalue(k, v, "wrap_t"_h32,	   m_wrapT	  ))) { return n;}
		if ((n = getvalue(k, v, "wrap_r"_h32,	   m_wrapR	  ))) { return n;}
		if ((n = getvalue(k, v, "mag_filter"_h32,  m_magFilter	  ))) { return n;}
		if ((n = getvalue(k, v, "min_filter"_h32,  m_minFilter	  ))) { return n;}
		if ((n = getvalue(k, v, "depth_mode"_h32,  m_depthMode	  ))) { return n;}
		if ((n = getvalue(k, v, "compare_mode"_h32,m_compMode	  ))) { return n;}
		if ((n = getvalue(k, v, "compare_func"_h32,m_compFunc	  ))) { return n;}
		if ((n = getvalue(k, v, "swizzle_r"_h32,   m_swizzleR	  ))) { return n;}
		if ((n = getvalue(k, v, "swizzle_g"_h32,   m_swizzleG	  ))) { return n;}
		if ((n = getvalue(k, v, "swizzle_b"_h32,   m_swizzleB	  ))) { return n;}
		if ((n = getvalue(k, v, "swizzle_a"_h32,   m_swizzleA	  ))) { return n;}
		if ((n = getvalue(k, v, "min_lod"_h32,	   m_minLod	  ))) { return n;}
		if ((n = getvalue(k, v, "max_lod"_h32,	   m_maxLod	  ))) { return n;}
		if ((n = getvalue(k, v, "lod_bias"_h32,	   m_lodBias	  ))) { return n;}
		if ((n = getvalue(k, v, "max_aniso"_h32,   m_maxAniso	  ))) { return n;}
		if ((n = getvalue(k, v, "border"_h32,	   m_border	  ))) { return n;}
		if ((n = getvalue(k, v, "cube_target"_h32, m_cubeTarget	  ))) { return n;}
		if ((n = getvalue(k, v, "auto_mipmap"_h32, m_isAutoMipmap ))) { return n;}
		if ((n = getvalue(k, v, "srgb_decode"_h32, m_isSrgbDecode ))) { return n;}

		// clang-format on
		return 0u;
	}

	bool use(uint32_t slot)
	{
		clearPreviousSampler(slot);  // for safety
		return doUse(slot);
	}

	void copy(
	        const TextureObject *src, const int32_t dst_loc[4], const int32_t src_loc[4],
	        const uint32_t size[4])
	{
		uint32_t copy_size[4];
		uint32_t dst_target = m_handle.target;
		uint32_t dst_depth = m_depth;

		if (dst_target == GL_TEXTURE_CUBE_MAP || dst_target == GL_TEXTURE_CUBE_MAP_ARRAY) {
			dst_depth *= 6;
		}

		if (size) {
			memcpy(copy_size, size, sizeof(copy_size));
		}
		else if (src->m_handle.ui == 0 || src->m_handle.target == GL_TEXTURE_BUFFER) {
			copy_size[0] = m_width;
			copy_size[1] = m_height;
			copy_size[2] = dst_depth;
			copy_size[3] = m_maxLevel;
		}
		else {
			copy_size[0] = std::min(m_width, src->m_width);
			copy_size[1] = std::min(m_height, src->m_height);
			copy_size[2] = std::min(dst_depth, src->m_depth);
			copy_size[3] = std::min(m_maxLevel, src->m_maxLevel);
		}

		RectLoc rloc = src->makeRectLoc(dst_loc, src_loc, copy_size, false);  // alex
		doCopy(src, rloc);
	}

	void send(
	        const void *pix, uint32_t pformat, const int32_t dst_loc[4] = nullptr,
	        const int32_t src_loc[4] = nullptr, const uint32_t size[4] = nullptr, bool is_clear = false,
	        uint32_t raw_pformat = 0, uint32_t raw_ptype = 0)
	{
		RectLoc rloc = makeRectLoc(dst_loc, src_loc, size, is_clear);

		spu_gl_texinfo_t texinfo = spu_gl_texinfo(pformat);

		if (raw_pformat != 0u) {
			if (raw_pformat == GL_SRGB8) raw_pformat = GL_RGB8;
			if (raw_pformat == GL_SRGB8_ALPHA8) raw_pformat = GL_RGBA8;
			texinfo.pformat = raw_pformat;
		}
		if (raw_ptype != 0u) {
			texinfo.ptype = raw_ptype;
		}

		doSend(pix, rloc, texinfo.pformat, texinfo.ptype);
	}

	void recv(
	        void *pix, uint32_t pformat, const int32_t dst_loc[4], const int32_t src_loc[4],
	        const uint32_t size[4], uint32_t raw_pformat = 0, uint32_t raw_ptype = 0)
	{
		RectLoc rloc = makeRectLoc(dst_loc, src_loc, size, false);
		spu_gl_texinfo_t texinfo = spu_gl_texinfo(pformat);

		if (raw_pformat != 0u) {
			if (raw_pformat == GL_SRGB8) raw_pformat = GL_RGB8;
			if (raw_pformat == GL_SRGB8_ALPHA8) raw_pformat = GL_RGBA8;
			texinfo.pformat = raw_pformat;
		}
		if (raw_ptype != 0u) {
			texinfo.ptype = raw_ptype;
		}

		doRecv(pix, rloc, texinfo.pformat, texinfo.ptype);
	}

	void report()
	{
		bind();
		doGet();
		doReport();
	}

	virtual void update() = 0;
	virtual void bind() = 0;

protected:
	void warnNotExist() const
	{
		spu_message(
		        0, "[%s] (handle=%08x) not exist (already deleted?)\n", opengl_const(m_handle.target),
		        m_handle.ui);
	}

	void textureError(bool cond, const char *s) const
	{
		aux_error(cond, "[%s] (handle=%08x) %s\n", opengl_const(m_handle.target), m_handle.ui, s);
	}

	[[noreturn]] void textureError(const char *s) const
	{
		aux_error(true, "[%s] (handle=%08x) %s\n", opengl_const(m_handle.target), m_handle.ui, s);
	}

	void checkFullSize(const RectLoc &r) const { textureError(!isFullSize(r), "not full size copy"); }

	bool isFullSize(const RectLoc &rloc) const
	{
		return (rloc.sx == 0 && rloc.sy == 0 && rloc.sz == 0 && rloc.dx == 0 && rloc.dy == 0
		        && rloc.dz == 0 && rloc.x == m_width && rloc.y == m_height && rloc.z == m_depth
		        && rloc.l == 0);
	}

	virtual bool doUse(uint32_t slot) = 0;
	virtual void doSet() = 0;
	virtual void doGet() = 0;
	virtual void doReport() = 0;
	virtual void doSend(const void *pix, const RectLoc &rloc, uint32_t pformat, uint32_t ptype) = 0;
	virtual void doRecv(void *pix, const RectLoc &rloc, uint32_t pformat, uint32_t ptype) = 0;
	virtual void doCopy(const TextureObject *src, const RectLoc &rloc) = 0;
	virtual void doGetBindless() { assert(0); }

private:
	RectLoc makeRectLoc(
	        const int32_t dst_loc[4], const int32_t src_loc[4], const uint32_t size[4], bool is_clear) const
	{
		// for old program
		assert((dst_loc == nullptr || dst_loc[3] >= 0) & (src_loc == nullptr || src_loc[3] >= 0)
		       /*& (size == nullptr || size[3] >= 0)*/);

		RectLoc rloc = {
		        dst_loc ? dst_loc[0] : 0,
		        dst_loc ? dst_loc[1] : 0,
		        dst_loc ? dst_loc[2] : 0,
		        dst_loc ? dst_loc[3] : 0,

		        src_loc ? src_loc[0] : 0,
		        src_loc ? src_loc[1] : 0,
		        src_loc ? src_loc[2] : 0,
		        src_loc ? src_loc[3] : 0,

		        // reserve old size
		        (size && (size[0] != 0)) ? size[0] : m_width,
		        (size && (size[1] != 0)) ? size[1] : m_height,
		        (size && (size[2] != 0)) ? size[2] : m_depth,

		        0,
		        is_clear,
		};
		return rloc;
	}

	void clearPreviousSampler(uint32_t slot)
	{
		static bool is_sampler[256];
		textureError(slot >= 256, "too large texture slot\n");

		if (m_handle.target == GL_TEXTURE_SAMPLER) {
			is_sampler[slot] = true;
		}
		else if (is_sampler[slot]) {
			F(glBindSampler, slot, 0);
			is_sampler[slot] = false;
		}
	}
};
}  // namespace spu::libspu::spu_texture
