//
// FogInspector :
//
#pragma once

#include <gsys/canvas/fog.h>
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_canvas {

class FogInspector : public gs_node::gui::Tweakbar {
public:
	using base_t = Tweakbar;

	FogInspector(Fog *canvas) : m_canvas(canvas)
	{
		base_t::setName(padstr(m_canvas->prettyName(), 32));  // expand
		base_t::addStdSlider("speed", 0.0, 1.5, &m_speed, 2.0);
		base_t::addStdSlider("far", 1.0, 64.0, &m_canvas->u_far, 2.0);
		base_t::addStdSlider("step", 16, 64, &m_canvas->u_step);

		base_t::addStdSlider("amplitude", 0.0, 1.0, &m_canvas->ub_fbm_fog.amplitude);
		base_t::addStdSlider("granularity", 1.0, 8.0, &m_canvas->ub_fbm_fog.granularity, 2);
		base_t::addStdSlider("octaves", 1.0, 4.0, &m_octaves);
		base_t::addStdSlider("lacunarity", 1.0, 4.0, &m_canvas->ub_fbm_fog.lacunarity);
		base_t::addStdSlider("octave decay", 0.0, 1.0, &m_canvas->ub_fbm_fog.octave_decay);
		base_t::addStdSlider("density min", 0.0, 1.0, &m_canvas->ub_fbm_fog.density_min);
		base_t::addStdSlider("density max", 0.0, 1.0, &m_canvas->ub_fbm_fog.density_max);
		base_t::bake();
	}

	void update() override
	{
		auto &ub_fbm_fog = m_canvas->ub_fbm_fog;
		ub_fbm_fog.octaves = m_octaves;
		ub_fbm_fog.speed = Vec3f(m_speed, 0, -m_speed);
		base_t::update();
	}

protected:
	Fog *m_canvas = nullptr;
	float m_speed = 0.05;
	float m_octaves = 4.0;
};
}  // namespace spu::gs_canvas
