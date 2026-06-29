//
// ObjectPicker :
//
#pragma once
#include "convex3f.h"
#include "composition.h"
#include "smath/geometry.h"

namespace spu {

class ObjectPicker {
public:
	struct Prim {
		Convex3f convex3;
		Vec3f center;
		Transformf transform;

		Prim(const Convex3f &convex3, const Transformf &transform = Transformf())
		        : convex3(convex3), transform(transform)
		{
			std::vector<Vec3f> points;
			for (auto &convex: convex3.convexes()) {
				vector_cat(points, convex.points());
			}
			center = vector_average(points);
		}

		Prim(const Mat4f &frustum, const Transformf &transform = Transformf())
		        : Prim(frustum.convex3f(), transform)
		{
		}
		Prim(const Range3f &range, const Transformf &transform = Transformf())
		        : Prim(Mat4f(range), transform)
		{
		}
	};

	void add(const Prim &prim);
	void pick(const Vec2f &cursor_screen, bool is_trans);
	void drop();

	void drag(const Vec2f &cursor_screen);
	Range3f getRange() const;

	void clear() { m_prims.clear(); }
	Prim &getPrim(int32_t index) { return m_prims.at(index); }
	const Prim &getPrim(int32_t index) const { return m_prims.at(index); }

	int32_t primCount() const { return m_prims.size(); }
	int32_t primIndex() const { return m_primIndex; }

	Composition &getComposition() { return m_composition; }
	const Composition &getComposition() const { return m_composition; }
	void useFixedAxis(bool use) { m_useFixedAxis = use; }
	void useWeightCenter(bool use) { m_useWeightCenter = use; }

private:
	std::vector<Prim> m_prims;
	Composition m_composition;

	Plane3f m_intersectPlane;
	// Sphere3f m_intersectSphere;
	Transformf m_transform;

	Mat4f m_nodescreen;
	Vec2f m_cursor;
	Vec3f m_intersectPoint;
	// float m_minRotW;
	uint32_t m_rotationMask = 0;
	int32_t m_primIndex = -1;
	bool m_isTranslate = false;
	bool m_useWeightCenter = true;
	bool m_useFixedAxis = true;
};
}  // namespace spu
