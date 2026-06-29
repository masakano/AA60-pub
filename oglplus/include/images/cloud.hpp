//
//$<<Header>>$
//

#pragma once

#include <images/image.hpp>

namespace spu::oglplus::images {

class Cloud : public Image {
private:
	float m_sub_scale;
	float m_sub_variance;
	float m_min_radius;

	void adjust_sphere(Vec3f &center, float &radius) const;
	bool apply_sphere(const Vec3f &center, float radius);

	static float rand_u();
	static float rand_s();

	void make_spheres(Vec3f center, float radius);

public:
	Cloud(int32_t width, int32_t height, int32_t depth, const Vec3f &origin = Vec3f(0.0F, -0.3F, 0.0F),
	      float init_radius = 0.7F, float sub_scale = 0.333F, float sub_variance = 0.5F,
	      float min_radius = 0.04F);
};

class Cloud2D : public Image {
public:
	Cloud2D(const Cloud &cloud);
};

}  // namespace spu::oglplus::images
#include <images/cloud.ipp>
