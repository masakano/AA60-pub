//
// Analizer :
//
#include "analizer.h"

namespace spu::gs_canvas::irradiance {

Analizer::Analizer(const std::vector<Vec3f> &pixels, uint32_t width, uint32_t height)
        : m_pixels(pixels), m_width(width), m_height(height)
{
	calcPeakdir();
	calcEmissionAndAmbient();
}

void Analizer::calcPeakdir()
{
	const auto RGB2Y = Vec3f(0.29900, 0.58700, 0.11400);
	auto max_luminance = dot(RGB2Y, m_pixels[0]);
	auto ix = 0u;
	auto iy = 0u;

	for (auto y = 0u; y < m_height; y++) {
		for (auto x = 0u; x < m_width; x++) {
			auto &pixel = m_pixels[y * m_width + x];
			auto luminance = dot(RGB2Y, pixel);
			if (max_luminance < luminance) {
				max_luminance = luminance;
				ix = x;
				iy = y;
			}
		}
	}
	auto peak_texcoord = Vec2f(float(ix) / float(m_width), float(iy) / float(m_height));
	auto phi = (peak_texcoord.x - 0.5f) * 2.0f * pi();
	auto theta = (peak_texcoord.y - 0.5f) * pi();
	m_peakdir = Vec3f(cosf(theta) * sinf(phi), sinf(theta), -cosf(theta) * cosf(phi));
	if (m_peakdir.y < 0) {
		aux_message(0, "peakdir.y is corrected from %f to 1.0\n", m_peakdir.y);
		m_peakdir = normalize(Vec3f(m_peakdir.x, 1.0, m_peakdir.z));
	}
}

void Analizer::calcEmissionAndAmbient()
{
	auto sum_emission = ezero<Vec4f>();
	auto sum_ambient = ezero<Vec4f>();

	for (auto y = 0u; y < m_height; y++) {
		auto theta = (float(y) / float(m_height) - 0.5f) * pi();
		auto sin_theta = sinf(theta);
		auto cos_theta = cosf(theta);

		for (auto x = 0u; x < m_width; x++) {
			auto phi = (float(x) / float(m_width) - 0.5f) * 2.0f * pi();
			auto sin_phi = sinf(phi);
			auto cos_phi = cosf(phi);
			auto raydir = Vec3f(cos_theta * sin_phi, sin_theta, -cos_theta * cos_phi);

			auto NdotL = std::max(dot(raydir, m_peakdir), 0.0f);

			auto irradiance = Vec3f(m_pixels[y * m_width + x]);

			auto emission_weight = NdotL * cos_theta;
			auto ambient_weight = (1.0f - NdotL) * cos_theta;

			sum_emission += Vec4f(irradiance, emission_weight);
			sum_ambient += Vec4f(irradiance, ambient_weight);
		}
	}
	m_emission = sum_emission / sum_emission.w;
	m_ambient = sum_ambient / sum_ambient.w;
}
}  // namespace spu::gs_canvas::irradiance
