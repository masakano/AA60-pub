//
//$<<Header>>$
//

#pragma once
#include <smath/mat4f.h>
#include <smath/range.h>

namespace spu::oglplus::math {

inline constexpr double pi() { return spu::pi<double>(); }

inline constexpr double two_pi() { return spu::pi<double>() * 2.0; }

inline Mat4f unit() { return Mat4f(); }

inline Mat4f worldview(const Vec3f &eye, const Vec3f &dir, const Vec3f &up = ey())
{
	Mat4f m;
	m.set_orientation(&eye, &dir, &up);
	return m.inverse();
}

inline Mat4f lookat(const Vec3f &eye, const Vec3f &center, const Vec3f &up = ey())
{
	Mat4f m;
	auto dir = Vec3f(center - eye);
	m.set_orientation(&eye, &dir, &up);
	return m.inverse();
}

inline Mat4f perspective(const Rectf &viewport, float fovx, float near, float far)
{
	auto aspect = viewport.sx / viewport.sy;
	Mat4f m;
	m.set_projection(&fovx, &aspect, &near, &far, false);  // use fovx
	return m;
}
}  // namespace spu::oglplus::math
