//
// PlaneInspector :
//
#pragma once
#include <gsys/painter/plane.h>
#include "../pbr/pbr_inspector.h"

namespace spu::gs_painter {

class PlaneInspector : public PBRInspector {
public:
	using base_t = PBRInspector;
	PlaneInspector(Plane *painter) : PBRInspector(painter)
	{
		auto reflectmap = painter->m_canvas.getBuffer("color0").id();
		base_t::setName(painter->prettyName());
		base_t::addTexviews("reflect", {reflectmap}, {0}, {0}, 1);
		base_t::bake();
		//coreMonitor();
	}
};
}  // namespace spu::gs_painter
