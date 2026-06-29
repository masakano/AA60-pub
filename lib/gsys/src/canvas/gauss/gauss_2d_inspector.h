//
// Gauss2DInspector :
//
#pragma once

#include <gsys/canvas/gauss.h>
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_canvas {

class Gauss2DInspector : public gs_node::gui::Tweakbar {
public:
	using base_t = gs_node::gui::Tweakbar;

	Gauss2DInspector(Gauss2D *canvas)
	{
		base_t::setName(canvas->prettyName());
		base_t::addStdSlider("variance", 0.0, 100.0, &canvas->u_variance, 4.0);
		base_t::bake();
	}
};
}  // namespace spu::gs_canvas
