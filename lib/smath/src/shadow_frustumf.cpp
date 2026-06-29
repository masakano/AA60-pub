//
// Mat4f :
//
#include <smath/shadow_frustumf.h>
#include <smath/geometry.h>

namespace spu {
namespace {

[[maybe_unused]] bool penetrate(const std::vector<Plane3f> &planes, const Line3f &ray, Vec3f &cross_point)
{
	for (auto &plane0: planes) {
		float t;
		if (dot<Vec3f>(plane0.eq, ray.dir) < 0 && plane0.intersect(ray, &t) && t > 0) {
			cross_point = ray.eye + t * ray.dir;
			auto is_inside = true;
			for (const auto &plane1: planes) {
				if (&plane0 != &plane1 && plane1.distance(cross_point) > epsilon()) {
					is_inside = false;
					break;
				}
			}
			if (is_inside) return true;
		}
	}
	return false;
}
}  // namespace

Mat4f ShadowFrustumf::capture(const std::vector<Vec3f> &blocker_points) const
{
	auto blocker_center = vector_average(blocker_points);

	Mat4f shadow_worldscreen;
	if (m_lightType == e_point) {
		auto light_direction = normalize(m_lightPosition - blocker_center);
		auto worldview = Mat4f::direction_matrix(m_lightPosition, light_direction).unitary_inverse();
		auto viewscreen = Mat4f::projection(-1, 1, -1, 1, 1, 2, true);
		shadow_worldscreen = viewscreen * worldview;
	}
	else {
		auto worldview = Mat4f::direction_matrix(blocker_center, m_lightDirection).unitary_inverse();
		auto viewscreen = Mat4f::projection(-1, 1, -1, 1, 1, 2, false);
		shadow_worldscreen = viewscreen * worldview;
	}
	Mat4f shift_depth = {
	        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0,
	};
	return shift_depth * shadow_worldscreen.shift(blocker_points);
}

Mat4f ShadowFrustumf::prune(const Mat4f &worldscreen, const Mat4f worldshadow) const
{
	Range2f range;
	range.invalidate();

	for (auto &point: worldscreen.points()) {
		auto point_shadow = worldshadow * Vec4f(point, 1);
		if (point_shadow.w < 0) {
			return worldshadow;
		}
		range.expand(point_shadow.pers3());
	}
	range.p0 = max(range.p0, Vec2f(-1));
	range.p1 = min(range.p1, Vec2f(+1));
	auto span = range.span();

	auto viewport0 = Rectf(-1, -1, 2, 2);  // ofs=(-1,-1), size=(2,2)
	auto viewport1 = Rectf(range.p0.x, range.p0.y, span.x, span.y);
	auto new_worldshadow = worldshadow.shift(viewport0, viewport1);

#if 0
	{
		Range3f range(new_worldshadow.pers3(worldscreen.points()));
		range.report("worldscreen range");
	}
	{
		Range3f range(new_worldshadow.pers3(worldshadow.points()));
		range.report("worldshadow range");
	}
#endif
	return new_worldshadow;
}

#if 0
std::vector<Vec3f> ShadowFrustumf::prune(
        const Mat4f &worldscreen, const std::vector<Vec3f> &blocker_points) const
{
	auto clip_planes = worldscreen.planes();
	auto is_all_inside = true;

	std::vector<Vec3f> inside_points;
	for (auto &p: blocker_points) {
		auto is_inside = true;
		for (auto &plane: clip_planes) {
			if (plane.distance(p) > epsilon()) {
				is_inside = false;
				break;
			}
		}
		if (!is_inside) {
			Line3f ray;
			ray.eye = p;
			ray.dir = m_lightType == e_point ? p - m_lightPosition : -m_lightDirection;

			Vec3f cross_point;
			if (penetrate(clip_planes, ray, cross_point)) {
				inside_points.push_back(cross_point);
			}
			else {
				is_all_inside = false;
			}
		}
	}
	if (is_all_inside) {
		return blocker_points;
	}
	else {
		auto blocker_frustum = Mat4f(Range3f(blocker_points));  // not inside_points
		worldscreen.inside(blocker_frustum, &inside_points);
		return inside_points;
	}
}
#endif
}  // namespace spu
