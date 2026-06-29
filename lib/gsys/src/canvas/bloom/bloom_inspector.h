//
// BloomInspector :
//
#pragma once

#include <gsys/canvas/bloom.h>
#include <gsys/src/canvas/tonemap/tonemap_inspector.h>

namespace spu::gs_canvas {

class BloomInspector : public TonemapInspector {
public:
	using base = TonemapInspector;

	BloomInspector(Bloom *canvas) : TonemapInspector(canvas)
	{
		base_t::setName(padstr(canvas->prettyName(), 32));
		base_t::addStdSlider("bias", -16.0, 0.0, &canvas->m_bias, 2.0);
		base_t::addStdSlider("gain", 1.0, 4.0, &canvas->m_gain, 2.0);
		base_t::addStdSlider("variance", 0.0, 8.0, &canvas->m_variance, 2.0);
		base_t::addStdSlider("footstep", 1.0, 4.0, &canvas->m_footstep, 1.0);

		auto id0 = canvas->m_gauss2dH.getBuffer("color0").id();
		auto id1 = canvas->m_gauss2dQ.getBuffer("color0").id();
		base_t::addTexviews("bloom", {id0, id1}, {0, 0}, {0, 0}, 2);
		base_t::bake();
	}
};
}  // namespace spu::gs_canvas
