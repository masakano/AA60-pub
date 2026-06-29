//
//
//
#pragma once
#include "ssys.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

namespace spu {
/// 4 x 32bit 4-way SIMD

using m128f = __m128;
using m128i = __m128i;

template<class T> struct simdv4_t {
	static_assert(std::is_trivially_copyable<T>::value, "not copyable");
	union {
		m128f fv;
		m128i iv;

		struct {
			T x, y, z, w;
		};
		struct {
			T r, g, b, a;
		};
		struct {
			T ox, oy, sx, sy;
		};
		struct {
			T x0, y0, x1, y1;
		};

		float f[4];
		int32_t i[4];
		uint32_t ui[4];

		int16_t s[8];
		uint16_t us[8];

		int8_t c[16];
		uint8_t uc[16];
	};

	constexpr simdv4_t() noexcept = default;
	constexpr simdv4_t(const m128f &fv) noexcept : fv(fv) {}
	constexpr simdv4_t(const m128i &iv) noexcept : iv(iv) {}
	constexpr simdv4_t(T x, T y, T z, T w) noexcept : x(x), y(y), z(z), w(w) {}

	constexpr simdv4_t(double x) noexcept { fv = _mm_set1_ps(x); }
	constexpr simdv4_t(float x) noexcept { fv = _mm_set1_ps(x); }
	constexpr simdv4_t(int32_t x) noexcept { iv = _mm_set1_epi32(x); }
	constexpr simdv4_t(uint32_t x) noexcept { iv = _mm_set1_epi32(x); }

	constexpr simdv4_t operator+() const noexcept { return *this; }
	constexpr simdv4_t operator-() const noexcept { return simdv4_t(-x, -y, -z, -w); }

	void report(const char *str) const;
};

template<> inline void simdv4_t<int32_t>::report(const char *str) const
{
	if (str && *str) aux_printf("%16s: ", str);
	aux_printf("%d %d %d %d\n", x, y, z, w);
}

template<> inline void simdv4_t<uint32_t>::report(const char *str) const
{
	if (str && *str) aux_printf("%16s: ", str);
	aux_printf("%08x %08x %08x %08x\n", x, y, z, w);
}
template<> inline void simdv4_t<float>::report(const char *str) const
{
	if (str && *str) aux_printf("%16s: ", str);
	aux_printf("%12.6f %12.6f %12.6f %12.6f\n", x, y, z, w);
}

template<class T> struct simdv3_t {
	static_assert(std::is_trivially_copyable<T>::value, "not copyable");
	union {
		struct {
			T x, y, z;
		};
		struct {
			T r, g, b;
		};
		float f[3];
		int32_t i[3];
	};
	constexpr simdv3_t() noexcept = default;
	constexpr simdv3_t(T x, T y, T z) noexcept : x(x), y(y), z(z) {}
	constexpr simdv3_t(T x) noexcept : x(x), y(x), z(x) {}

	constexpr simdv3_t operator+() const noexcept { return *this; }
	constexpr simdv3_t operator-() const noexcept { return simdv3_t(-x, -y, -z); }

	template<class T2> constexpr simdv3_t(const simdv4_t<T2> &f4) noexcept : x(f4.x), y(f4.y), z(f4.z) {}
	template<class T2> constexpr operator simdv4_t<T2>() const noexcept { return simdv4_t<T2>(x, y, z, 0); }

	void report(const char *str) const;
};

template<> inline void simdv3_t<int32_t>::report(const char *str) const
{
	if (str && *str) aux_printf("%16s: ", str);
	aux_printf("%d %d %d\n", x, y, z);
}
template<> inline void simdv3_t<uint32_t>::report(const char *str) const
{
	if (str && *str) aux_printf("%16s: ", str);
	aux_printf("%08x %08x %08x\n", x, y, z);
}
template<> inline void simdv3_t<float>::report(const char *str) const
{
	if (str && *str) aux_printf("%16s: ", str);
	aux_printf("%12.6f %12.6f %12.6f\n", x, y, z);
}

template<class T> struct simdv2_t {
	static_assert(std::is_trivially_copyable<T>::value, "not copyable");
	union {
		struct {
			T x, y;
		};
		float f[2];
		int32_t i[2];
	};
	constexpr simdv2_t() noexcept = default;
	constexpr simdv2_t(T x, T y) noexcept : x(x), y(y) {}
	constexpr simdv2_t(T x) noexcept : x(x), y(x) {}

	template<class T2> constexpr simdv2_t(const simdv4_t<T2> &f4) noexcept : x(f4.x), y(f4.y) {}
	template<class T2> constexpr operator simdv4_t<T2>() const noexcept { return simdv4_t<T2>(x, y, 0, 0); }

	void report(const char *str) const;
};

template<> inline void simdv2_t<int32_t>::report(const char *str) const
{
	if (str && *str) aux_printf("%16s: ", str);
	aux_printf("%d %d\n", x, y);
}
template<> inline void simdv2_t<uint32_t>::report(const char *str) const
{
	if (str && *str) aux_printf("%16s: ", str);
	aux_printf("%08x %08x\n", x, y);
}
template<> inline void simdv2_t<float>::report(const char *str) const
{
	if (str && *str) aux_printf("%16s: ", str);
	aux_printf("%12.6f %12.6f\n", x, y);
}

using vec4f_t = simdv4_t<float>;
using vec3f_t = simdv3_t<float>;
using vec2f_t = simdv2_t<float>;
using vec4u_t = simdv4_t<uint32_t>;
using vec3u_t = simdv3_t<uint32_t>;
using vec2u_t = simdv2_t<uint32_t>;
using vec4i_t = simdv4_t<int32_t>;
using vec3i_t = simdv3_t<int32_t>;
using vec2i_t = simdv2_t<int32_t>;

namespace sse {

template<class T = float> constexpr simdv4_t<T> ex() { return simdv4_t<T>(1, 0, 0, 0); }
template<class T = float> constexpr simdv4_t<T> ey() { return simdv4_t<T>(0, 1, 0, 0); }
template<class T = float> constexpr simdv4_t<T> ez() { return simdv4_t<T>(0, 0, 1, 0); }
template<class T = float> constexpr simdv4_t<T> ew() { return simdv4_t<T>(0, 0, 0, 1); }
template<class T = float> constexpr simdv4_t<T> ezero() { return simdv4_t<T>(0, 0, 0, 0); }
template<class T = float> constexpr simdv4_t<T> eone() { return simdv4_t<T>(1, 1, 1, 1); }

inline m128i iset(int x, int y, int z, int w) noexcept { return _mm_set_epi32(w, z, y, x); }
inline m128i iset(int s) noexcept { return _mm_set1_epi32(s); }
inline m128i izero() noexcept { return _mm_setzero_si128(); }

inline m128i iand(const m128i &v0, const m128i &v1) noexcept { return _mm_and_si128(v0, v1); }
inline m128i ior(const m128i &v0, const m128i &v1) noexcept { return _mm_or_si128(v0, v1); }
inline m128i ixor(const m128i &v0, const m128i &v1) noexcept { return _mm_xor_si128(v0, v1); }

inline m128i isll(const m128i &v0, const m128i &v1) noexcept { return _mm_sllv_epi32(v0, v1); }
inline m128i isrl(const m128i &v0, const m128i &v1) noexcept { return _mm_srlv_epi32(v0, v1); }
inline m128i isra(const m128i &v0, const m128i &v1) noexcept { return _mm_srav_epi32(v0, v1); }

inline m128i isll(const m128i &v0, int32_t shift) noexcept { return _mm_slli_epi32(v0, shift); }
inline m128i isrl(const m128i &v0, int32_t shift) noexcept { return _mm_srli_epi32(v0, shift); }
inline m128i isra(const m128i &v0, int32_t shift) noexcept { return _mm_srai_epi32(v0, shift); }

inline m128i iadd(const m128i &v0, const m128i &v1) noexcept { return _mm_add_epi32(v0, v1); }
inline m128i isub(const m128i &v0, const m128i &v1) noexcept { return _mm_sub_epi32(v0, v1); }
inline m128i imul(const m128i &v0, const m128i &v1) noexcept { return _mm_mullo_epi32(v0, v1); }

inline m128i imax(const m128i v0, const m128i v1) noexcept { return _mm_max_epi32(v0, v1); }
inline m128i imin(const m128i v0, const m128i v1) noexcept { return _mm_min_epi32(v0, v1); }

inline m128i ieq(const m128i v0, const m128i v1) noexcept { return _mm_cmpeq_epi32(v0, v1); }
inline m128i igt(const m128i v0, const m128i v1) noexcept { return _mm_cmpgt_epi32(v0, v1); }
inline m128i ilt(const m128i v0, const m128i v1) noexcept { return _mm_cmplt_epi32(v0, v1); }

inline m128f itof(const m128i &v) noexcept { return _mm_cvtepi32_ps(v); }
inline m128i ftoi(const m128f &v) noexcept { return _mm_cvttps_epi32(v); }

inline m128f set(float x, float y, float z, float w) noexcept { return _mm_set_ps(w, z, y, x); }
inline m128f set(float s) noexcept { return _mm_set1_ps(s); }

inline m128f iand(const m128f &v0, const m128f &v1) noexcept { return _mm_and_ps(v0, v1); }
inline m128f ior(const m128f &v0, const m128f &v1) noexcept { return _mm_or_ps(v0, v1); }
inline m128f ixor(const m128f &v0, const m128f &v1) noexcept { return _mm_xor_ps(v0, v1); }
inline m128f add(const m128f &v0, const m128f &v1) noexcept { return _mm_add_ps(v0, v1); }
inline m128f sub(const m128f &v0, const m128f &v1) noexcept { return _mm_sub_ps(v0, v1); }
inline m128f mul(const m128f &v0, const m128f &v1) noexcept { return _mm_mul_ps(v0, v1); }
inline m128f madd(const m128f &v0, const m128f &v1, const m128f &v2) noexcept
{
	return _mm_fmadd_ps(v0, v1, v2);
}
inline m128f div(const m128f &v0, const m128f &v1) noexcept { return _mm_div_ps(v0, v1); }

inline m128f max(const m128f v0, const m128f v1) noexcept { return _mm_max_ps(v0, v1); }
inline m128f min(const m128f v0, const m128f v1) noexcept { return _mm_min_ps(v0, v1); }

inline m128f floor(const m128f v) noexcept { return _mm_floor_ps(v); }
inline m128f ceil(const m128f v) noexcept { return _mm_ceil_ps(v); }
inline m128f round(const m128f v) noexcept { return _mm_round_ps(v, 0); }

inline m128f abs(const m128f &v) noexcept
{
	return _mm_and_ps(v, _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff)));
}

inline m128f neg(const m128f &v) noexcept
{
	return _mm_xor_ps(v, _mm_castsi128_ps(_mm_set1_epi32(0x80000000)));
}

inline m128f eq(const m128f v0, const m128f v1) noexcept { return _mm_cmpeq_ps(v0, v1); }
inline m128f gt(const m128f v0, const m128f v1) noexcept { return _mm_cmpgt_ps(v0, v1); }
inline m128f lt(const m128f v0, const m128f v1) noexcept { return _mm_cmplt_ps(v0, v1); }
inline m128f ge(const m128f v0, const m128f v1) noexcept { return _mm_cmpge_ps(v0, v1); }
inline m128f le(const m128f v0, const m128f v1) noexcept { return _mm_cmple_ps(v0, v1); }

inline int32_t pack(const m128i iv) noexcept { return _mm_movemask_epi8(iv); }

inline m128f movlh(const m128f v0, const m128f v1) noexcept { return _mm_movelh_ps(v0, v1); }
inline m128f movhl(const m128f v0, const m128f v1) noexcept { return _mm_movehl_ps(v0, v1); }

inline m128f unpacklo(const m128f v0, const m128f v1) noexcept { return _mm_unpacklo_ps(v0, v1); }
inline m128f unpackhi(const m128f v0, const m128f v1) noexcept { return _mm_unpackhi_ps(v0, v1); }

template<int32_t INDEX> inline int32_t extract(const m128i &a) noexcept { return _mm_extract_epi32(a, INDEX); }
template<int32_t INDEX> inline int32_t extract(const m128f &a) noexcept { return _mm_extract_ps(a, INDEX); }

template<int32_t X, int32_t Y, int32_t Z, int32_t W> inline m128f shuffle(const m128f v) noexcept
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(W, Z, Y, X));
}

inline m128f cross(const m128f a, const m128f b) noexcept
{
	auto a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
	auto a_zxy = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2));
	auto b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
	auto b_zxy = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2));
	auto ret0 = _mm_mul_ps(a_yzx, b_zxy);
	auto ret1 = _mm_mul_ps(a_zxy, b_yzx);
	return _mm_sub_ps(ret0, ret1);
}

inline m128f select(const m128f mask, const m128f a, const m128f b) noexcept
{
	return _mm_blendv_ps(b, a, mask);
}
inline m128f select(const m128f mask, const m128f a) noexcept { return _mm_and_ps(mask, a); }

inline m128f normalize4(m128f v)
{
	auto dot = _mm_dp_ps(v, v, 0xFF);
	auto len = _mm_sqrt_ps(dot);
	const auto eps = _mm_set1_ps(1e-30f);
	auto mask = _mm_cmpgt_ps(len, eps);
	auto invLen = _mm_div_ps(_mm_set1_ps(1.0f), len);
	auto n = _mm_mul_ps(v, invLen);
	return _mm_and_ps(n, mask);
}

inline m128f normalize3(m128f v)
{
	auto xyz = _mm_blend_ps(v, _mm_setzero_ps(), 0b1000);  // mask w
	return normalize4(xyz);
}

inline m128f normalize2(m128f v)
{
	auto xy = _mm_blend_ps(v, _mm_setzero_ps(), 0b1100);  // mask zw
	return normalize4(xy);
}

template<int32_t DST, int32_t SRC = 0, int32_t CLR = 0>
inline m128f insert(const m128f &a, const m128f &b) noexcept
{
	return _mm_insert_ps(a, b, (DST << 4) | (SRC << 6) | CLR);
}

template<int32_t DST> inline m128f insert(const m128f &a, float s) noexcept
{
	return insert<DST>(a, _mm_set1_ps(s));
}

inline m128f denom(const m128f v) noexcept
{
	const auto zero = _mm_setzero_ps();
	const auto one = _mm_set1_ps(1.0);
	const auto mask = _mm_cmpeq_ps(v, zero);
	return _mm_blendv_ps(v, one, mask);
}

inline constexpr simdv4_t<int32_t> makemask(const int32_t mask) noexcept
{
	const simdv4_t<int32_t> i = {
	        (mask & 0x000f) != 0 ? -1 : 0,
	        (mask & 0x00f0) != 0 ? -1 : 0,
	        (mask & 0x0f00) != 0 ? -1 : 0,
	        (mask & 0xf000) != 0 ? -1 : 0,
	};
	return i;
}

template<int32_t MASK> inline constexpr simdv4_t<int32_t> makemask() noexcept
{
	constexpr const simdv4_t<int32_t> i = {
	        (MASK & 0x000f) != 0 ? -1 : 0,
	        (MASK & 0x00f0) != 0 ? -1 : 0,
	        (MASK & 0x0f00) != 0 ? -1 : 0,
	        (MASK & 0xf000) != 0 ? -1 : 0,
	};
	return i;
}

}  // namespace sse
}  // namespace spu

#ifdef __clang__
#pragma clang diagnostic pop
#endif
