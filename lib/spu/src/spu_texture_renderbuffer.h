//
// TextureRenderbuffer :
//
#pragma once

#include "spu_texture_object.h"

namespace spu::libspu::spu_texture {

class TextureRenderbuffer : public TextureObject {
public:
	TextureRenderbuffer(const Attrs &attrs) : TextureObject(attrs)
	{
		if (m_handle.ui == 0) {
			F(glGenRenderbuffers, 1, &m_handle.ui);
			m_handle.target = GL_RENDERBUFFER;
		}
		set(attrs);
		assert(m_iformat != 0);
		{
			spu_gl_texinfo_t texinfo = spu_gl_texinfo(m_iformat);
			F(glRenderbufferStorageMultisample, m_handle.target, m_multisample, texinfo.iformat,
			  m_width, m_height);
		}
	}

	~TextureRenderbuffer() override
	{
		uint32_t id_ui = m_handle.id;
		F(glDeleteRenderbuffers, 1, &id_ui);
	}

	void bind() override
	{
		F(glBindRenderbuffer, GL_RENDERBUFFER, m_handle.id);
		// FRA(is_renderbuffer, glIsRenderbuffer, m_handle.id);
		auto is_renderbuffer = glIsRenderbuffer(m_handle.id);
		if (is_renderbuffer == 0) {
			warnNotExist();
		}
	}

	void update() override {}

private:
	bool doUse(uint32_t /*slot*/) override { textureError("cannot bind"); }

	void doSend(const void * /*pix*/, const RectLoc & /*rloc*/, uint32_t /*pformat*/, uint32_t /*ptype*/)
	        override
	{
		spu_message(0, "do_send in renderbuffer (id=%d) (ignore)\n", m_handle.id);
	}

	void doRecv(void * /*pix*/, const RectLoc & /*rloc*/, uint32_t /*pformat*/, uint32_t /*ptype*/) override
	{
		spu_message(0, "do_recv in renderbuffer (id=%d) (ignore)\n", m_handle.id);
	}

	void doReport() override
	{
		aux_printf("render buffer [ %d ]\n", m_handle.id);
		aux_printf("\twidth  %d\n", m_width);
		aux_printf("\theight %d\n", m_height);
		aux_printf("\tformat %s\n", opengl_const(m_iformat));
	}

	void doSet() override
	{
		if (!m_isAllocated) {
			spu_gl_texinfo_t texinfo = spu_gl_texinfo(m_iformat);
			F(glRenderbufferStorageMultisample, GL_RENDERBUFFER, m_multisample, texinfo.iformat,
			  m_width, m_height);
			m_isAllocated = true;
		}
	}

	void doGet() override
	{
		auto int_width = int32_t(m_width);
		auto int_height = int32_t(m_height);

		getParameter(GL_RENDERBUFFER_WIDTH, &int_width);
		getParameter(GL_RENDERBUFFER_HEIGHT, &int_height);

		m_width = int_width;
		m_height = int_height;
	}

	void doCopy(const TextureObject * /*src*/, const RectLoc & /*rloc*/) override
	{
		textureError("cannot copy");
	}

	static void getParameter(uint32_t key, int32_t *val)
	{
		int32_t val2;
		F(glGetRenderbufferParameteriv, GL_RENDERBUFFER, key, &val2);
		if (val2) {
			*val = val2;  // ignore if zero
		}
	}
};
}  // namespace spu::libspu::spu_texture
