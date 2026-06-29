//
// GsLambertToPBR :
//
#include <gsys/util/lambert_to_pbr.h>

namespace spu {
GsLambertToPBR::GsLambertToPBR(
        const Vec3f &diffuse, const Vec3f &ambient, const Vec3f &specular, const float specular_power)
{
	m_diffuse = length(diffuse);
	m_ambient = length(ambient);
	m_specular = length(specular);
	m_specularPower = specular_power;
}

float GsLambertToPBR::roughness() const
{
	return std::clamp(powf(2.0f / (m_specularPower + 2.0f), 0.25f), 0.01f, 0.99f);
}

float GsLambertToPBR::metallic() const
{
	return std::clamp(m_diffuse > 0 ? m_specular / m_diffuse : 0.0f, 0.01f, 0.99f);
}

float GsLambertToPBR::ao() const
{
	return std::clamp(m_diffuse > 0 ? m_ambient / m_diffuse : 0.5f, 0.01f, 0.99f);
}
}  // namespace spu
