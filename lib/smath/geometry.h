//
// Line3f :
//
#pragma once
#include "quatf.h"
#include "range.h"

namespace spu {

// clang-format off

template<class shape_t, class point_t = Vec3f>
concept has_distance = requires(const shape_t &x, const point_t &point, point_t *nearest) {
	{ x.distance(point, nearest) } -> std::same_as<decltype(point.x)>;
};

template<class shape_t>
concept has_transform = requires(const shape_t &x, const Mat4f &m) {
	{ m *x } -> std::same_as<shape_t>;
};

template<class shape_t, class point_t = Vec3f>
concept has_inside = requires(const shape_t &x, const point_t &point, const std::vector<point_t> &points) {
	{ x.inside(point) } -> std::same_as<bool>;
	{ x.inside(points) } -> std::same_as<std::vector<point_t>>;
};

// clang-format on
/// line in 3D space
struct Line3f {
	Vec3f eye;
	Vec3f dir;

	Line3f() = default;
	Line3f(const Vec3f &eye, const Vec3f &dir) : eye(eye), dir(normalize(dir)) {}

	float distance(const Vec3f &point, Vec3f *nearest = nullptr) const
	{
		auto d = point - eye;
		auto t = dot(dir, d) * dir;
		if (nearest) *nearest = eye - t;
		return length(d - t);
	}

	friend Line3f operator*(const Mat4f &m, const Line3f &line)  // generic
	{
		auto eye0 = m.pers3(line.eye);
		auto eye1 = m.pers3(line.eye + line.dir);
		auto dir = normalize(eye1 - eye0);
		return {eye0, dir};
	}
};
static_assert(has_distance<Line3f> && has_transform<Line3f>);

/// line segment in 3D space
struct Segment3f {
	Vec3f p0;  // start
	Vec3f p1;  // end

	Segment3f() = default;

	Segment3f(const Vec3f &p0, const Vec3f &p1) noexcept : p0(p0), p1(p1) {}

	float distance(const Vec3f &point, Vec3f *nearest = nullptr) const
	{
		auto n = p1 - p0;
		if (dot(point - p0, n) < 0) {
			if (nearest) *nearest = p0;
			return length(point - p0);
		}
		if (dot(point - p1, n) > 0) {
			if (nearest) *nearest = p1;
			return length(point - p1);
		}
		return Line3f(p0, n).distance(point, nearest);
	}

	friend Segment3f operator*(const Mat4f &m, const Segment3f &segment)
	{
		return {m.pers3(segment.p0), m.pers3(segment.p1)};
	}
};
static_assert(has_distance<Segment3f> && has_transform<Segment3f>);

/// 2D plane in 3D space
struct Plane3f {
	Vec4f eq;  // (a,b,c,d) : a^2 + b^2 + c^2 = 1

	Plane3f() = default;

	explicit Plane3f(const Vec4f &eq) noexcept : eq(eq / length(Vec3f(eq))) {}  //

	Plane3f(const Vec3f &center, const Vec3f &normal) : eq(Vec4f(normal, dot(-normal, center))) {}

	Plane3f(const Vec3f &p0, const Vec3f &p1, const Vec3f &p2)
	{
		auto normal = normalize(cross(p1 - p0, p2 - p0));
		eq = Vec4f(normal, dot(-normal, p0));
	}

	float distance(const Vec3f &point, Vec3f *nearest = nullptr) const
	{
		auto d = dot(eq, Vec4f(point, 1));
		if (nearest) *nearest = point - d * Vec3f(eq);
		return d;
	}

	void support(const std::vector<Vec3f> &points, float *min_d = nullptr, float *max_d = nullptr) const
	{
		auto det = [&](const Vec3f &p) { return dot(eq, Vec4f(p, 1)); };
		auto minmax = vector_minmax(points, det);
		if (min_d) *min_d = det(*minmax.first);
		if (max_d) *max_d = det(*minmax.second);
	}

	bool intersect(const Line3f &line, float *t = 0) const
	{
		auto det0 = dot<Vec3f>(eq, line.dir);
		auto det1 = dot<Vec4f>(eq, Vec4f(line.eye, 1));
		if (t) {
			*t = det0 ? -(det1 / det0) : 0.0f;
		}
		return det0 != 0;
	}

	bool intersect(const Segment3f &segment, Vec3f *nearest_point = 0) const
	{
		auto d0 = distance(segment.p0);
		auto d1 = distance(segment.p1);

		auto a0 = std::abs(d0);
		auto a1 = std::abs(d1);

		if (d0 * d1 < epsilon()) {
			if (nearest_point) {
				*nearest_point = lerp(segment.p0, segment.p1, a0 / (a0 + a1));
			}
			return true;
		}
		if (nearest_point) {
			*nearest_point = a0 < a1 ? segment.p0 : segment.p1;
		}
		return false;
	}
	bool intersect(const Plane3f &plane, Line3f *intersect_line = 0) const
	{
		auto ld = cross(eq, plane.eq);  // line directtion
		if (length(ld) > 0) {
			if (intersect_line) {
				// http://www.eyedeal.co.jp/product/eyemLib_caliper.pdf
				auto n1 = eq;
				auto n2 = plane.eq;
				auto dot12 = dot(n1, n2);
				auto dot11 = dot(n1, n1);
				auto dot22 = dot(n2, n2);
				auto dotld = dot(ld, ld);

				auto d1 = n1.w;
				auto d2 = n2.w;
				auto lp = ((dot12 * d2 - dot22 * d1) * n1 + (dot12 * d1 - dot11 * d2) * n2)
				        / dotld;
				*intersect_line = Line3f(lp, ld);
			}
			return true;
		}
		return false;
	}

	Plane3f flip() const { return Plane3f(-eq); }

	friend Plane3f operator*(const Mat4f &m, const Plane3f &plane)  // work in all case
	{
		return Plane3f((m.inverse().transpose4()) * plane.eq);
	}
};
static_assert(has_distance<Plane3f> && has_transform<Plane3f>);

/// sphere in 3D space
struct Sphere3f {
	Vec3f center;
	float radius;

	Sphere3f() = default;

	Sphere3f(const Vec3f &center, float radius) noexcept : center(center), radius(radius) {}

	float intersect(const Line3f &line, float *ts = nullptr) const
	{
		auto dir = center - line.eye;
		auto alpha = dot(line.dir, dir);
		auto dist = length(dir);
		auto det = alpha * alpha - (dist * dist - radius * radius);

		if (ts) {
			ts[0] = (alpha - sqrtf(det > 0 ? det : 0));
			ts[1] = (alpha + sqrtf(det > 0 ? det : 0));
		}
		return det;
	}

	float distance(const Vec3f &point, Vec3f *nearest = nullptr) const
	{
		if (nearest) *nearest = normalize(point - center) + center;
		return spu::distance(point, center);
	}

	friend Sphere3f operator*(const Mat4f &m, const Sphere3f &sphere)  // unitary only
	{
		return {m.ortho3(sphere.center), sphere.radius};
	}
};
inline Line3f unproject(const Vec2f &cursor, const Mat4f &worldcursor)
{
	auto cursorworld = worldcursor.inverse();
	auto near_world = cursorworld.pers3(Vec3f(cursor, -1));
	auto far_world = cursorworld.pers3(Vec3f(cursor, +1));
	auto dir_world = normalize(far_world - near_world);
	return Line3f(near_world, dir_world);
}

static_assert(has_distance<Plane3f> && has_transform<Plane3f>);

inline bool center_and_normal(const std::vector<Vec3f> &points, Vec3f &center, Vec3f &normal)
{
	center = vector_average(points);
	normal = ezero();
	auto d0 = points.back() - center;
	for (const auto &p: points) {
		const auto d1 = p - center;
		const auto c = cross(d0, d1);
		normal += dot(c, normal) >= 0 ? c : -c;  // '>=', not '>'
		d0 = d1;
	}

	const auto len = length(normal);
	if (len > epsilon()) {
		normal /= len;
		return true;
	}
	return false;
}

inline void quantize_and_uniq(std::vector<Vec3f> &points, float quantize_rate = 0.0f)
{
	if (quantize_rate > 0) {
		auto span = Range3f(points).span();
		auto quantize_step = span * quantize_rate;
		for (auto &p: points) {
			p = p.quantize(quantize_step);
		}
	}
	auto gt = [](const Vec3f &p0, const Vec3f &p1) { return greater(p0, p1); };
	auto eq = [](const Vec3f &p0, const Vec3f &p1) { return equal(p0, p1); };
	vector_sort_and_uniq(points, gt, eq);
}

std::vector<int32_t> convex2f_hull(const std::vector<Vec3f> &points, const Vec3f &normal);
std::vector<std::vector<int32_t>> convex3f_hull(const std::vector<Vec3f> &points, int32_t max_count = 16384);
std::vector<int32_t> triangulate(const std::vector<Vec3f> &points);
}  // namespace spu
