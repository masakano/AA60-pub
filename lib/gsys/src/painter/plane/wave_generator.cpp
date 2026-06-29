//
// RealWaveGenerator :
//
#include <gsys/painter/water_plane.h>
#include <ssys/random_generator.h>

namespace spu::gs_painter {

float RealWaveGenerator::mitsuyasuDistribution(float f, float theta)
{
	float s;
	float ang = cosf(theta / 2);
	ang *= ang;  // calculate square of cos(theta/2)

	if (f > 1.0) {
		s = 9.77f * powf(f, -2.5f);
	}
	else {
		s = 6.97f * powf(f, 5.0f);
	}
	return gammaf(s + 1) * powf(ang, s) / (2 * sqrtf(c_pi) * gammaf(s + 0.5));
}

float RealWaveGenerator::EPM(float f) const  // Energy according to the Pierson-Moskowitz formula
{
	return m_alpha * m_gravity * m_gravity * exp(-1.25 * powf(f, -4)) / powf(c_2pi, 4);
}

float RealWaveGenerator::energyDistribution(float f, float theta) const
{
	return EPM(f) * mitsuyasuDistribution(f, theta);
}

float RealWaveGenerator::getAmplitude(float freq, float freq_peak, float theta, float k) const
{
	return sqrtf(
	        energyDistribution(freq / freq_peak, theta) * m_gravity * c_pi * c_pi / (k * freq * m_omega0));
}

float RealWaveGenerator::getLambda(float freq) const { return c_2pi / (powf(c_2pi * freq, 2.0) / m_gravity); }

int32_t RealWaveGenerator::generate(
        plane::UB_REAL_WAVE &ub_real_wave, float wind_speed, float wind_direction_deg)
{
	RandomGenerator<float> frand;

	auto wind_direction = wind_direction_deg * c_pi / 180.0f;
	auto freq_peak = 0.13f * m_gravity / wind_speed;  // calculate peak frequency

	// m_waves.clear();
	auto real_wave_count = 0;
	auto amp_max = 0.0f;

	while (real_wave_count < def_ub_real_wave_max) {
		auto freq = freq_peak * frand();  // need normal distribution

		if (freq > 0.0) {
			plane::UB_REAL_WAVE_ELEMENT element;

			auto lambda = c_2pi / (powf(c_2pi * freq, 2.0f) / m_gravity);
			auto k = c_2pi / lambda;
			auto omega = c_2pi * freq;

			element.omega = omega;
			element.direction = (frand() + 0.5) * c_pi + wind_direction;  // radian
			element.dirX = cosf(element.direction);
			element.dirY = sinf(element.direction);

			auto phi = frand() * c_pi;
			auto amplitude = getAmplitude(freq, freq_peak, element.direction - wind_direction, k);

			element.amplitude = amplitude * cosf(phi);
			element.phase = amplitude * sinf(phi);

			if (fabsf(element.amplitude) >= 0.0001f) {
				ub_real_wave.elements[real_wave_count++] = element;
			}
			amp_max = std::max(amp_max, amplitude);
		}
	}
	return real_wave_count;
}
}  // namespace spu::gs_painter
