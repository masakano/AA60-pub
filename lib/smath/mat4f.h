//
// Mat4f :
//
#pragma once
#include "range.h"

namespace spu {

struct Plane3f;
class Convex3f;
struct Transformf;
struct Segment3f;

struct Mat4f {
	Vec4f c[4];

	Mat4f() noexcept { set(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1); }

	Mat4f(const Vec4f &c0, const Vec4f &c1, const Vec4f &c2, const Vec4f &c3 = Vec4f(0, 0, 0, 1)) noexcept
	        : c{c0, c1, c2, c3}
	{
	}

	Mat4f(float x0, float x1, float x2, float x3, float y0, float y1, float y2, float y3, float z0,
	      float z1, float z2, float z3, float w0, float w1, float w2, float w3) noexcept
	{
		set(x0, x1, x2, x3, y0, y1, y2, y3, z0, z1, z2, z3, w0, w1, w2, w3);
	}

	explicit Mat4f(const Range3f &range, uint32_t mask = 0xfff);

	friend Vec4f operator*(const Mat4f &m, const Vec4f &v)
	{
		// clang-format off
		return (v.shuffle<0, 0, 0, 0>() * m.c[0] +
			v.shuffle<1, 1, 1, 1>() * m.c[1] +
			v.shuffle<2, 2, 2, 2>() * m.c[2] +
			v.shuffle<3, 3, 3, 3>() * m.c[3]);
		// clang-format on
	}

	friend Mat4f operator+(const Mat4f &m0, const Mat4f &m1) noexcept
	{
		return {
		        m0.c[0] + m1.c[0],
		        m0.c[1] + m1.c[1],
		        m0.c[2] + m1.c[2],
		        m0.c[3] + m1.c[3],
		};
	}

	friend Mat4f operator-(const Mat4f &m0, const Mat4f &m1) noexcept
	{
		return {
		        m0.c[0] - m1.c[0],
		        m0.c[1] - m1.c[1],
		        m0.c[2] - m1.c[2],
		        m0.c[3] - m1.c[3],
		};
	}

	friend Mat4f operator*(const Mat4f &m0, const Mat4f &m1) noexcept
	{
		return {m0 * m1.c[0], m0 * m1.c[1], m0 * m1.c[2], m0 * m1.c[3]};
	}

	friend Mat4f operator*(const Mat4f &m, float s) noexcept
	{
		return {m.c[0] * s, m.c[1] * s, m.c[2] * s, m.c[3] * s};
	}
	friend Mat4f operator*(float s, const Mat4f &m) noexcept
	{
		return {m.c[0] * s, m.c[1] * s, m.c[2] * s, m.c[3] * s};
	}

	float *data() noexcept { return c[0].f; }
	const float *data() const noexcept { return c[0].f; }

	bool is_ortho() const noexcept
	{
		auto r4 = Vec4f(c[0].w, c[1].w, c[2].w, c[3].w - 1.0f);
		return dot(r4, r4) < epsilon();
	}

	Vec3f rot3(const Vec3f &v3) const noexcept { return (*this) * Vec4f(v3, 0); }
	Vec3f ortho3(const Vec3f &v3) const noexcept { return (*this) * Vec4f(v3, 1); }
	Vec3f pers3(const Vec3f &v3) const noexcept { return ((*this) * Vec4f(v3, 1)).pers3(); }

	template<class T> std::vector<T> ortho3(const std::vector<T> &v0) const
	{
		std::vector<T> v1;
		v1.reserve(v0.size());
		for (const auto &v: v0) {
			v1.push_back(T(ortho3(Vec3f(v))));
		}
		return v1;
	}

	template<class T> std::vector<T> pers3(const std::vector<T> &v0) const
	{
		if (is_ortho()) {
			return ortho3(v0);
		}
		std::vector<T> v1;
		v1.reserve(v0.size());
		for (const auto &v: v0) {
			v1.push_back(T(pers3(Vec3f(v))));
		}
		return v1;
	}

	Mat4f trans(const Vec3f &t) const noexcept
	{
		return {c[0], c[1], c[2], c[3] + Vec4f(t)};  // only c[3] change
	}

	Mat4f scale(const Vec3f &s) const noexcept
	{
		auto sv = Vec4f(s, 1);
		return {c[0] * sv, c[1] * sv, c[2] * sv, c[3] * sv};
	}

	Mat4f scale(float s) const noexcept
	{
		auto sv = Vec4f(s, s, s, 1);
		return {c[0] * sv, c[1] * sv, c[2] * sv, c[3] * sv};
	}

	Mat4f transpose3() const;
	Mat4f transpose4() const;

	Mat4f rot_axis(float radian, const Vec3f &axis) const;
	Mat4f rot(const char *axes, ...) const;
	Mat4f normalize() const;
	Mat4f unitary_inverse() const;
	Mat4f precise_inverse(bool *flag = nullptr) const;
	Mat4f inverse() const;
	float det() const;
	double precise_det() const;

	bool set_orientation(const Vec3f *eye, const Vec3f *dir, const Vec3f *up);
	bool get_orientation(Vec3f *eye, Vec3f *dir, Vec3f *up) const;

	bool set_projection(
	        const float *fov, const float *aspect, const float *near, const float *far,
	        bool is_fovy = true);

	bool get_projection(float *fovy, float *aspect, float *near, float *far, bool is_fovy = true) const;
	void report(const char *str) const;

	template<class T, uint32_t M> friend Range<T, M> operator*(const Mat4f &m, const Range<T, M> &r0)
	{
		Range<T, M> r1;
		r1.invalidate();

		auto is_ortho = m.is_ortho();
		for (auto &src_point: r0.points()) {
			auto dst_point = m * Vec4f(src_point, 1);
			if (is_ortho) {
				r1.expand(dst_point);
			}
			else if (dst_point.w > 0) {
				r1.expand(dst_point.pers3());
			}
		}
		return r1;
	}

	Convex3f convex3f() const;
	Range3f range() const;
	std::vector<Vec3f> points() const;
	std::vector<Segment3f> segments() const;
	std::vector<Plane3f> planes() const;
	bool is_projectable(const std::vector<Vec3f> &points) const;

	bool inside(const Vec3f &point) const;
	bool inside(const std::vector<Vec3f> &points, std::vector<Vec3f> *inside_points = nullptr) const;
	bool inside(const Plane3f &plane, std::vector<Vec3f> *inside_points = nullptr) const;

	// bool inside(const Mat4f &frustum, std::vector<Vec3f> *inside_points = nullptr) const;
	bool intersect(const Mat4f &frustum) const;

	Mat4f shift(const std::vector<Vec3f> &points) const;
	Mat4f shift(const Rectf &from_viewport, const Rectf &to_viewport) const;

	friend bool operator==(const Mat4f &m0, const Mat4f &m1)
	{
		return memcmp(m0.data(), m1.data(), sizeof(Mat4f)) == 0;
	}
	friend bool operator!=(const Mat4f &m0, const Mat4f &m1) { return !(m0 == m1); }

	static Mat4f direction_matrix(const Vec3f &eye, const Vec3f &dir);
	static Mat4f cross_product_matrix(const Vec3f &v);
	static std::vector<Mat4f> cube_matrices(const Vec3f &capture_position);

	static Mat4f screentexc();
	static Mat4f texcscreen();
	static Mat4f texcfrag(const Vec4f &viewport);
	static Mat4f fragtexc(const Vec4f &viewport);

	static Mat4f projection(
	        double left, double right, double bottom, double top, double near, double far,
	        bool is_perspective = true);

	static Mat4f projection(const Vec3f &ray, const Plane3f &plane, bool is_perspective = true);
	static Mat4f tangentworld3(const Vec3f &dp0, const Vec3f &dp1, const Vec3f &dt0, const Vec3f &dt1);
	static Mat4f tangentworld4(
	        const Vec3f &p0, const Vec3f &p1, const Vec3f &p2, const Vec3f &t0, const Vec3f &t1,
	        const Vec3f &t2);

	static Mat4f reprojection(const std::vector<Vec2f> &from, const std::vector<Vec2f> &to);
	static Mat4f orbiting(
	        const Vec3f &target, float esec, float radius, float radius_amplitude, float radius_period,
	        float azimuth, float azimuth_period, float elevation, float elevation_amplitude,
	        float elevation_period, const Vec3f &up = {0, 1, 0});

protected:
	void set(
	        float x0, float x1, float x2, float x3, float y0, float y1, float y2, float y3, float z0,
	        float z1, float z2, float z3, float w0, float w1, float w2, float w3) noexcept
	{
		c[0] = {x0, y0, z0, w0};
		c[1] = {x1, y1, z1, w1};
		c[2] = {x2, y2, z2, w2};
		c[3] = {x3, y3, z3, w3};
	}
};

template<> inline Mat4f normalize(const Mat4f &m)
{
	return {
	        Vec4f(normalize<Vec3f>(m.c[0]), 0),
	        Vec4f(normalize<Vec3f>(m.c[1]), 0),
	        Vec4f(normalize<Vec3f>(m.c[2]), 0),
	        m.c[3],
	};
}

}  // namespace spu
