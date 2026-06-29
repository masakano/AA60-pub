//
// TextureNull :
//
#pragma once

#include "spu_texture_object.h"

namespace spu::libspu::spu_texture {

class TextureNull : public TextureObject {
public:
	TextureNull(const Attrs &attrs) : TextureObject(attrs) { set(attrs); }

	~TextureNull() override = default;

	void bind() override {}
	void update() override {}

private:
	bool doUse(uint32_t slot) override
	{
		if (m_handle.target == GL_TEXTURE_SAMPLER) {
			F(glBindSampler, slot, 0);
		}
		else {
			F(glActiveTexture, GL_TEXTURE0 + slot);
			F(glBindTexture, GL_TEXTURE_2D, 0);
			F(glBindSampler, slot, 0);
		}
		return true;
	}

	void doSend(const void *pix, const RectLoc &rloc, uint32_t pformat, uint32_t ptype) override
	{
		textureError(m_handle.target != 0u, "send to bad target");
		checkFullSize(rloc);
		if (pix) {
			doGet();  // update size
			F(glDrawPixels, m_width, m_height, pformat, ptype, pix);
		}
	}

	void doRecv(void *pix, const RectLoc &rloc, uint32_t pformat, uint32_t ptype) override
	{
		textureError(m_handle.target != 0u, "recv to bad target");
		if (pix && rloc.sl == 0) {  // level #0 only
			doGet();            // update size
			F(glReadPixels, rloc.sx, rloc.sy, rloc.x, rloc.y, pformat, ptype, pix);
		}
	}

	void doReport() override
	{
		doGet();  // update size
		aux_printf("display buffer [ %d ]\n", m_handle.ui);
		aux_printf("\twidth  %d\n", m_width);
		aux_printf("\theight %d\n", m_height);
	}

	void doSet() override { doGet(); }

	void doGet() override
	{
		SpuPad *pad = nullptr;
		spu_graphics_get("pad", &pad);
		m_width = pad->winsize[0];
		m_height = pad->winsize[1];
	}

	void doCopy(const TextureObject * /*src*/, const RectLoc & /*rloc*/) override
	{
		textureError("cannot copy");
	}
};
}  // namespace spu::libspu::spu_texture
