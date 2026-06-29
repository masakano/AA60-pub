//
// DofInspector :
//
#pragma once
#include <gsys/canvas/dof.h>
#include <gsys/src/canvas/tonemap/tonemap_inspector.h>

namespace spu::gs_canvas {

class DofInspector : public TonemapInspector {
public:
	using base = TonemapInspector;

	DofInspector(Dof *canvas) : TonemapInspector(canvas)
	{
		float near, far;
		auto current = GsCanvas::getCurrent();
		// auto &c = current->getComposition();
		current->viewscreen().get_projection(nullptr, nullptr, &near, &far);

		base_t::setName(padstr(canvas->prettyName(), 28));
		base_t::addStdSlider("aperture radius", 0.0, 8.0, &canvas->u_aperture_radius, 2.0);
		base_t::addStdSlider("focal distance", near, far, &canvas->u_focal_distance, 2.0);
		base_t::addStdButton("show focal point", &canvas->u_show_focal_point);
		base_t::bake();
	}
};
}  // namespace spu::gs_canvas
