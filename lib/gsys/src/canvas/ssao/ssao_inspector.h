//
// SSAOInspector :
//
#pragma once

#include <gsys/canvas/ssao.h>
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_canvas {

class SSAOInspector : public gs_node::gui::Tweakbar {
public:
	using base_t = gs_node::gui::Tweakbar;
	SSAOInspector(SSAO *ssao_canvas)
	{
		const std::vector<gs_node::gui::Menu::Item> quality_items = {
		        {"low",     0},
		        {"mid",     1},
		        {"high",    2},
		        {"highest", 3},
		};

		base_t::setName(ssao_canvas->prettyName());
		base_t::addStdSlider("ssao lerp ", 0.0, 1.0, &ssao_canvas->u_ssao_lerp);
		base_t::addStdSlider("ssao power", 1.0, 3.0, &ssao_canvas->u_ssao_power, 2.0);
		base_t::addStdSlider("sample radius", 0.002, 0.25, &ssao_canvas->u_sample_radius, 2.0);
		base_t::addStdButton("ssao only", &ssao_canvas->u_ssao_only);
		base_t::addMenus("quality", {quality_items}, {&ssao_canvas->m_quality});

		base_t::bake();
	}
};
}  // namespace spu::gs_canvas
