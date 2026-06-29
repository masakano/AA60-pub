//
// Vec4i :
//
#pragma once

#include <ssys/simd.h>
#include "util_smath.h"

// use clang-format with 112 lines, no break at construction initializer
namespace spu {

/// 32bit integer 4-way SIMD
struct Vec4i : public vec4i_t {
	Vec4i() = default;

	constexpr Vec4i(const vec4i_t &v4) noexcept : vec4i_t(v4) {}
	constexpr Vec4i(const m128f &fv) noexcept : vec4i_t(fv) {}
	constexpr Vec4i(const m128i &iv) noexcept : vec4i_t(iv) {}
	constexpr Vec4i(int32_t x, int32_t y, int32_t z, int32_t w) noexcept : vec4i_t(x, y, z, w) {}
	constexpr explicit Vec4i(int32_t s) noexcept : vec4i_t(s) {}
	constexpr explicit Vec4i(int32_t *i) noexcept : vec4i_t(i[0], i[1], i[2], i[3]) {}

	std::string to_string(const char *fmt = "%d %d %d %d") const { return string_printf(fmt, x, y, z, w); }

	int32_t *data() noexcept { return i; }
	const int32_t *data() const noexcept { return i; }

	int32_t dot(const Vec4i &v) const noexcept { return x * v.x + y * v.y + z * v.z + w * v.w; }
	int32_t pack() const noexcept { return sse::pack(iv); }

	template<int32_t INDEX> int32_t extract() const noexcept { return sse::extract<INDEX>(iv); }

	Vec4i operator+=(const Vec4i &v) noexcept { return iv = sse::iadd(iv, v.iv); }
	Vec4i operator-=(const Vec4i &v) noexcept { return iv = sse::isub(iv, v.iv); }
	Vec4i operator*=(const Vec4i &v) noexcept { return iv = sse::imul(iv, v.iv); }
	Vec4i operator/=(const Vec4i &v) noexcept { return iv = sse::iset(x / v.x, y / v.y, z / v.z, w / v.w); }
	Vec4i operator&=(const Vec4i &v) noexcept { return fv = sse::iand(fv, v.fv); }
	Vec4i operator|=(const Vec4i &v) noexcept { return fv = sse::ior(fv, v.fv); }
	Vec4i operator^=(const Vec4i &v) noexcept { return fv = sse::ixor(fv, v.fv); }
	Vec4i operator+() const noexcept { return *this; }
	Vec4i operator-() const noexcept { return sse::isub(sse::izero(), iv); }
	Vec4i operator~() const noexcept { return sse::ixor(iv, sse::iset(-1)); }

	friend Vec4i operator+(const Vec4i &v0, const Vec4i &v1) noexcept { return sse::iadd(v0.iv, v1.iv); }
	friend Vec4i operator-(const Vec4i &v0, const Vec4i &v1) noexcept { return sse::isub(v0.iv, v1.iv); }
	friend Vec4i operator*(const Vec4i &v0, const Vec4i &v1) noexcept { return sse::imul(v0.iv, v1.iv); }
	friend Vec4i operator/(const Vec4i &v0, const Vec4i &v1) noexcept
	{
		return sse::iset(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z, v0.w / v1.w);
	}
	friend Vec4i operator&(const Vec4i &v0, const Vec4i &v1) noexcept { return sse::iand(v0.iv, v1.iv); }
	friend Vec4i operator|(const Vec4i &v0, const Vec4i &v1) noexcept { return sse::ior(v0.iv, v1.iv); }
	friend Vec4i operator^(const Vec4i &v0, const Vec4i &v1) noexcept { return sse::ixor(v0.iv, v1.iv); }

	friend Vec4i operator+(const Vec4i &v0, int32_t s1) noexcept { return sse::iadd(v0.iv, sse::iset(s1)); }
	friend Vec4i operator-(const Vec4i &v0, int32_t s1) noexcept { return sse::isub(v0.iv, sse::iset(s1)); }
	friend Vec4i operator*(const Vec4i &v0, int32_t s1) noexcept { return sse::imul(v0.iv, sse::iset(s1)); }
	friend Vec4i operator/(const Vec4i &v0, int32_t s1) noexcept
	{
		return sse::iset(v0.x / s1, v0.y / s1, v0.z / s1, v0.w / s1);
	}
	friend Vec4i operator&(const Vec4i &v0, int32_t s1) noexcept { return sse::iand(v0.iv, sse::iset(s1)); }
	friend Vec4i operator|(const Vec4i &v0, int32_t s1) noexcept { return sse::ior(v0.iv, sse::iset(s1)); }
	friend Vec4i operator^(const Vec4i &v0, int32_t s1) noexcept { return sse::ixor(v0.iv, sse::iset(s1)); }

	friend Vec4i operator+(int32_t s0, const Vec4i &v1) noexcept { return sse::iadd(sse::iset(s0), v1.iv); }
	friend Vec4i operator-(int32_t s0, const Vec4i &v1) noexcept { return sse::isub(sse::iset(s0), v1.iv); }
	friend Vec4i operator*(int32_t s0, const Vec4i &v1) noexcept { return sse::imul(sse::iset(s0), v1.iv); }
	friend Vec4i operator/(int32_t s0, const Vec4i &v1) noexcept
	{
		return sse::iset(s0 / v1.x, s0 / v1.y, s0 / v1.z, s0 / v1.w);
	}
	friend Vec4i operator&(int32_t s0, const Vec4i &v1) noexcept { return sse::iand(sse::iset(s0), v1.iv); }
	friend Vec4i operator|(int32_t s0, const Vec4i &v1) noexcept { return sse::ior(sse::iset(s0), v1.iv); }
	friend Vec4i operator^(int32_t s0, const Vec4i &v1) noexcept { return sse::ixor(sse::iset(s0), v1.iv); }

	friend Vec4i operator<<(const Vec4i &v0, const Vec4i &v1) noexcept { return sse::isll(v0.iv, v1.iv); }
	friend Vec4i operator>>(const Vec4i &v0, const Vec4i &v1) noexcept { return sse::isra(v0.iv, v1.iv); }
	friend Vec4i operator<<(const Vec4i &v0, int32_t s1) noexcept { return sse::isll(v0.iv, s1); }
	friend Vec4i operator>>(const Vec4i &v0, int32_t s1) noexcept { return sse::isra(v0.iv, s1); }

	friend Vec4i operator==(const Vec4i &v0, const Vec4i &v1) noexcept { return sse::ieq(v0.iv, v1.iv); }
	friend Vec4i operator!=(const Vec4i &v0, const Vec4i &v1) noexcept
	{
		return sse::ixor(sse::ieq(v0.iv, v1.iv), sse::iset(-1));
	}
	friend Vec4i operator>(const Vec4i &v0, const Vec4i &v1) noexcept { return sse::igt(v0.iv, v1.iv); }
	friend Vec4i operator<(const Vec4i &v0, const Vec4i &v1) noexcept { return sse::ilt(v0.iv, v1.iv); }
	friend Vec4i operator>=(const Vec4i &v0, const Vec4i &v1) noexcept
	{
		return sse::ixor(sse::ilt(v0.iv, v1.iv), sse::iset(-1));
	}
	friend Vec4i operator<=(const Vec4i &v0, const Vec4i &v1) noexcept
	{
		return sse::ixor(sse::igt(v0.iv, v1.iv), sse::iset(-1));
	}
	static constexpr int32_t size() { return 4; }
};
static_assert(std::is_trivially_copyable<Vec4i>::value, "not copyable");

template<class T> struct Vecf : public vec4f_t {
	Vecf() = default;

	operator Vec4i() const noexcept { return sse::ftoi(fv); }
	Vecf(const Vec4i &v) { this->fv = sse::itof(v.iv); }

	std::string to_string(const char *fmt = "%g %g %g %g") const { return string_printf(fmt, x, y, z, w); }

	float *data() noexcept { return f; }
	const float *data() const noexcept { return f; }

	template<int32_t INDEX> int32_t extract() const noexcept { return sse::extract<INDEX>(fv); }
	template<int32_t X, int32_t Y, int32_t Z, int32_t W> T shuffle() const noexcept
	{
		return sse::shuffle<X, Y, Z, W>(fv);
	}
	template<int32_t DST, int32_t SRC = 0, int32_t CLR = 0> T insert(const T &v) const noexcept
	{
		return sse::insert<DST, SRC, CLR>(fv, v.fv);
	}
	template<int32_t N> T scat() const noexcept { return sse::shuffle<N, N, N, N>(fv); }

	T floor() const noexcept { return sse::floor(fv); }
	T ceil() const noexcept { return sse::ceil(fv); }
	T abs() const noexcept { return sse::abs(fv); }

	T quantize(const T &quantize_step) const
	{
		const auto s0 = quantize_step;
		const auto s1 = 1.0f / quantize_step;
		return sse::mul(sse::round(sse::mul(fv, s1.fv)), s0.fv);
	}

	T operator+=(const T &v) noexcept { return fv = sse::add(fv, v.fv); }
	T operator-=(const T &v) noexcept { return fv = sse::sub(fv, v.fv); }
	T operator*=(const T &v) noexcept { return fv = sse::mul(fv, v.fv); }
	T operator/=(const T &v) noexcept { return fv = sse::div(fv, sse::denom(v.fv)); }

	T operator+=(float s) noexcept { return fv = sse::add(fv, vec4f_t(s).fv); }
	T operator-=(float s) noexcept { return fv = sse::sub(fv, vec4f_t(s).fv); }
	T operator*=(float s) noexcept { return fv = sse::mul(fv, vec4f_t(s).fv); }
	T operator/=(float s) noexcept { return fv = sse::div(fv, sse::denom(vec4f_t(s).fv)); }

	T operator+() const noexcept { return fv; }
	T operator-() const noexcept { return sse::neg(fv); }

	friend T operator+(const T &v0, const T &v1) noexcept { return sse::add(v0.fv, v1.fv); }
	friend T operator-(const T &v0, const T &v1) noexcept { return sse::sub(v0.fv, v1.fv); }
	friend T operator*(const T &v0, const T &v1) noexcept { return sse::mul(v0.fv, v1.fv); }
	friend T operator/(const T &v0, const T &v1) noexcept { return sse::div(v0.fv, sse::denom(v1.fv)); }
	friend T operator+(const T &v0, float s1) noexcept { return v0 + T(s1); }
	friend T operator-(const T &v0, float s1) noexcept { return v0 - T(s1); }
	friend T operator*(const T &v0, float s1) noexcept { return v0 * T(s1); }
	friend T operator/(const T &v0, float s1) { return (s1 == 1 || s1 == 0) ? v0 : v0 * T(1 / s1); }
	friend T operator+(float s0, const T &v1) noexcept { return T(s0) + v1; }
	friend T operator-(float s0, const T &v1) noexcept { return T(s0) - v1; }
	friend T operator*(float s0, const T &v1) noexcept { return T(s0) * v1; }
	friend T operator/(float s0, const T &v1) noexcept { return T(s0) / sse::denom(v1.fv); }
	friend Vec4i operator==(const T &v0, const T &v1) noexcept { return sse::eq(v0.fv, v1.fv); }
	friend Vec4i operator!=(const T &v0, const T &v1) noexcept { return ~(v0 == v1); }
	friend Vec4i operator>(const T &v0, const T &v1) noexcept { return sse::gt(v0.fv, v1.fv); }
	friend Vec4i operator<(const T &v0, const T &v1) noexcept { return sse::lt(v0.fv, v1.fv); }
	friend Vec4i operator>=(const T &v0, const T &v1) noexcept { return sse::ge(v0.fv, v1.fv); }
	friend Vec4i operator<=(const T &v0, const T &v1) noexcept { return sse::le(v0.fv, v1.fv); }

	friend bool greater(const T &v0, const T &v1) noexcept
	{
		if (v0.x > v1.x) return true;
		if (v0.x < v1.x) return false;
		if (v0.y > v1.y) return true;
		if (v0.y < v1.y) return false;
		if (v0.z > v1.z) return true;
		if (v0.z < v1.z) return false;
		if (v0.w > v1.w) return true;
		if (v0.w < v1.w) return false;
		return false;
	}

	friend bool equal_almost(const T &v0, const T &v1, const T &quantize_step)
	{
		auto av0 = v0.quantize(quantize_step);
		auto av1 = v1.quantize(quantize_step);
		return equal(av0, av1);
	}
	friend bool greater_almost(const T &v0, const T &v1, const T &quantize_step)
	{
		auto av0 = v0.quantize(quantize_step);
		auto av1 = v1.quantize(quantize_step);
		return greater(av0, av1);
	}
};

/// 16byte aligned 2D floating point vector
struct Vec2f : public Vecf<Vec2f> {
	Vec2f() = default;
	Vec2f(const m128f &fv) noexcept { this->fv = fv; }  // short cut
	Vec2f(const m128i &iv) noexcept { this->iv = iv; }  // short cut, no cast
	Vec2f(const vec4f_t &v) noexcept { fv = sse::select(sse::makemask<0x0ff>().fv, v.fv); }
	Vec2f(const vec3f_t &v) noexcept { fv = sse::set(v.x, v.y, 0, 0); }
	Vec2f(const vec2f_t &v) noexcept { fv = sse::set(v.x, v.y, 0, 0); }
	Vec2f(const vec4i_t &v) noexcept { fv = sse::set(v.x, v.y, 0, 0); }
	Vec2f(float x, float y) noexcept { fv = sse::set(x, y, 0, 0); }

	explicit Vec2f(float s) noexcept { fv = sse::set(s, s, 0, 0); }
	template<class T> explicit Vec2f(const T *v) noexcept { fv = sse::set(v[0], v[1], 0, 0); }
	operator vec2f_t() const noexcept { return vec2f_t(x, y); }

	float dot(const Vec2f &v) const noexcept { return x * v.x + y * v.y; }
	float length() const { return sqrtf(x * x + y * y); }
	Vec2f normalize() const { return sse::normalize2(fv); }

	friend bool equal(const Vec2f &v0, const Vec2f &v1) noexcept
	{
		return ((v0 == v1).pack() & 0x00ff) == 0x00ff;
	}

	static constexpr int32_t size() { return 2; }
};
static_assert(std::is_trivially_copyable<Vec2f>::value, "not copyable");

/// 16byte aligned 3D floating point vector
struct Vec3f : public Vecf<Vec3f> {
	Vec3f() = default;
	Vec3f(const m128f &fv) noexcept { this->fv = fv; }  // short cut
	Vec3f(const m128i &iv) noexcept { this->iv = iv; }  // short cut, no cast
	Vec3f(const vec4f_t &v) noexcept { fv = sse::select(sse::makemask<0x0fff>().fv, v.fv); }
	Vec3f(const vec3f_t &v) noexcept { fv = sse::set(v.x, v.y, v.z, 0); }
	Vec3f(const vec2f_t &v) noexcept { fv = sse::set(v.x, v.y, 0, 0); }
	Vec3f(const vec4i_t &v) noexcept { fv = sse::set(v.x, v.y, v.z, 0); }
	Vec3f(float x, float y, float z) noexcept { fv = sse::set(x, y, z, 0); }
	Vec3f(const Vec2f &v, float z) noexcept { fv = sse::set(v.x, v.y, z, 0); }

	explicit Vec3f(float s) noexcept { fv = sse::set(s, s, s, 0); }
	template<class T> explicit Vec3f(const T *v) noexcept { fv = sse::set(v[0], v[1], v[2], 0); }
	operator vec3f_t() const noexcept { return vec3f_t(x, y, z); }

	float dot(const Vec3f &v) const noexcept { return x * v.x + y * v.y + z * v.z; }
	float length() const { return sqrtf(x * x + y * y + z * z); }
	Vec3f normalize() const { return sse::normalize3(fv); }

	friend bool equal(const Vec3f &v0, const Vec3f &v1) noexcept
	{
		return ((v0 == v1).pack() & 0x0fff) == 0x0fff;
	}

	static constexpr int32_t size() { return 3; }
};
static_assert(std::is_trivially_copyable<Vec3f>::value, "not copyable");

/// 16byte aligned 2D floating point vector
struct Vec4f : public Vecf<Vec4f> {
	Vec4f() = default;
	Vec4f(const m128f &fv) noexcept { this->fv = fv; }  // short cutｓ
	Vec4f(const m128i &iv) noexcept { this->iv = iv; }  // short cut, no cast
	Vec4f(const vec4f_t &v) noexcept { fv = v.fv; }
	Vec4f(const vec3f_t &v) noexcept { fv = sse::set(v.x, v.y, v.z, 0); }
	Vec4f(const vec2f_t &v) noexcept { fv = sse::set(v.x, v.y, 0, 0); }
	Vec4f(const vec4i_t &v) noexcept { fv = sse::set(v.x, v.y, v.z, v.w); }
	Vec4f(float x, float y, float z, float w) noexcept { fv = sse::set(x, y, z, w); }
	Vec4f(const Vec2f &v, float z, float w) noexcept { fv = sse::set(v.x, v.y, z, w); }
	Vec4f(const Vec3f &v, float w) noexcept { fv = sse::set(v.x, v.y, v.z, w); }

	explicit Vec4f(float s) noexcept { fv = sse::set(s, s, s, s); }
	template<class T> explicit Vec4f(const T *v) noexcept { fv = sse::set(v[0], v[1], v[2], v[3]); }

	Vec4f pers3() const noexcept { return *this / w; };
	float dot(const Vec4f &v) const noexcept { return x * v.x + y * v.y + z * v.z + w * v.w; }
	float length() const { return sqrtf(x * x + y * y + z * z + w * w); }
	Vec2f normalize() const { return sse::normalize4(fv); }

	friend bool equal(const Vec4f &v0, const Vec4f &v1) noexcept
	{
		return ((v0 == v1).pack() & 0xffff) == 0xffff;
	}

	static constexpr int32_t size() { return 4; }
};
static_assert(std::is_trivially_copyable<Vec4f>::value, "not copyable");

template<class vec_t = Vec3f, class scalar_t = float> constexpr vec_t ex()
{
	return vec_t(sse::ex<scalar_t>());
}
template<class vec_t = Vec3f, class scalar_t = float> constexpr vec_t ey()
{
	return vec_t(sse::ey<scalar_t>());
}
template<class vec_t = Vec3f, class scalar_t = float> constexpr vec_t ez()
{
	return vec_t(sse::ez<scalar_t>());
}
template<class vec_t = Vec3f, class scalar_t = float> constexpr vec_t ew()
{
	return vec_t(sse::ew<scalar_t>());
}
template<class vec_t = Vec3f, class scalar_t = float> constexpr vec_t ezero()
{
	return vec_t(sse::ezero<scalar_t>());
}
template<class vec_t = Vec3f, class scalar_t = float> constexpr vec_t eone()
{
	return vec_t(sse::eone<scalar_t>());
}
template<class T> inline T cross(const T &v0, const T &v1) noexcept { return sse::cross(v0.fv, v1.fv); }
template<class T> inline auto dot(const T &v0, const T &v1) noexcept { return v0.dot(v1); }
template<class T> inline T normalize(const T &v) { return v.normalize(); }
// template<class T> inline T inverse(const T &v) { return v.inverse(); }

template<class T> inline auto length(const T &v) noexcept { return v.length(); }
template<class T> inline auto distance(const T &v0, const T &v1) noexcept { return length(v0 - v1); }

template<class T> inline T madd(const T &v0, const T &v1, const T &v2) noexcept
{
	return sse::madd(v0.fv, v1.fv, v2.fv);
}
template<class T> inline T select(const Vec4i &mv, const T &v0, const T &v1) noexcept
{
	return sse::select(mv.fv, v0.fv, v1.fv);
}
template<class T> inline T select(const uint32_t m, const T &v0, const T &v1) noexcept
{
	return select(sse::makemask(m), v0, v1);
}
template<int32_t MASK, class T> inline T select(const T &v0, const T &v1)
{
	return select(sse::makemask<MASK>(), v0, v1);
}

template<class T> T max(const T &v0, const T &v1) { return std::max(v0, v1); }
template<class T>
T max(const T &v0, const T &v1)
        requires std::is_base_of_v<vec4f_t, T>
{
	return sse::max(v0.fv, v1.fv);
}
template<class T>
T max(const T &v0, const T &v1)
        requires std::is_base_of_v<vec4i_t, T>
{
	return sse::imax(v0.iv, v1.iv);
}

template<class T> T min(const T &v0, const T &v1) { return std::min(v0, v1); }
template<class T>
T min(const T &v0, const T &v1)
        requires std::is_base_of_v<vec4f_t, T>
{
	return sse::min(v0.fv, v1.fv);
}
template<class T>
T min(const T &v0, const T &v1)
        requires std::is_base_of_v<vec4i_t, T>
{
	return sse::imin(v0.iv, v1.iv);
}

template<class T> T abs(const T &v) { return std::abs(v); }
template<class T>
T abs(const T &v)
        requires std::is_base_of_v<vec4f_t, T>
{
	return sse::abs(v.fv);
}
template<class T>
T abs(const T &v)
        requires std::is_base_of_v<vec4i_t, T>
{
	return max(v, -v);
}

template<class T> T clamp(const T &v, const T &low, const T &high) { return std::clamp(v, low, high); }
template<class T>
T clamp(const T &v, const T &low, const T &high)
        requires std::is_base_of_v<vec4f_t, T>
{
	return min(max(v, low), high);
}
template<class T>
T clamp(const T &v, const T &low, const T &high)
        requires std::is_base_of_v<vec4i_t, T>
{
	return min(max(v, low), high);
}

template<class T> T floor(const T &v) { return std::floor(v); }
template<class T>
T floor(const T &v)
        requires std::is_base_of_v<vec4f_t, T>
{
	return sse::floor(v.fv);
}

template<class T> T ceil(const T &v) { return std::ceil(v); }
template<class T>
T ceil(const T &v)
        requires std::is_base_of_v<vec4f_t, T>
{
	return sse::ceil(v.fv);
}

template<class T> T round(const T &v) { return std::round(v); }
template<class T>
T round(const T &v)
        requires std::is_base_of_v<vec4f_t, T>
{
	return sse::round(v.fv);
}

template<class T> T cross_candidate(const T &v)
{
	auto av = abs(v);
	if (av.x < av.y && av.x < av.z) return T(1, 0, 0);
	if (av.y < av.z && av.y < av.x) return T(0, 1, 0);
	return T(0, 0, 1);
}

}  // namespace spu
