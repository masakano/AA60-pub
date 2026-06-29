//
// TonemapInspector :
//
#pragma once
#include <gsys/canvas/tonemap.h>
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_canvas {

class TonemapInspector : public gs_node::gui::Tweakbar {
public:
	using base_t = gs_node::gui::Tweakbar;

	TonemapInspector(Tonemap *canvas) : m_canvas(canvas)
	{
		const std::vector<gs_node::gui::Menu::Item> tonemap_items = {
		        {"none",     Tonemap::e_none    },
                        {"linear",   Tonemap::e_linear  },
		        {"reinhard", Tonemap::e_reinhard},
                        {"hugo",     Tonemap::e_hugo    },
		        {"filmic",   Tonemap::e_filmic  },
		};

		base_t::setName(padstr(canvas->prettyName(), 32));
		base_t::addStdSlider("input gain", 0.0, 1.0, &canvas->ub_connect.sources[0].gain);
		base_t::addStdSlider("adapt spped", 0.0, 1.0, &canvas->m_adaptSpeed, 2.0);
		base_t::addStdSlider("min luminance", 0.01, 0.2, &canvas->u_tonemap_min_luminance, 2.0);
		base_t::addStdButton("adaptive exposure", &canvas->u_tonemap_is_adaptive_exposure);
		base_t::addStdButton("vignette", &canvas->u_tonemap_is_vignette);
		base_t::addMenus("type", {tonemap_items}, {&canvas->u_tonemap_type});
		base_t::bake();
	}

protected:
	Tonemap *m_canvas = nullptr;
};

}  // namespace spu::gs_canvas
