//
// Resolver :
//
#pragma once
#include "spu_frame_viewport.h"

namespace spu::libspu::spu_frame {

#if 0
class Resolver final {
public:
	~Resolver()
	{
	}

	void attach(const Handle &dst_texture, const Handle &src_texture, int32_t attach_point)
	{
	}

	void resolve(uint32_t src_frame_id)
	{
	}

	void report() const
	{
#if 0
		aux_printf(
		        "\t%-16s : %08x %s\n", "resolver", m_textureId,
		        m_attachPoint ? opengl_const(m_attachPoint) : "");
#endif
	}

private:
};
#endif

class Attach final {
public:
	Attach(const hash32_t &key, int32_t attach_point, const Handle &texture)
	        : m_key(key), m_texture(texture), m_attachPoint(attach_point)
	{
	}

	const hash32_t &key() const { return m_key; }

	uint32_t get() const { return m_texture.ui; }
	void update() const { spu_texture_update(m_texture.ui); }

	bool isClearable(int32_t layer) const
	{
		if (m_texture.target == GL_TEXTURE_CUBE_MAP || m_texture.target == GL_TEXTURE_CUBE_MAP_ARRAY) {
			if (layer == -1) {
				return false;
			}
		}
		return true;
	}

	void attach(int32_t layer, int32_t level)
	{
		m_layer = layer;
		m_level = level;

		switch (m_texture.target) {
		case GL_RENDERBUFFER:
			F(glFramebufferRenderbuffer, GL_FRAMEBUFFER, m_attachPoint, GL_RENDERBUFFER,
			  m_texture.id);
			return;
		case GL_TEXTURE_2D:
		case GL_TEXTURE_2D_MULTISAMPLE:
		case GL_TEXTURE_RECTANGLE:
			F(glFramebufferTexture2D, GL_FRAMEBUFFER, m_attachPoint, m_texture.target, m_texture.id,
			  m_level);
			return;
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_CUBE_MAP: {
			if (m_layer >= 0) {
				F(glFramebufferTextureLayer, GL_FRAMEBUFFER, m_attachPoint, m_texture.id,
				  m_level, m_layer);
			}
			else {
				F(glFramebufferTexture, GL_FRAMEBUFFER, m_attachPoint, m_texture.id, m_level);
			}
			return;
		}
		case GL_TEXTURE_3D:
			if (m_layer >= 0) {
				F(glFramebufferTexture3D, GL_FRAMEBUFFER, m_attachPoint, m_texture.target,
				  m_texture.id, m_level, m_layer);
			}
			else {
				F(glFramebufferTexture, GL_FRAMEBUFFER, m_attachPoint, m_texture.id, m_level);
			}
			return;
		default: aux_error(true, "invalid target [%s]\n", opengl_const(m_texture.target));
		}
	}

	void report()
	{
		uint32_t iformat;
		spu_texture_get(m_texture.ui, "iformat", &iformat);
		aux_printf("\t%-16s : %08x %2d %2d ", m_key.c_str(), m_texture.ui, m_layer, m_level);
		aux_printf("%-16s ", opengl_const(m_texture.target));
		aux_printf("%s\n", opengl_const(iformat));
	}

private:
	hash32_t m_key;
	Handle m_texture;
	int32_t m_attachPoint = 0;
	int32_t m_layer = -1;
	int32_t m_level = 0;
};
}  // namespace spu::libspu::spu_frame
