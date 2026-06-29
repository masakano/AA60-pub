//
// TextureSampler :
//
#pragma once

#include "spu_texture_object.h"

namespace spu::libspu::spu_texture {

class TextureSampler : public TextureObject {
public:
	TextureSampler(const Attrs &attrs) : TextureObject(attrs)
	{
		if (m_handle.ui == 0) {
			F(glGenSamplers, 1, &m_handle.ui);
			m_handle.target = GL_TEXTURE_SAMPLER;
		}
		set(attrs);
	}

	~TextureSampler() override
	{
		uint32_t id_ui = m_handle.id;
		F(glDeleteSamplers, 1, &id_ui);
	}

	void bind() override {}

	void update() override { textureError("cannot update"); }

private:
	bool doUse(uint32_t slot) override
	{
		F(glBindSampler, slot, m_handle.id);  // no check for speed
		return true;
	}

	void doSend(
	        const void *pix, const RectLoc & /*rloc*/, uint32_t /*pformat*/, uint32_t /*ptype*/) override
	{
		if (pix) {
			spu_message(0, "do_send in sampler (id=%d) (ignore)\n", m_handle.id);
		}
	}

	void doRecv(void *pix, const RectLoc & /*rloc*/, uint32_t /*pformat*/, uint32_t /*ptype*/) override
	{
		if (pix) {
			spu_message(0, "do_recv in sampler (id=%d) (ignore)\n", m_handle.id);
		}
	}

	void doReport() override
	{
		aux_printf("texture [ %d GL_TEXTURE_SAMPLER]\n", m_handle.id);
		aux_printf("\twrap_s      %s\n", opengl_const(m_wrapS));
		aux_printf("\twrap_t      %s\n", opengl_const(m_wrapT));
		aux_printf("\twrap_r      %s\n", opengl_const(m_wrapR));
		aux_printf("\tmag_filter  %s\n", opengl_const(m_magFilter));
		aux_printf("\tmin_filter  %s\n", opengl_const(m_minFilter));
		aux_printf("\tcomp_mode   %s\n", opengl_const(m_compMode));
		aux_printf("\tcomp_func   %s\n", opengl_const(m_compFunc));
		aux_printf("\tmin_lod     %f\n", m_minLod);
		aux_printf("\tmax_lod     %f\n", m_maxLod);
		aux_printf("\tlod_bias    %f\n", m_lodBias);
		aux_printf("\tmax_aniso   %f\n", m_maxAniso);
		aux_printf("\tborder      %f,%f,%f,%f\n", m_border.x, m_border.y, m_border.z, m_border.w);
		aux_printf("\tauto_mipmap %d\n", m_isAutoMipmap);
	}

	void doSet() override
	{
		setParameter(GL_TEXTURE_MIN_FILTER, &m_minFilter);
		setParameter(GL_TEXTURE_MAG_FILTER, &m_magFilter);
		setParameter(GL_TEXTURE_WRAP_S, &m_wrapS);
		setParameter(GL_TEXTURE_WRAP_T, &m_wrapT);
		setParameter(GL_TEXTURE_WRAP_R, &m_wrapR);
		setParameter(GL_TEXTURE_COMPARE_MODE, &m_compMode);
		setParameter(GL_TEXTURE_COMPARE_FUNC, &m_compFunc);
		setParameter(GL_TEXTURE_MIN_LOD, &m_minLod);
		setParameter(GL_TEXTURE_MAX_LOD, &m_maxLod);
		setParameter(GL_TEXTURE_LOD_BIAS, &m_lodBias);
		setParameter(GL_TEXTURE_MAX_ANISOTROPY, &m_maxAniso);
		setParameter(GL_TEXTURE_BORDER_COLOR, &m_border.x);
	}

	void doGet() override
	{
		getParameter(GL_TEXTURE_MIN_FILTER, &m_minFilter);
		getParameter(GL_TEXTURE_MAG_FILTER, &m_magFilter);
		getParameter(GL_TEXTURE_WRAP_S, &m_wrapS);
		getParameter(GL_TEXTURE_WRAP_T, &m_wrapT);
		getParameter(GL_TEXTURE_WRAP_R, &m_wrapR);
		getParameter(GL_TEXTURE_COMPARE_MODE, &m_compMode);
		getParameter(GL_TEXTURE_COMPARE_FUNC, &m_compFunc);
		getParameter(GL_TEXTURE_MIN_LOD, &m_minLod);
		getParameter(GL_TEXTURE_MAX_LOD, &m_maxLod);
		getParameter(GL_TEXTURE_LOD_BIAS, &m_lodBias);
		getParameter(GL_TEXTURE_MAX_ANISOTROPY, &m_maxAniso);
		getParameter(GL_TEXTURE_BORDER_COLOR, &m_border.x);
	}

	void doCopy(const TextureObject * /*src*/, const RectLoc & /*rloc*/) override
	{
		textureError("cannot copy");
	}

	void setParameter(int32_t key, uint32_t *val)
	{
		if (val) {
			F(glSamplerParameteriv, m_handle.id, key, reinterpret_cast<int32_t *>(val));
		}
	}

	void setParameter(int32_t key, int32_t *val)
	{
		if (val) {
			F(glSamplerParameteriv, m_handle.id, key, val);
		}
	}

	void setParameter(int32_t key, float *val)
	{
		if (val) {
			F(glSamplerParameterfv, m_handle.id, key, val);
		}
	}

	void getParameter(int32_t key, uint32_t *val)
	{
		F(glGetSamplerParameteriv, m_handle.id, key, reinterpret_cast<int32_t *>(val));
	}

	void getParameter(int32_t key, int32_t *val) { F(glGetSamplerParameteriv, m_handle.id, key, val); }

	void getParameter(int32_t key, float *val) { F(glGetSamplerParameterfv, m_handle.id, key, val); }

	void get(int32_t key, float *val) { F(glGetSamplerParameterfv, m_handle.id, key, val); }
};
}  // namespace spu::libspu::spu_texture
