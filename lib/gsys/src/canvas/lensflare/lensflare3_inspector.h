//
// Lensflare3Inspector :
//
#pragma once

#include "../tonemap/tonemap_inspector.h"
#include <gsys/canvas/lensflare3.h>

namespace spu::gs_canvas {

class Lensflare3Inspector : public TonemapInspector {
public:
	using base = TonemapInspector;

	Lensflare3Inspector(Lensflare3 *canvas) : TonemapInspector(canvas)
	{
		base_t::setName(padstr(canvas->prettyName(), 36));
		base_t::addStdSlider("flare gain", 0.0, 4.0, &m_canvas->ub_connect.sources[1].gain);
		base_t::addStdSlider("flar bias", -8.0, 0.0, &canvas->m_bias, 2.0);
		base_t::addStdButton("filmic", &canvas->m_type);

		// blur
		{
			auto id0 = canvas->m_gaussCanvas0.getBuffer("color0").id();
			auto id1 = canvas->m_gaussCanvas1.getBuffer("color0").id();
			auto id2 = canvas->m_gaussCanvas2.getBuffer("color0").id();
			auto id3 = canvas->m_gaussCanvas3.getBuffer("color0").id();
			base_t::addTexviews("blur", {id0, id1, id2, id3}, {0, 0, 0, 0}, {0, 0, 0, 0}, 4);
		}

		// streak
		{
			auto id0 = canvas->m_streakFramesA[0].getBuffer("color0").id();
			auto id1 = canvas->m_streakFramesA[1].getBuffer("color0").id();
			auto id2 = canvas->m_streakFramesA[2].getBuffer("color0").id();
			auto id3 = canvas->m_streakFramesA[3].getBuffer("color0").id();

			base_t::addTexviews("streak", {id0, id1, id2, id3}, {0, 0, 0, 0}, {0, 0, 0, 0}, 4);
		}

		// ghost
		{
			auto id0 = canvas->m_cameraGhostFrame.getBuffer("color0").id();
			auto id1 = canvas->m_filmicGhostFrame.getBuffer("color0").id();
			base_t::addTexviews("ghost", {id0, id1}, {0, 0}, {0, 0}, 2);
		}
		base_t::bake();
	}
};
}  // namespace spu::gs_canvas
