//
// WaterPlaneInspector :
//
#pragma once
#include <gsys/painter/water_plane.h>
#include "plane_inspector.h"

namespace spu::gs_painter {

class WaterPlaneInspector : public PlaneInspector {
public:
	using base_t = PlaneInspector;
	WaterPlaneInspector(WaterPlane *painter) : PlaneInspector(painter)
	{
		base_t::setName(painter->prettyName());
		base_t::addStdButton("use real water", &painter->u_use_realwater);
		base_t::bake();
	}
};
}  // namespace spu::gs_painter
