//
// RealWaveGenerator :
//
#pragma once

#include "plane.h"
#include <gsys/shaders/painter/plane/ub_real_wave.us>

namespace spu::gs_painter {

class RealWaveGenerator {
public:
	static constexpr const float c_pi = pi();
	static constexpr const float c_2pi = 2.0 * pi();

	float m_gravity = 9.80665;  // standard gravitational acceleration
	float m_alpha = 0.0081;     // coefficient for Pierson-Moskowitz formula
	float m_omega0 = 10.0;      // area of the water surface part

	static float mitsuyasuDistribution(float f, float theta);
	float EPM(float f) const;  // Energy according to the Pierson-Moskowitz formula
	float energyDistribution(float f, float theta) const;
	float getAmplitude(float freq, float freq_peak, float theta, float k) const;
	float getLambda(float freq) const;
	int32_t generate(plane::UB_REAL_WAVE &ub_real_wave, float wind_speed, float wind_direction_deg);
};

class WaterPlane : public Plane {
public:
	int32_t u_use_realwater = false;

	float u_water_height_scale = 1.0 / 32.0;
	Mat4f u_water_maptexcoords[4];

	float u_time = 0;
	int32_t u_real_wave_count = 0;
	plane::UB_REAL_WAVE ub_real_wave;

	WaterPlane(const char *name = nullptr) : Plane(name) {}
	WaterPlane(const Attrs &attrs) { init(attrs); }
	void init(const Attrs &attrs) override;
	void update() override;
	void startInspector() override;

private:
	SpuTexture m_waterHeightmap;
};
}  // namespace spu::gs_painter
