//
// Lensflare2Inspector :
//
#pragma once

#include <gsys/canvas/lensflare2.h>
#include <gsys/src/canvas/tonemap/tonemap_inspector.h>

namespace spu::gs_canvas {

class Lensflare2Inspector : public TonemapInspector {
public:
	using base = TonemapInspector;

	Lensflare2Inspector(Lensflare2 *canvas) : TonemapInspector(canvas)
	{
		const std::vector<gs_node::gui::Menu::Item> output_items = {
		        {"source", 0},
		        {"bloom",  1},
		        {"flare",  2},
		        {"all",    3},
		};

		base_t::setName(padstr(canvas->prettyName(), 32));

		base_t::addStdSlider("bloom bias", -16.0, 0.0, &canvas->m_bloomBias);
		base_t::addStdSlider("bloom gain", 0.0, 1.0, &canvas->m_bloomGain);
		base_t::addStdSlider("bloom blur", 0.0, 128.0, &canvas->m_bloomBlurRadius);

		base_t::addStdSlider("flare bias", -16.0, 0.0, &canvas->m_flareBias);
		base_t::addStdSlider("flare gain", 0.0, 1.0, &canvas->m_flareGain);
		base_t::addStdSlider("flare blur", 0.0, 128.0, &canvas->m_flareBlurRadius);

		base_t::addStdSlider("flare samples", 1.0, 24.0, &canvas->m_flareSamples);
		base_t::addStdSlider("flare dispersal", 0.0, 0.2, &canvas->m_flareDispersal);
		base_t::addStdSlider("flare halo width", 0.0, 10.0, &canvas->m_flareHaloWidth);
		base_t::addStdSlider("flare distortion", 0.0, 16.0, &canvas->m_flareDistortion);

		base_t::addMenus("output", {output_items}, {&canvas->m_outputMode});

		// bloom view
		base_t::addTexviews("bloom", {canvas->m_bloomCanvas.getBuffer("color0").id()}, {0}, {0}, 1);

		// flare view
		auto id0 = canvas->m_flareCanvas.getBuffer("color0").id();
		auto id1 = canvas->m_scaleBiasCanvas.getBuffer("color0").id();
		base_t::addTexviews("flare", {id0, id1}, {0, 0}, {0, 0}, 2);

		base_t::bake();
	}
};
}  // namespace spu::gs_canvas
