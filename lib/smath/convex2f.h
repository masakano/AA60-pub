//
// Convex2f :
//
#pragma once
#include "geometry.h"
namespace spu {

class Convex2f {
public:
	Convex2f() = default;
	Convex2f(const std::vector<Vec3f> &points, const Plane3f &plane) : m_points(points), m_plane(plane) {}
	Convex2f(const std::vector<Vec3f> &points, const std::vector<int32_t> &indices);
	Convex2f(const std::vector<Vec3f> &points, bool is_qhull);

	void flip();
	bool inside(const Vec3f &point) const;
	std::vector<Vec3f> inside(const std::vector<Vec3f> &point) const;

	float distance(const Vec3f &point, Vec3f *nearest = nullptr) const;

	bool intersect(const Line3f &line, Vec3f *intersect_point = nullptr) const;
	bool intersect(const Segment3f &segment, Vec3f *intersect_point = nullptr) const;
	bool intersect(const Plane3f &plane, Segment3f *intersect_segment = nullptr) const;
	bool intersect(const Convex2f &convex, Segment3f *intersect_segment = nullptr) const;

	Convex2f cup(const Convex2f &convexB) const;
	Convex2f cap(const Convex2f &convexB) const;
	std::vector<Vec3f> walls() const;

	const std::vector<Vec3f> &points() const { return m_points; }
	const Plane3f &plane() const { return m_plane; }

	void modulate(const std::vector<Vec3f> &points);
	friend Convex2f operator*(const Mat4f &m, const Convex2f &c0);

protected:
	std::vector<Vec3f> m_points;
	Plane3f m_plane;
	void remap();
};
}  // namespace spu
