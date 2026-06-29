//
// Lensflare1Inspector :
//
#pragma once

#include <gsys/canvas/lensflare1.h>
#include <gsys/src/canvas/tonemap/tonemap_inspector.h>

namespace spu::gs_canvas {

class Lensflare1Inspector : public TonemapInspector {
public:
	using base = TonemapInspector;

	Lensflare1Inspector(Lensflare1 *canvas) : TonemapInspector(canvas)
	{
		base_t::setName(padstr(canvas->prettyName(), 32));
		base_t::addStdSlider("flare gain", 0.0, 4.0, &canvas->ub_connect.sources[1].gain);
		base_t::addStdSlider("flare bias", -16.0, 0.0, &canvas->m_bias, 2.0);
		base_t::addStdSlider("variance H", 0.0, 8.0, &canvas->m_varianceH, 2.0);
		base_t::addStdSlider("variance F", 0.0, 16.0, &canvas->m_varianceF, 2.0);

		base_t::addStdSlider("dispersal  ", 0.0, 0.4, &canvas->u_dispersal, 2.0);
		base_t::addStdSlider("halo width ", 0.0, 1.0, &canvas->u_halo_width, 2.0);

		base_t::bake();
	}
};
}  // namespace spu::gs_canvas
