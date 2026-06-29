//
// Composition :
//
#include <smath/composition.h>
#include <smath/geometry.h>
namespace spu {

void Composition::takeover(const Composition &composition)
{
	m_worldviews = composition.m_worldviews;
	m_viewscreen = composition.m_viewscreen;
}

void Composition::takeover(
        const Composition &composition, uint32_t src_worldview_index, uint32_t dst_worldview_index)
{
	m_worldviews.at(dst_worldview_index) = composition.m_worldviews.at(src_worldview_index);
	m_viewscreen = composition.m_viewscreen;
}

void Composition::adjustViewscreen(uint32_t worldview_index, const std::vector<Vec3f> &points)
{
	if (points.empty()) return;
	auto points_view = m_worldviews.at(worldview_index).ortho3(points);
	m_viewscreen = m_viewscreen.shift(points_view);
}

void Composition::adjustDepth(uint32_t worldview_index, const std::vector<Vec3f> &points)
{
	if (points.empty()) return;

	auto far = -huge();
	auto near = +huge();

	for (auto &p: m_worldviews.at(worldview_index).ortho3(points)) {
		far = std::max(far, -p.z);
		near = std::min(near, -p.z);
	}

	const auto c_max_near_far_ratio = 0.0001f;
	const auto c_far_margin_ratio = 1.01f;

	far *= c_far_margin_ratio;
	near = std::max(near, far * c_max_near_far_ratio);
	m_viewscreen.set_projection(nullptr, nullptr, &near, &far, true);
}

void Composition::adjustAspect(uint32_t viewport_index)
{
	const auto &viewport = m_viewports.at(viewport_index);
	assert(viewport.sx > 0 && viewport.sy > 0);
	auto aspect = viewport.sx / viewport.sy;
	m_viewscreen.set_projection(nullptr, &aspect, nullptr, nullptr);
}

void Composition::adjustCamera(
        uint32_t worldview_index, const Range3f &range, const Vec3f &up, const Vec3f &dir)
{
#if 1
	float fovy;
	m_viewscreen.get_projection(&fovy, nullptr, nullptr, nullptr);

	auto radius = length(range.span()) * 0.5;
	auto distance = radius / tanf(radians(fovy/2));
	auto eye = range.center() - normalize(dir) * distance;

	Mat4f viewworld;
	viewworld.set_orientation(&eye, &dir, &up);
	m_worldviews.at(worldview_index) = viewworld.unitary_inverse();

#else
	auto &worldview = m_worldviews.at(worldview_index);

	auto center = range.center();
	auto diameter = length(range.span());
	auto trial_distance = diameter * 2;
	auto eye = center - dir * trial_distance;

	Mat4f trial_viewworld;
	trial_viewworld.set_orientation(&eye, &dir, &up);

	auto worldscreen = m_viewscreen * trial_viewworld.unitary_inverse();
	auto range_screen = worldscreen * range;
	auto diameter_screen = length(range_screen.span());

	auto distance = trial_distance * 0.5 / diameter_screen;
	trial_viewworld.c[3] = Vec4f(center - dir * distance, 1);
	worldview = trial_viewworld.unitary_inverse();
#endif
}

optional_t<Vec3f> Composition::intersectToPlane(
        uint32_t worldview_index, const Plane3f &plane, const Vec2f &cursor_screen)
{
	auto ray = spu::unproject(cursor_screen, worldscreen(worldview_index));
	auto t = 0.0f;
	if (plane.intersect(ray, &t)) {
		return ray.eye + ray.dir * t;
	}
	else {
		return {false, ray.eye};
	}
}

Composition Composition::cubeComposition(const Vec3f &center, float near, float far)
{
	auto fovy = 90.0f;
	auto aspect = 1.0f;
	Composition c;

	auto &worldviews = c.getWorldviews();
	worldviews.clear();
	for (auto &worldview: Mat4f::cube_matrices(center)) {
		worldviews.push_back(worldview);
	}

	c.m_viewscreen.set_projection(&fovy, &aspect, &near, &far);
	return c;
}

Mat4f Composition::screenfrag(uint32_t viewport_index) const
{
	const auto &viewport = m_viewports.at(viewport_index);
	auto scale = Vec3f(viewport.sx / 2, viewport.sy / 2, 0.5);
	auto offset = Vec3f(viewport.ox, viewport.oy, 0.0);
	return Mat4f().scale(scale).trans(offset + scale);
}

void Composition::report(const char *str) const
{
	if (str) printf("%s:\n", str);

	for (auto &worldview: m_worldviews) {
		// if (!equal(worldview, Mat4f())) {
		if (worldview != Mat4f()) {
			worldview.report("  worldview");
		}
	}
	m_viewscreen.report("  viewscreen");

	for (auto &viewport: m_viewports) {
		if (!equal(viewport, ezero<Vec4f>())) {
			viewport.report("  viewport");
		}
	}
	for (auto &scissor: m_scissors) {
		if (!equal(scissor, ezero<Vec4f>())) {
			scissor.report("  scissor");
		}
	}
}
}  // namespace spu
