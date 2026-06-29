//
// Convex3f :
//
#pragma once
#include "convex2f.h"

namespace spu {

class Convex3f {
public:
	Convex3f() = default;
	explicit Convex3f(const std::vector<Convex2f> &convexes);
	explicit Convex3f(const std::vector<std::vector<Vec3f>> &points_array);
	explicit Convex3f(const std::vector<Vec3f> &points);  // convex hull
	Convex3f(const std::vector<Vec3f> &points, const std::vector<std::vector<int32_t>> &indices_array);

	void autoflip();
	void marge();

	Range3f range() const;
	bool inside(const Vec3f &point) const;
	std::vector<Vec3f> inside(const std::vector<Vec3f> &point) const;
	float distance(const Vec3f &point, Vec3f *nearest = nullptr) const;
	bool intersect(const Plane3f &plane, std::vector<Segment3f> *intersect_segments = nullptr) const;
	bool intersect(const Convex3f &convex3, std::vector<Segment3f> *intersect_segments = nullptr) const;

	Vec3f center_of_gravity() const;
	Mat4f moment() const;

	std::vector<Vec3f> points() const;
	void modulate(const std::vector<Vec3f> &points);
	bool penetrate(const Line3f &ray, float *t = nullptr) const;

	const std::vector<Convex2f> &convexes() const { return m_convexes; }

	friend Convex3f operator*(const Mat4f &m, const Convex3f &c0)
	{
		Convex3f c1;
		c1.m_convexes.reserve(c0.m_convexes.size());
		for (const auto &convex: c0.m_convexes) {
			c1.m_convexes.push_back(m * convex);
		}
		return c1;
	}

	static Convex3f makeCylinder(int32_t nx, int32_t ny);

protected:
	std::vector<Convex2f> m_convexes;
};
}  // namespace spu
