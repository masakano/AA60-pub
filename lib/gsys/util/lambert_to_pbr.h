//
// GsLambertToPBR :
//
#pragma once
#include <smath/vec.h>

// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
namespace spu {
class GsLambertToPBR {
public:
	GsLambertToPBR(
	        const Vec3f &diffuse, const Vec3f &ambient, const Vec3f &specular, const float specular_power);
	float roughness() const;
	float metallic() const;
	float ao() const;

private:
	float m_diffuse;
	float m_ambient;
	float m_specular;
	float m_specularPower;
};

class GsPbrToLambert {
public:
	GsPbrToLambert(const Vec3f &albedo, float roughness, float metallic, float ao);
	Vec3f ambient() const;
	Vec3f specular() const;
	float specularPower() const;

private:
	Vec3f m_albedo;
	float m_roughness;
	float m_metallic;
	float m_ao;
};

}  // namespace spu
