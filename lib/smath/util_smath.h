//
//
//
#pragma once

#include <cfloat>
#include <cmath>
#include <functional>
#include <numbers>  // pi_v
#include <limits>   // epsilon, max
#include <vector>

// namespace spu::util {
namespace spu {

template<class T = float> constexpr T pi() noexcept { return std::numbers::pi_v<T>; }

template<class T = float> constexpr T epsilon() noexcept { return std::numeric_limits<T>::epsilon(); }

template<class T = float> constexpr T infinity() noexcept { return std::numeric_limits<T>::infinity(); }

template<class T = float> constexpr T huge() noexcept { return std::numeric_limits<T>::max(); }

template<class T> constexpr T radians(const T &x) noexcept
{
	static_assert(!static_cast<bool>(std::is_integral<T>::value), "radians with int");
	return x * T(pi<double>() / 180.0);  // for Vec
}

template<class T> constexpr T degrees(const T &x) noexcept
{
	static_assert(!static_cast<bool>(std::is_integral<T>::value), "degrees with int");
	return x * T(180.0 / pi<double>());  // for Vec
}

template<class T> T sign(const T &p) { return p > T(0) ? T(1) : p < T(0) ? T(-1) : T(0); }

template<class T> T repeat(const T &p, const T &l, const T &h)
{
	auto d = distance(h - l);
	return p > T(0) ? std::fmod(p, d) : std::fmod(p, d) + d;
}

template<class T, class RT> T lerp(const T &a0, const T &a1, const RT &r) { return a0 * (T(1) - r) + a1 * r; }

template<class T, class RT> T smoothstep(const T &a0, const T &a1, const RT &r)
{
	return lerp(a0, a1, r * r * r * (r * (r * RT(6) - RT(15)) + RT(10)));
}

template<class T, class RT> T coserp(const T &a0, const T &a1, const RT &r)
{
	auto cos_r = (-std::cos(radians(r * RT(180))) + RT(1)) * RT(0.5);
	return lerp(a0, a1, cos_r);
}

template<class T, class RT> T accelerp(const T &a0, const T &a1, const RT &r) { return lerp(a0, a1, r * r); }

template<class T, class RT> T decelerp(const T &a0, const T &a1, const RT &r)
{
	auto decel_r = RT(1) - r;
	return lerp(a0, a1, RT(1) - (decel_r * decel_r));
}

template<class T, class RT> T quadlerp(const T a[4], const RT &rA, const RT &rB)
{
	auto a0 = lerp(a[0], a[2], rB);
	auto a1 = lerp(a[1], a[3], rB);
	return lerp(a0, a1, rA);
}

// https://www.pcg-random.org/
inline uint32_t pcg_next(uint32_t &state)
{
    state = state * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

inline float pcg_frand01(uint32_t &state)
{
    return float(pcg_next(state)) * (1.0 / 4294967296.0);
}

}  // namespace spu
/* clang-format on */
