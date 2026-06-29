//
// 
//
#pragma once
#include <gsys/painter/fur_pbr.h>
#include "pbr_inspector.h"

namespace spu::gs_painter {
class FurPBRInspector : public PBRInspector {
public:
	using base_t = PBRInspector;
	float m_lineWidth = 1.0;

	FurPBRInspector(FurPBR *painter) : PBRInspector(painter)  // short format
	{
		auto &ub_fur = painter->ub_fur;

		base_t::setName(padstr(painter->prettyName(), 32));
		base_t::addStdSlider("line_width", 1.0, 6.0, &m_lineWidth);

		base_t::addStdSlider("fur height", 0.0, 1.0, &ub_fur.height);
		base_t::addStdSlider("position jitter", 0.0, 1.0, &ub_fur.position_jitter_scale);
		base_t::addStdSlider("normal jitter", 0.0, 1.0, &ub_fur.normal_jitter_scale);
		base_t::addStdSlider("heigh jitter", 0.0, 1.0, &ub_fur.height_jitter_scale);
		base_t::bake();
	}
	void doReload() override
	{

		m_lineWidth = m_painter->getDrawcalls().at(0).line_width;
	}

	void doUpdate() override
	{
		for (auto &drawcall: m_painter->getDrawcalls()) {
			drawcall.line_width = m_lineWidth;
		}
	}

};
}  // namespace spu::gs_painter
