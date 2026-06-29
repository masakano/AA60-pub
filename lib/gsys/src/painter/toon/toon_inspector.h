//
// ToonInspector :
//
#pragma once
#include <gsys/node/gui/tweakbar.h>
#include <gsys/painter/toon.h>

namespace spu::gs_painter {

class ToonInspector : public gs_node::gui::Tweakbar {
public:
	using base_t = gs_node::gui::Tweakbar;

	ToonInspector(gs_painter::Toon *painter) : m_painter(painter)
	{
		base_t::setName(m_painter->name());
		base_t::addStdSlider("toon threshold", 0.0, 1.0, &m_painter->u_toon_threshold);
		base_t::addStdSlider("toon ao", 0.0, 1.0, &m_painter->u_toon_ao);
		base_t::addStdSlider("edge width", 0.00, 0.050, &m_painter->u_edge_width, 2.0);
		base_t::addStdSlider("edge mix rate", 0.0, 1.0, &m_painter->u_edge_mix_rate);
		base_t::addStdColorSlider("edge color", &m_painter->u_edge_color);
		base_t::bake();
		// enableMaterial();
	}
#if 0
	void update() override
	{
		if (base_t::isFocus()) {
			auto touch_count = base_t::lastUpdateCount();
			base_t::update();
			if (touch_count != base_t::lastUpdateCount()) {
				coreUpdate();
			}
		}
	}
#endif
protected:
	// std::vector<GsDrawcall> m_drawcalls;
	gs_painter::Toon *m_painter = nullptr;
	// int32_t m_isOverrideMaterial = false;
	// Vec4f m_albedo = eone<Vec4f>();
	// float m_ao = 0.5;
#if 0
	void coreUpdate()
	{
		enableMaterial();
#if 0
		auto &drawcalls = m_painter->getDrawcalls();
		drawcalls = m_drawcalls;  // reset

		for (auto &drawcall: drawcalls) {
			if (m_isOverrideMaterial) {
				drawcall.ub_material.albedo = m_albedo;
				drawcall.ub_material.ao = m_ao;
			}
		}
#endif		
	}
#endif
#if 0	
	void enableMaterial()
	{
		auto state = m_isOverrideMaterial ? e_active : e_disabled;
		std::vector<const void *> ptrs = {
		        &m_ao, &m_albedo.r, &m_albedo.g, &m_albedo.b, &m_albedo.a,
		};
		base_t::changeState(ptrs, state);
	}
#endif
};
}  // namespace spu::gs_painter
