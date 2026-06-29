//
//$<<Header>>$
//

#pragma once
#include <cmath>
#include <cstdint>
#include <cfloat>
#include <cuda.h>
#include <cuda_runtime.h>

#ifdef __NVCC__
#ifndef _D_
#define _D_ __inline__ __device__
#endif

_D_ float uint_as_float(uint i) { return __uint_as_float(i); }

#else
#ifndef _D_
#define _D_ inline
#endif

#ifndef cu_ck
#define cu_ck(call)                                     \
	{                                               \
		auto e = (call);                        \
		if (e) {                                \
			printf("cuda error %08x\n", e); \
			assert(0);                      \
		}                                       \
	}
#endif

_D_ float rsqrtf(float x) { return 1.0f / sqrtf(x); }

_D_ float uint_as_float(uint i)
{
	union {
		float f;
		uint i;
	} u;
	u.i = i;
	return u.f;
}

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

struct mat3f {
	float3 c[3];
	mat3f() = default;
	_D_ mat3f(const float3 &c0, const float3 &c1, const float3 &c2) : c{c0, c1, c2} {}
};

struct mat4f {
	float4 c[4];
	mat4f() = default;
	_D_ mat4f(const float4 &c0, const float4 &c1, const float4 &c2, const float4 &c3) : c{c0, c1, c2, c3} {}
};

struct transformf {
	float3 t;
	ushort model_index;
	ushort matset_index;
	float4 q;
};

//
// math
//

_D_ float4 make_float4(const float4 &v, float w) { return make_float4(v.x, v.y, v.z, w); }

_D_ float4 make_float4(const float3 &v, float w) { return make_float4(v.x, v.y, v.z, w); }

_D_ float4 make_float4(const float2 &v, float z, float w) { return make_float4(v.x, v.y, z, w); }

_D_ float4 make_float4(const float s) { return make_float4(s, s, s, s); }

_D_ float3 make_float3(const float4 &v) { return make_float3(v.x, v.y, v.z); }

_D_ float3 make_float3(const float2 &v, float z) { return make_float3(v.x, v.y, z); }

_D_ float3 make_float3(const float s) { return make_float3(s, s, s); }

_D_ float2 make_float2(const float4 &v) { return make_float2(v.x, v.y); }

_D_ float2 make_float2(const float3 &v) { return make_float2(v.x, v.y); }

_D_ float2 make_float2(const float s) { return make_float2(s, s); }

_D_ float4 operator-(const float4 &a) { return make_float4(-a.x, -a.y, -a.z, -a.w); }

_D_ float4 operator+=(float4 &a, const float4 &b)
{
	return a = make_float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

_D_ float4 operator-=(float4 &a, const float4 &b)
{
	return a = make_float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

_D_ float4 operator*=(float4 &a, const float4 &b)
{
	return a = make_float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

_D_ float4 operator/=(float4 &a, const float4 &b)
{
	return a = make_float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

_D_ float4 operator+=(float4 &a, const float &b) { return a = make_float4(a.x + b, a.y + b, a.z + b, a.w + b); }

_D_ float4 operator-=(float4 &a, const float &b) { return a = make_float4(a.x - b, a.y - b, a.z - b, a.w - b); }

_D_ float4 operator*=(float4 &a, const float &b) { return a = make_float4(a.x * b, a.y * b, a.z * b, a.w * b); }

_D_ float4 operator/=(float4 &a, const float &b) { return a = make_float4(a.x / b, a.y / b, a.z / b, a.w / b); }

_D_ float4 operator+(const float4 &a, const float4 &b)
{
	return make_float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

_D_ float4 operator-(const float4 &a, const float4 &b)
{
	return make_float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

_D_ float4 operator*(const float4 &a, const float4 &b)
{
	return make_float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

_D_ float4 operator/(const float4 &a, const float4 &b)
{
	return make_float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

_D_ float4 operator+(const float4 &a, const float b) { return make_float4(a.x + b, a.y + b, a.z + b, a.w + b); }

_D_ float4 operator-(const float4 &a, const float b) { return make_float4(a.x - b, a.y - b, a.z - b, a.w - b); }

_D_ float4 operator*(const float4 &a, const float b) { return make_float4(a.x * b, a.y * b, a.z * b, a.w * b); }

_D_ float4 operator/(const float4 &a, const float b) { return make_float4(a.x / b, a.y / b, a.z / b, a.w / b); }

_D_ float4 operator+(const float a, const float4 &b) { return make_float4(a + b.x, a + b.y, a + b.z, a + b.w); }

_D_ float4 operator-(const float a, const float4 &b) { return make_float4(a - b.x, a - b.y, a - b.z, a - b.w); }

_D_ float4 operator*(const float a, const float4 &b) { return make_float4(a * b.x, a * b.y, a * b.z, a * b.w); }

_D_ float4 operator/(const float a, const float4 &b) { return make_float4(a / b.x, a / b.y, a / b.z, a / b.w); }

_D_ float3 operator-(const float3 &a) { return make_float3(-a.x, -a.y, -a.z); }

_D_ float3 operator+=(float3 &a, const float3 &b) { return a = make_float3(a.x + b.x, a.y + b.y, a.z + b.z); }

_D_ float3 operator-=(float3 &a, const float3 &b) { return a = make_float3(a.x - b.x, a.y - b.y, a.z - b.z); }

_D_ float3 operator*=(float3 &a, const float3 &b) { return a = make_float3(a.x * b.x, a.y * b.y, a.z * b.z); }

_D_ float3 operator/=(float3 &a, const float3 &b) { return a = make_float3(a.x / b.x, a.y / b.y, a.z / b.z); }

_D_ float3 operator+=(float3 &a, const float &b) { return a = make_float3(a.x + b, a.y + b, a.z + b); }

_D_ float3 operator-=(float3 &a, const float &b) { return a = make_float3(a.x - b, a.y - b, a.z - b); }

_D_ float3 operator*=(float3 &a, const float &b) { return a = make_float3(a.x * b, a.y * b, a.z * b); }

_D_ float3 operator/=(float3 &a, const float &b) { return a = make_float3(a.x / b, a.y / b, a.z / b); }

_D_ float3 operator+(const float3 &a, const float3 &b) { return make_float3(a.x + b.x, a.y + b.y, a.z + b.z); }

_D_ float3 operator-(const float3 &a, const float3 &b) { return make_float3(a.x - b.x, a.y - b.y, a.z - b.z); }

_D_ float3 operator*(const float3 &a, const float3 &b) { return make_float3(a.x * b.x, a.y * b.y, a.z * b.z); }

_D_ float3 operator/(const float3 &a, const float3 &b) { return make_float3(a.x / b.x, a.y / b.y, a.z / b.z); }

_D_ float3 operator+(const float3 &a, const float b) { return make_float3(a.x + b, a.y + b, a.z + b); }

_D_ float3 operator-(const float3 &a, const float b) { return make_float3(a.x - b, a.y - b, a.z - b); }

_D_ float3 operator*(const float3 &a, const float b) { return make_float3(a.x * b, a.y * b, a.z * b); }

_D_ float3 operator/(const float3 &a, const float b) { return make_float3(a.x / b, a.y / b, a.z / b); }

_D_ float3 operator+(const float a, const float3 &b) { return make_float3(a + b.x, a + b.y, a + b.z); }

_D_ float3 operator-(const float a, const float3 &b) { return make_float3(a - b.x, a - b.y, a - b.z); }

_D_ float3 operator*(const float a, const float3 &b) { return make_float3(a * b.x, a * b.y, a * b.z); }

_D_ float3 operator/(const float a, const float3 &b) { return make_float3(a / b.x, a / b.y, a / b.z); }

_D_ float2 operator-(const float2 &a) { return make_float2(-a.x, -a.y); }

_D_ float2 operator+=(float2 &a, const float2 &b) { return a = make_float2(a.x + b.x, a.y + b.y); }

_D_ float2 operator-=(float2 &a, const float2 &b) { return a = make_float2(a.x - b.x, a.y - b.y); }

_D_ float2 operator*=(float2 &a, const float2 &b) { return a = make_float2(a.x * b.x, a.y * b.y); }

_D_ float2 operator/=(float2 &a, const float2 &b) { return a = make_float2(a.x / b.x, a.y / b.y); }

_D_ float2 operator+=(float2 &a, const float &b) { return a = make_float2(a.x + b, a.y + b); }

_D_ float2 operator-=(float2 &a, const float &b) { return a = make_float2(a.x - b, a.y - b); }

_D_ float2 operator*=(float2 &a, const float &b) { return a = make_float2(a.x * b, a.y * b); }

_D_ float2 operator/=(float2 &a, const float &b) { return a = make_float2(a.x / b, a.y / b); }

_D_ float2 operator+(const float2 &a, const float2 &b) { return make_float2(a.x + b.x, a.y + b.y); }

_D_ float2 operator-(const float2 &a, const float2 &b) { return make_float2(a.x - b.x, a.y - b.y); }

_D_ float2 operator*(const float2 &a, const float2 &b) { return make_float2(a.x * b.x, a.y * b.y); }

_D_ float2 operator/(const float2 &a, const float2 &b) { return make_float2(a.x / b.x, a.y / b.y); }

_D_ float2 operator+(const float2 &a, const float b) { return make_float2(a.x + b, a.y + b); }

_D_ float2 operator-(const float2 &a, const float b) { return make_float2(a.x - b, a.y - b); }

_D_ float2 operator*(const float2 &a, const float b) { return make_float2(a.x * b, a.y * b); }

_D_ float2 operator/(const float2 &a, const float b) { return make_float2(a.x / b, a.y / b); }

_D_ float2 operator+(const float a, const float2 &b) { return make_float2(a + b.x, a + b.y); }

_D_ float2 operator-(const float a, const float2 &b) { return make_float2(a - b.x, a - b.y); }

_D_ float2 operator*(const float a, const float2 &b) { return make_float2(a * b.x, a * b.y); }

_D_ float2 operator/(const float a, const float2 &b) { return make_float2(a / b.x, a / b.y); }

_D_ float dot(const float4 &a, const float4 &b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

_D_ float dot(const float3 &a, const float3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

_D_ float dot(const float2 &a, const float2 &b) { return a.x * b.x + a.y * b.y; }

template<class T> _D_ T normalize(const T &v) { return v * rsqrtf(dot(v, v)); }

template<class T> _D_ float square_length(const T &v) { return dot(v, v); }

template<class T> _D_ float length(const T &v) { return sqrtf(dot(v, v)); }

template<class T> _D_ float square_distance(const T &v0, const T &v1) { return square_length(v0 - v1); }

template<class T> _D_ float distance(const T &v0, const T &v1) { return length(v0 - v1); }

_D_ mat3f make_mat3f(const float3 &s)
{
	mat3f m;
	m.c[0] = make_float3(s.x, 0, 0);
	m.c[1] = make_float3(0, s.y, 0);
	m.c[2] = make_float3(0, 0, s.z);
	return m;
}

_D_ mat4f make_mat4f(const float3 &s, const float3 &t = make_float3(0))
{
	mat4f m;
	m.c[0] = make_float4(s.x, 0, 0, 0);
	m.c[1] = make_float4(0, s.y, 0, 0);
	m.c[2] = make_float4(0, 0, s.z, 0);
	m.c[3] = make_float4(t, 1);
	return m;
}

_D_ mat4f make_mat4f(const float s, const float3 &t) { return make_mat4f(make_float3(s), t); }

_D_ mat4f make_mat4f(const float s) { return make_mat4f(s, make_float3(0)); }

_D_ mat3f transpose(const mat3f &m0)
{
	mat3f m1;
	m1.c[0] = make_float3(m0.c[0].x, m0.c[1].x, m0.c[2].x);
	m1.c[1] = make_float3(m0.c[0].y, m0.c[1].y, m0.c[2].y);
	m1.c[2] = make_float3(m0.c[0].z, m0.c[1].z, m0.c[2].z);
	return m1;
}
_D_ mat4f transpose(const mat4f &m0)
{
	mat4f m1;
	m1.c[0] = make_float4(m0.c[0].x, m0.c[1].x, m0.c[2].x, m0.c[3].x);
	m1.c[1] = make_float4(m0.c[0].y, m0.c[1].y, m0.c[2].y, m0.c[3].y);
	m1.c[2] = make_float4(m0.c[0].z, m0.c[1].z, m0.c[2].z, m0.c[3].z);
	m1.c[3] = make_float4(m0.c[0].w, m0.c[1].w, m0.c[2].w, m0.c[3].w);
	return m1;
}

_D_ float4 operator*(const mat4f &m, const float4 &v)
{
	return v.x * m.c[0] + v.y * m.c[1] + v.z * m.c[2] + v.w * m.c[3];
}

_D_ float3 operator*(const mat4f &m, const float3 &v)
{
	return v.x * make_float3(m.c[0]) + v.y * make_float3(m.c[1]) + v.z * make_float3(m.c[2]);
}

_D_ float3 operator*(const mat3f &m, const float3 &v) { return v.x * m.c[0] + v.y * m.c[1] + v.z * m.c[2]; }

_D_ mat3f operator*(const mat3f &m0, const mat3f &m1)
{
	mat3f m;
	m.c[0] = m0 * m1.c[0];
	m.c[1] = m0 * m1.c[1];
	m.c[2] = m0 * m1.c[2];
	return m;
}

_D_ mat4f operator*(const mat4f &m0, const mat4f &m1)
{
	mat4f m;
	m.c[0] = m0 * m1.c[0];
	m.c[1] = m0 * m1.c[1];
	m.c[2] = m0 * m1.c[2];
	m.c[3] = m0 * m1.c[3];
	return m;
}

_D_ mat4f make_mat4f(const mat3f &m3)
{
	mat4f m4;
	m4.c[0] = make_float4(m3.c[0], 0);
	m4.c[1] = make_float4(m3.c[1], 0);
	m4.c[2] = make_float4(m3.c[2], 0);
	m4.c[3] = make_float4(0, 0, 0, 1);
	return m4;
}

_D_ mat3f make_mat3f(const mat4f &m4)
{
	mat3f m3;
	m3.c[0] = make_float3(m4.c[0]);
	m3.c[1] = make_float3(m4.c[1]);
	m3.c[2] = make_float3(m4.c[2]);
	return m3;
}

template<class T> _D_ T abs(const T &a) { return a > 0 ? a : -a; }

template<> _D_ float3 abs(const float3 &a) { return make_float3(abs(a.x), abs(a.y), abs(a.z)); }

template<class T> _D_ T sign(const T &a) { return a > 0 ? T(1) : T(-1); }

template<> _D_ float3 sign(const float3 &a) { return make_float3(sign(a.x), sign(a.y), sign(a.z)); }

template<class T> _D_ T min(const T &a, const T &b) { return a < b ? a : b; }

template<> _D_ float3 min(const float3 &a, const float3 &b)
{
	return make_float3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

template<class T> _D_ T max(const T &a, const T &b) { return a > b ? a : b; }

template<> _D_ float3 max(const float3 &a, const float3 &b)
{
	return make_float3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

_D_ float3 cross(const float3 &a, const float3 &b)
{
	return make_float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

_D_ float dot3(const float3 &a, const float3 &b, const float3 &c)
{
	return (a.x * b.y * c.z - a.x * b.z * c.y + a.y * b.z * c.x - a.y * b.x * c.z + a.z * b.x * c.y
	        - a.z * b.y * c.x);
}

template<class T> _D_ T clamp(const T &v, const T &l, const T &h) { return v < l ? l : v > h ? h : v; }

template<> _D_ float4 clamp(const float4 &v, const float4 &l, const float4 &h)
{
	return make_float4(
	        clamp(v.x, l.x, h.x), clamp(v.y, l.y, h.y), clamp(v.z, l.z, h.z), clamp(v.w, l.w, h.w));
}

_D_ float4 clamp(const float4 &v, float l, float h)
{
	return make_float4(clamp(v.x, l, h), clamp(v.y, l, h), clamp(v.z, l, h), clamp(v.w, l, h));
}

_D_ void make_ntb(const float3 &N, float3 &T, float3 &B)
{
	T = fabs(N.x) > fabs(N.y) ? make_float3(0, 1, 0) : make_float3(1, 0, 0);
	B = normalize(cross(N, T));
	T = cross(B, N);
}

_D_ float3 reflect(const float3 &V, const float3 &N)
{
	return normalize(V - (2.0 * dot(V, N)) * N);  // need normalize?
}

_D_ float4 quat_from_radian(float radian, const float3 &axis)
{
	float h = radian / 2;
	float s = sinf(h), c = cosf(h);
	return make_float4(s * normalize(axis), c);
}

_D_ float4 quat_conj(const float4 &q) { return make_float4(-q.x, -q.y, -q.z, q.w); }

_D_ float4 quat_mul(const float4 &q0, const float4 &q1)
{
	const float3 p0 = make_float3(q0);
	const float3 p1 = make_float3(q1);

	const float3 p2 = p0 * q1.w + p1 * q0.w + cross(p0, p1);
	const float r2 = q0.w * q1.w - dot(p0, p1);

	return make_float4(p2, r2);
}

_D_ float3 quat_rot(const float4 &q0, const float3 &p1)
{
	const float4 q1 = make_float4(p1, 0);
	return make_float3(quat_mul(q0, quat_mul(q1, quat_conj(q0))));
}

_D_ mat3f quat_to_mat3f(const float4 &q)
{
	float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
	float xy = q.x * q.y, yz = q.y * q.z, zx = q.z * q.x;
	float xw = q.x * q.w, yw = q.y * q.w, zw = q.z * q.w;

	mat3f m;
	m.c[0] = make_float3(1 - 2 * (yy + zz), 2 * (xy + zw), 2 * (zx - yw));
	m.c[1] = make_float3(2 * (xy - zw), 1 - 2 * (xx + zz), 2 * (yz + xw));
	m.c[2] = make_float3(2 * (zx + yw), 2 * (yz - xw), 1 - 2 * (xx + yy));
	return m;
}

_D_ mat4f quat_to_mat4f(const float4 &q) { return make_mat4f(quat_to_mat3f(q)); }

_D_ float srgb_to_linear(float value)
{
	return (value <= 0.04045f) ? value * (1.0f / 12.92f) : powf((value + 0.055f) * (1.0f / 1.055f), 2.4f);
}

_D_ float linear_to_srgb(float value)
{
	return (value <= 0.0031308f) ? 12.92f * value : 1.055f * powf(value, 1 / 2.4f) - 0.055f;
}

_D_ float exp_pack(float value) { return 1.0f - expf(-abs(value)); }

_D_ float reinhard_pack(float value) { return value / (1.0f + abs(value)); }

#define APPLY_FLOAT34(F)                                                              \
	_D_ float3 F(const float3 &v) { return make_float3(F(v.x), F(v.y), F(v.z)); } \
	_D_ float4 F(const float4 &v) { return make_float4(F(v.x), F(v.y), F(v.z), v.w); }

APPLY_FLOAT34(linear_to_srgb)
APPLY_FLOAT34(srgb_to_linear)
APPLY_FLOAT34(exp_pack)
APPLY_FLOAT34(reinhard_pack)

#undef APPLY_FLOAT34

template<class T> _D_ T safe_divide(const T &a, const T &b) { return b ? a / b : T(0); }

template<> _D_ float3 safe_divide(const float3 &a, const float3 &b)
{
	return make_float3(safe_divide(a.x, b.x), safe_divide(a.y, b.y), safe_divide(a.z, b.z));
}

_D_ void divide_by_w(float4 &src)
{
	if (src.w > 0) {
		src.x /= src.w;
		src.y /= src.w;
		src.z /= src.w;
	}
}

_D_ uint hash_next(uint x)
{
	x += (x << 10u);
	x ^= (x >> 6u);
	x += (x << 3u);
	x ^= (x >> 11u);
	x += (x << 15u);
	return x;
}

_D_ float rand_next(uint &rand_index)
{
	const uint mantissa_mask = 0x007FFFFFu;
	const uint one = 0x3F800000u;

	uint h = rand_index = hash_next(rand_index);
	h &= mantissa_mask;
	h |= one;

	const float r2 = uint_as_float(h);
	return r2 - 1.0;
}
_D_ bool rand_less(float threshold, uint &rand_index) { return rand_next(rand_index) <= threshold; }


// 64bit PCG
class PCG32RNG {
	uint64_t state;

public:
#ifndef __NVCC__
	_D_ PCG32RNG() = default;
#endif
	_D_ void setState(uint64_t _state) { state = _state; }

	_D_ uint32_t operator()()
	{
		uint64_t oldstate = state;
		state = oldstate * 6364136223846793005ULL + 1;
		uint32_t xorshifted = static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);
		uint32_t rot = oldstate >> 59u;
		return (xorshifted >> rot) | (xorshifted << ((-static_cast<int32_t>(rot)) & 31));
	}

	_D_ float getFloat0cTo1o()
	{
		uint32_t fractionBits = ((*this)() >> 9) | 0x3f800000;
		union {
			uint32_t i;
			float f;
		} u;
		u.i = fractionBits;
		return u.f - 1.0f;
	}
};

