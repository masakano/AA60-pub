//
//
//
#pragma once
#include "vec.h"
namespace spu {
Vec3f color_by_name(const char *name);
Vec3f linear_to_srgb(const Vec3f &linear);
Vec3f srgb_to_linear(const Vec3f &srgb);
Vec3f rgb_to_hsv(const Vec3f &rgb);
Vec3f hsv_to_rgb(const Vec3f &hsv);

#define F(f) \
	inline Vec4f f(const Vec4f &s) { return Vec4f(f(Vec3f(s)), s.w); }
F(linear_to_srgb)
F(srgb_to_linear)
F(rgb_to_hsv)
F(hsv_to_rgb)
#undef F

#define F(f) \
	inline vec4f_t f(const vec4f_t &s) { return vec4f_t(f(Vec4f(s))); }
F(linear_to_srgb)
F(srgb_to_linear)
F(rgb_to_hsv)
F(hsv_to_rgb)
#undef F

}  // namespace spu
