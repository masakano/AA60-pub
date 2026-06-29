//
// LoupeBirdview :
//
#pragma once

#include <gsys/node/gui/tweakbar.h>
#include <gsys/canvas/copy.h>

namespace spu::gs_node::gui {

class LoupeBirdview : public Birdview {
public:
	explicit LoupeBirdview(const char *name = nullptr) : Birdview(name) {}
	explicit LoupeBirdview(const Attrs &attrs) : LoupeBirdview() { init(attrs); }

	void init(const Attrs &attrs) override
	{
		Birdview::init(attrs);

		auto span = getARange().span();

		m_tx = span.x * 0.5;  // ad-hoc
		m_ty = span.y * 0.5;
		Attrs texture_attrs = {
		        {"target",     GL_TEXTURE_2D},
                        {"iformat",    GL_RGBA8     },
                        {"min_filter", GL_NEAREST   },
		        {"mag_filter", GL_NEAREST   },
                        {"width",      m_tx         },
                        {"height",     m_ty         },
		};
		m_texture.init(texture_attrs);
		m_copyCanvas.init(Attrs());
		m_copyCanvas.u_color0 = m_texture.id();
	}

protected:
	SpuTexture m_texture;
	gs_canvas::Copy m_copyCanvas;

	int32_t m_tx = 0;
	int32_t m_ty = 0;

	void coreDraw() override
	{
		auto &camera = getCamera();
		auto &curr = camera.getGesture()->curr();

		auto wx = curr.winsize[0];
		auto wy = curr.winsize[1];

		auto ix = curr.cursor[0] - m_tx / 2;
		auto iy = curr.cursor[1] - m_ty / 2;

		ix = std::clamp(ix, 0, wx - m_tx);
		iy = std::clamp(iy, 0, wy - m_ty);

		int32_t dst_loc[] = {0, 0, 0, 0};
		int32_t src_loc[] = {ix, iy, 0, 0};
		uint32_t size[] = {uint32_t(m_tx), uint32_t(m_ty), 1u, 1u};

		spu_texture_copy(m_texture.id(), 0, dst_loc, src_loc, size);
		m_copyCanvas.takeover();
		m_copyCanvas.render();
	}
};
}  // namespace spu::gs_node::gui
