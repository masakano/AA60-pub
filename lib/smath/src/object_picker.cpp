//
// ObjectPicker :
//
#include <smath/object_picker.h>

namespace spu {

void ObjectPicker::add(const Prim &prim) { m_prims.push_back(prim); }

void ObjectPicker::pick(const Vec2f &cursor_screen, bool is_translate)
{
	auto t_min = huge();
	m_cursor = cursor_screen;
	m_isTranslate = is_translate;
	m_primIndex = -1;

	for (auto &prim: m_prims) {
		auto nodescreen = m_composition.worldscreen(0) * prim.transform;
		auto ray = unproject(cursor_screen, nodescreen);  // in node corrdinate
		auto t = 0.0f;

		if (prim.convex3.penetrate(ray, &t)) {
			if (t < t_min) {
				t_min = t;
				m_primIndex = &prim - &m_prims[0];
				m_transform = prim.transform;
				m_nodescreen = nodescreen;
				m_intersectPoint = ray.eye + t_min * ray.dir;
				m_intersectPlane = Plane3f(m_intersectPoint, ray.dir);
				m_rotationMask = 0;
			}
		}
	}
}

void ObjectPicker::drop() { m_primIndex = -1; }

void ObjectPicker::drag(const Vec2f &cursor_screen)
{
	if (m_primIndex < 0) {
		return;
	}

	auto &prim = m_prims[m_primIndex];
	auto ray = unproject(cursor_screen, m_nodescreen);  // node coordinate

	float t;
	m_intersectPlane.intersect(ray, &t);
	auto new_intersect_point = ray.eye + ray.dir * t;

	if (m_isTranslate) {
		auto translation = new_intersect_point - m_intersectPoint;
		prim.transform = m_transform * Transformf(translation);
	}
	else {
		auto rotation_center = m_useWeightCenter ? prim.center : ezero();
		auto p0 = normalize(m_intersectPoint - rotation_center);
		auto p1 = normalize(new_intersect_point - rotation_center);
		auto rotation = Quatf::from_target(p1, p0);

		if (m_useFixedAxis) {
			auto rotw = abs(rotation.w);
			if (rotw > 0.999) {  // need parameterize
				auto abs_axis = abs(rotation.axis());
				auto axis_max = std::max({abs_axis.x, abs_axis.y, abs_axis.z});
				m_rotationMask = (abs_axis >= Vec3f(axis_max)).pack() | 0xf000;
			}
			if (m_rotationMask) {
				rotation = Quatf(
				        normalize(select(m_rotationMask, Vec4f(rotation), ezero<Vec4f>())));
				prim.transform
				        = m_transform * Transformf(rotation_center, rotation, -rotation_center);
			}
		}
		else {
			prim.transform = m_transform * Transformf(rotation_center, rotation, -rotation_center);
		}
	}
}

Range3f ObjectPicker::getRange() const
{
	Range3f range;
	range.invalidate();
	for (auto &prim: m_prims) {
		auto transform = Mat4f(prim.transform);
		for (auto &convex: prim.convex3.convexes()) {
			range.expand((transform * convex).points());
		}
	}
	return range;
}
}  // namespace spu
