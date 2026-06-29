//
// Gjk :
//
#pragma once
#include "convex3f.h"

namespace spu {

class Gjk {
public:
	enum Status {
		e_inside = 1,
		e_outside = 0,
		e_timeout = -1,
	};

	struct Face {
		Vec3f p0, p1, p2;
		Vec4f eq;
		Face(const Vec3f &p0, Vec3f &p1, Vec3f &p2) : p0(p0), p1(p1), p2(p2)
		{
			auto normal = normalize(cross(p1 - p0, p2 - p0));
			eq = Vec4f(normal, dot(-normal, p0));
			if (eq.w > 0) eq = -eq;  // expanding dir
		}
	};

	struct Edge {
		Vec3f p0, p1;
		bool operator==(const Edge &e)
		{
			return ((equal(p0, e.p0) && equal(p1, e.p1)) || (equal(p0, e.p1) && equal(p1, e.p0)));
		}
	};

	std::vector<Vec3f> m_pointsA;
	std::vector<Vec3f> m_pointsB;

	int32_t m_gjkCount = 0;
	int32_t m_epaCount = 0;
	int32_t m_maxGjkCount = 16;
	int32_t m_maxEpaCount = 16;

	Vec3f m_p0;
	Vec3f m_p1;
	Vec3f m_p2;
	Vec3f m_p3;

	Vec3f m_nearest = ezero();
	Vec3f m_prevNearest = ezero();

	Gjk(const std::vector<Vec3f> &node_pointsA, const std::vector<Vec3f> &node_pointsB);
	Status gjk();
	std::vector<Face> epa();

	Vec3f support(const Vec3f &dir, const std::vector<Vec3f> &points) const;
	Vec3f support(const Vec3f &dir) const;
};
}  // namespace spu
