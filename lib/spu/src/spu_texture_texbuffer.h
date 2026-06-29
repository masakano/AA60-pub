//
// TextureTexbuffer :
//
#pragma once

#include "spu_texture_object.h"

namespace spu::libspu::spu_texture {

class TextureTexbuffer : public TextureObject {
public:
	TextureTexbuffer(const Attrs &attrs) : TextureObject(attrs)
	{
		if (m_handle.ui == 0) {
			F(glGenTextures, 1, &m_handle.ui);
			m_handle.target = GL_TEXTURE_BUFFER;
		}
		if (m_bufferId == 0) {
			F(glGenBuffers, 1, &m_bufferId);
		}
		set(attrs);
		m_width = m_size;

		auto *pix = attrs.get<void *>("data", nullptr);
		if (pix) {
			send(pix, m_iformat);
		}
	}

	~TextureTexbuffer() override
	{
		uint32_t id_ui = m_handle.id;
		F(glDeleteTextures, 1, &id_ui);
		F(glDeleteBuffers, 1, &m_bufferId);
	}

	void bind() override
	{
		F(glBindTexture, GL_TEXTURE_BUFFER, m_handle.id);
		auto is_texture = glIsTexture(m_handle.id);
		if (is_texture == 0) {
			warnNotExist();
		}
	}

	void update() override {}

private:
	bool doUse(uint32_t slot) override
	{
		F(glActiveTexture, GL_TEXTURE0 + slot);            // necessary
		F(glBindTexture, GL_TEXTURE_BUFFER, m_handle.id);  // skip check for speed
		return true;
	}

	void doSend(const void *pix, const RectLoc &rloc, uint32_t pformat, uint32_t ptype) override
	{
		warnEmptyBuffer();

		if (m_size > 0) {
			F(glBindBuffer, GL_TEXTURE_BUFFER, m_bufferId);
			if (!m_isAllocated) {
				F(glBufferData, GL_TEXTURE_BUFFER, m_size, nullptr, GL_STATIC_DRAW);
				m_isAllocated = true;
			}
			if (rloc.is_clear) {
				checkFullSize(rloc);
				F(glClearBufferData, GL_TEXTURE_BUFFER, m_iformat, pformat, ptype, pix);
			}
			else {
				/*
				printf("glBufferSubData:\n");
				printf("   bufferId : %08x\n",  m_bufferId);
				printf("   iformat  : %s \n",  opengl_const(m_iformat));
				*/
				const void *ptr = reinterpret_cast<const char *>(pix) + rloc.sx;
				F(glBufferSubData, GL_TEXTURE_BUFFER, rloc.dx, rloc.x, ptr);
				F(glBindTexture, GL_TEXTURE_BUFFER, m_handle.id);
				F(glTexBuffer, GL_TEXTURE_BUFFER, m_iformat, m_bufferId);
			}
		}
	}

#if 0  // MapBufferRange()
	void doRecv(void *pix, const RectLoc &rloc, uint32_t /*pformat*/, uint32_t /*ptype*/) override
	{
		warnEmptyBuffer();
		if (m_size > 0) {
			F(glBindBuffer, GL_TEXTURE_BUFFER, m_bufferId);
			void *src = glMapBufferRange(GL_TEXTURE_BUFFER, rloc.sx, rloc.x, GL_MAP_READ_BIT);
			memcpy(pix, src, rloc.x);
			F(glUnmapBuffer, GL_TEXTURE_BUFFER);
		}
	}
#else  // GetBufferSubData()
	void doRecv(void *pix, const RectLoc &rloc, uint32_t /*pformat*/, uint32_t /*ptype*/) override
	{
		warnEmptyBuffer();
		if (m_size > 0) {
			F(glBindBuffer, GL_TEXTURE_BUFFER, m_bufferId);
			void *ptr = reinterpret_cast<char *>(pix) + rloc.dx;
			F(glGetBufferSubData, GL_TEXTURE_BUFFER, rloc.sx, rloc.x, ptr);
		}
	}
#endif
	void doReport() override
	{
		aux_printf("texture buffer [ %d ]\n", m_handle.id);
		aux_printf("\tbuffer_id  %d\n", m_bufferId);
		aux_printf("\tsize       %d\n", m_size);
	}

	void doSet() override
	{
		if (m_size != 0) {
			F(glBindBuffer, GL_TEXTURE_BUFFER, m_bufferId);
			F(glBufferData, GL_TEXTURE_BUFFER, m_size, nullptr, GL_STATIC_DRAW);
			m_isAllocated = true;
		}
		F(glBindTexture, GL_TEXTURE_BUFFER, m_handle.id);
		F(glTexBuffer, GL_TEXTURE_BUFFER, m_iformat, m_bufferId);
	}

	void doGet() override
	{
		int32_t int_val;
		F(glGetTexLevelParameteriv, m_handle.target, 0, GL_TEXTURE_INTERNAL_FORMAT, &int_val);
		m_iformat = int_val;
	}

	void doCopy(const TextureObject *src, const RectLoc &rloc) override
	{
		textureError(rloc.dl != 0 || rloc.l != 0, "cannot copy multiple levels\n");

		auto src_handle = src->handle();
		auto src_target = src_handle.target;

		switch (src_target) {
		case GL_TEXTURE_1D:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_3D:
		case GL_TEXTURE_RECTANGLE: {
			auto src_texinfo = spu_gl_texinfo(src->m_iformat);
			auto level = rloc.sl;
			auto pformat = src_texinfo.pformat;
			auto ptype = src_texinfo.ptype;
			auto bpp = spu_gl_texinfo(pformat, ptype).bit_per_pixel / 8;

			textureError(level != 0, "source level must be zero\n");
			textureError(
			        uint32_t(m_size) != src->m_width * src->m_height * src->m_depth * bpp,
			        "copy area mismatch\n");

			F(glBindBuffer, GL_PIXEL_PACK_BUFFER, m_bufferId);

			// alocate only
			F(glBufferData, GL_PIXEL_PACK_BUFFER, m_size, nullptr, GL_STATIC_DRAW);
			F(glBindTexture, src_target, src_handle.id);
			F(glGetTexImage, src_target, level, pformat, ptype, 0);
			F(glBindTexture, src_target, 0);
			F(glBindBuffer, GL_PIXEL_PACK_BUFFER, 0);
			break;
		}
		case GL_TEXTURE_BUFFER: {
			textureError(m_size != src->m_size, "copy area mismatch\n");

			F(glBindBuffer, GL_PIXEL_PACK_BUFFER, m_bufferId);
			F(glBindBuffer, GL_TEXTURE_BUFFER, src->m_bufferId);
			F(glCopyBufferSubData, GL_TEXTURE_BUFFER, GL_PIXEL_PACK_BUFFER, 0, 0, m_size);
			F(glBindBuffer, GL_TEXTURE_BUFFER, 0);
			F(glBindBuffer, GL_PIXEL_PACK_BUFFER, 0);
			break;
		}
		default: aux_error(true, "unsupported target [%s]\n", opengl_const(src_target));
		}
	}

	void warnEmptyBuffer() const
	{
		if (m_size == 0) {
			spu_message(0, "texbuf size is zero. (try spu_texture_set() to reset)");
		}
	}

	void warnFormat(const spu_gl_texinfo_t &texinfo) const
	{
		if (texinfo.iformat != m_iformat) {
			spu_message(
			        0, "%2d: cannot convert format. use %s\n", m_handle.id,
			        opengl_const(m_iformat));
		}
	}
};
}  // namespace spu::libspu::spu_texture
