//
// Quatf :
//
#pragma once
#include "mat4f.h"

// use clang-format with 112 lines, no break at construction initializer
namespace spu {

// Notes:
// - Quaternion multiplication order: q0 * q1 applies q1 then q0 (right-to-left).
// - Vec3f rotation uses q * v * q^{-1} (right-hand rule for axis angle).
// - Axis-angle ctor assumes axis is a direction vector; sign follows right-hand rule.
// - from_target(target, effect) returns a rotation that maps effect -> target (inputs expected normalized).
// - from_target_and_axis rotates around axis so effect becomes closest to target (inputs expected normalized).
// - from_eulerXYZ/ZYX apply intrinsic rotations in the listed order.
// - to_euler() returns angles compatible with from_eulerZYX (may wrap near +/-pi).
// - stabilize(up) adjusts roll so cross(up, dir) matches the rotated x-axis (up is world up).

struct Quatf : public Vec4f {
	Quatf() noexcept : Vec4f(0, 0, 0, 1) {}
	explicit Quatf(const vec4f_t &fv) noexcept : Vec4f(fv) {}

	Quatf(float x, float y, float z, float w) noexcept : Vec4f(x, y, z, w) {}
	Quatf(float radian, const Vec3f &axis);
	Quatf(const Mat4f &mat);

	Vec3f axis() const { return *this; }
	float real() const { return w; }

	operator Mat4f() const;

	Quatf conj() const { return {-x, -y, -z, w}; }
	Quatf normalize() const { return Quatf(sse::normalize4(fv)); }
	Quatf inverse() const { return conj().normalize(); }

	Quatf slerp(const Quatf &q1, float rate) const;
	Vec3f to_euler() const;

	Quatf stabilize(const Vec3f &up)
	{
		auto dir = *this * ez();
		auto dx0 = *this * ex();
		auto dx1 = spu::normalize<Vec3f>(cross(up, dir));
		auto qc = Quatf::from_target(dx1, dx0);
		return qc * *this;
	}

	Quatf operator*=(const Quatf &q1) { return *this = mul(*this, q1); }

	Quatf operator+() const noexcept { return *this; }
	Quatf operator-() const noexcept { return Quatf(sse::neg(fv)); }

	friend bool operator==(const Quatf &q0, const Quatf &q1) { return equal(q0, q1); }
	friend bool operator!=(const Quatf &q0, const Quatf &q1) { return !(q0 == q1); }

	friend Quatf operator+(const Quatf &q0, const Quatf &q1) { return Quatf(Vec4f(q0) + Vec4f(q1)); }
	friend Quatf operator-(const Quatf &q0, const Quatf &q1) { return Quatf(Vec4f(q0) - Vec4f(q1)); }
	friend Quatf operator*(const Quatf &q0, const Quatf &q1) { return mul(q0, q1); }

	friend Vec3f operator*(const Quatf &q0, const Vec3f &v1) { return q0 * Quatf(v1) * q0.inverse(); }
	friend Mat4f operator*(const Quatf &q, const Mat4f &m) { return Mat4f(q) * m; }
	friend Mat4f operator*(const Mat4f &m, const Quatf &q) { return m * Mat4f(q); }
	friend Quatf operator*(float s, const Quatf &q) { return Quatf(Vec4f(q) * s); }
	friend Quatf operator*(const Quatf &q, float s) { return Quatf(Vec4f(q) * s); }
	friend Quatf operator/(const Quatf &q, float s) { return Quatf(Vec4f(q) / s); }

	static Quatf mul(const Quatf &q0, const Quatf &q1)
	{
		const auto a0 = q0.axis();
		const auto a1 = q1.axis();
		const auto r0 = q0.real();
		const auto r1 = q1.real();
		return Quatf(Vec4f(a0 * r1 + a1 * r0 + cross(a0, a1), r0 * r1 - a0.dot(a1)));
	}

	static Quatf from_target(const Vec3f &target, const Vec3f &effect, float max_radian = radians(360.0));

	static Quatf from_target_and_axis(
	        const Vec3f &target, const Vec3f &effect, const Vec3f &axis, float max_radian = radians(360.0));

	static Quatf from_eulerXYZ(const Vec3f &euler);
	static Quatf from_eulerZYX(const Vec3f &euler);
};

template<> inline Quatf lerp(const Quatf &a0, const Quatf &a1, const float &r) { return a0.slerp(a1, r); }

struct Transformf {
	Vec3f t;
	Quatf q;

	Transformf() : t(0) {}
	Transformf(float t) : t(Vec3f(t)) {}  // for bspline
	Transformf(const Vec3f &t) : t(t) {}
	Transformf(const Vec3f &t, const Quatf &q) : t(t), q(q) {}
	Transformf(const Vec3f &t0, const Quatf &q, const Vec3f &t1)
	{
		*this = Transformf(t1) * Transformf(t0, q);
	}

	Transformf(const Mat4f &m) : Transformf(m.c[3], Quatf(m)) {}

	operator Mat4f() const
	{
		return Mat4f(q).trans(t);  // rot then trans
	}
	Transformf inverse() const
	{
		auto q1 = q.inverse();
		auto t1 = -(q1 * t);
		return {t1, q1};
	}

	Transformf slerp(const Transformf &tr1, float rate) const
	{
		return {lerp(t, tr1.t, rate), q.slerp(tr1.q, rate)};
	}

	Transformf operator*=(const Transformf &tr1)
	{
		// rot then trans
		t += q * tr1.t;
		q *= tr1.q;
		return *this;
	}

	friend bool operator==(const Transformf &tr0, const Transformf &tr1) noexcept
	{
		return equal(tr0.q, tr1.q) && equal(tr0.t, tr1.t);
	}

	friend Vec3f operator*(const Transformf &t, const Vec3f &v) { return t.q * v + t.t; }
	friend Mat4f operator*(const Transformf &t, const Mat4f &m) { return Mat4f(t) * m; }
	friend Mat4f operator*(const Mat4f &m, const Transformf &t) { return m * Mat4f(t); }

	void report(const char *str) const;

	friend Transformf operator*(const Transformf &tr0, const Transformf &tr1)
	{
		auto t = tr0;
		return t *= tr1;
	}

	template<class T, uint32_t M> friend Range<T, M> operator*(const Transformf &tr, const Range<T, M> &r0)
	{
		return Range<T, M>(tr * r0.points());
	}

	template<class T> friend std::vector<T> operator*(const Transformf &tr, const std::vector<T> &v0)
	{
		std::vector<T> v1;
		v1.reserve(v0.size());
		for (const auto &v: v0) {
			v1.push_back(T(tr * Vec3f(v)));
		}
		return v1;
	}

	static Transformf from_orientation(const Vec3f &eye, const Vec3f &dir, const Vec3f &up)
	{
		auto q0 = Quatf::from_target(dir, -ez());
		auto q1 = Quatf::from_target(up, ey());
		return Transformf(eye, (q0 * q1).stabilize(up));
	}
};

template<> inline Transformf lerp(const Transformf &a0, const Transformf &a1, const float &r)
{
	return a0.slerp(a1, r);
}
}  // namespace spu
