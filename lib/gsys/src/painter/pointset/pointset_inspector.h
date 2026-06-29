//
// PointsetInspector :
//
#pragma once

#include "../pbr/pbr_inspector.h"

namespace spu::gs_painter {

class PointsetInspector : public PBRInspector {
public:
	using base_t = PBRInspector;
	PointsetInspector(GsPainter *painter) : PBRInspector(painter)
	{
		base_t::setName(padstr(painter->name(), 32));
		base_t::addStdSlider("point size  ", 0.01, 0.25, &ub_material.point_size);
		base_t::addStdSlider("alpha       ", 0.01, 1.00, &ub_material.albedo.a);
		base_t::bake();
	}

protected:
	void doUpdate() override
	{
		for (auto &drawcall: m_painter->getDrawcalls()) {
			drawcall.ub_material.albedo.a = ub_material.albedo.a;
			drawcall.ub_material.point_size = ub_material.point_size;
		}
	}

};
}  // namespace spu::gs_painter
