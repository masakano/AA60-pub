//
// CloudyIrradianceInspector :
//
#pragma once

#include "cloudy_irradiance.h"
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_canvas {

class CloudyIrradianceInspector : public gs_node::gui::Tweakbar {
public:
	using base_t = gs_node::gui::Tweakbar;

	CloudyIrradianceInspector(CloudyIrradiance *canvas) : m_canvas(canvas)
	{
		auto &sun = canvas->m_sun;
		auto &ub = canvas->ub_cloudy_irradiance;

		base_t::setName(padstr(canvas->prettyName(), 32));
		base_t::addStdSlider("sun hour", 0.0, 24.0, &sun.hour);
		base_t::addStdSlider("sun speed", -0.5, 0.5, &sun.speed);
		base_t::addStdSlider("speed", 0.0, 1.5, &canvas->m_speed, 2.0);
		base_t::addStdSlider("granularity", 5.0, 100.0, &ub.granularity, 2);
		base_t::addStdSlider("gain", 0.0, 1.0, &ub.gain);
		base_t::addStdSlider("density min", 0.0, 1.0, &ub.density_min);
		base_t::addStdSlider("density max", 0.0, 1.0, &ub.density_max);
		base_t::addStdSlider("density scale", 0.0, 1.0, &ub.density_scale);
		base_t::addStdSlider("cloud bottom", 1, 25, &ub.cloud_bottom);
		base_t::addStdSlider("cloud top", 1, 25, &ub.cloud_top);
		base_t::addStdSlider("sky cutoff angle", 0.85, 0.99, &ub.sky_cutoff_angle);
		base_t::addStdSlider("cloud fade angle", 0.02, 0.50, &ub.cloud_fade_angle);
		base_t::addStdButton("auto", &sun.is_auto_direction);
		bake();
	}

protected:
	CloudyIrradiance *m_canvas = nullptr;
};
}  // namespace spu::gs_canvas
