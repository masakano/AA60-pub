//
// GlareInspector :
//
#pragma once
#include <gsys/src/canvas/tonemap/tonemap_inspector.h>
#include <gsys/canvas/glare.h>
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_canvas {

class GlareInspector : public TonemapInspector {
public:
	using base = TonemapInspector;

	GlareInspector(Glare *canvas) : TonemapInspector(canvas)
	{
		base_t::setName(padstr(canvas->prettyName(), 32));
		base_t::addStdSlider("source gain", 0.0, 1.0, &m_canvas->ub_connect.sources[0].gain);
		base_t::addStdSlider("glare gain", 0.0, 1.0, &m_canvas->ub_connect.sources[1].gain);
		base_t::addStdSlider("bias", -16.0, 0.0, &canvas->m_bias, 2.0);
		base_t::addStdSlider("gain", 1.0, 4.0, &canvas->m_gain, 2.0);
		base_t::addStdSlider("variance", 0.0, 32.0, &canvas->m_variance, 2.0);
		base_t::addStdSlider("footstep", 1.0, 4.0, &canvas->m_footstep, 1.0);

		auto id0 = canvas->m_gauss2dH.getBuffer("color0").id();
		auto id1 = canvas->m_gauss2dQ.getBuffer("color0").id();
		base_t::addTexviews("bloom", {id0, id1}, {0, 0}, {0, 0}, 2);
		base_t::bake();
	}
};
}  // namespace spu::gs_canvas
