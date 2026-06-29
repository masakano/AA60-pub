//
// Composition :
//
#pragma once

#include <smath/geometry.h>

namespace spu::sb6 {
class Composition {
public:
	const Mat4f &worldview() const { return m_worldview; }
	const Mat4f &viewscreen() const { return m_viewscreen; }
	const Mat4f worldscreen() const { return m_viewscreen * m_worldview; }

	void lookat(const Vec3f &eye, const Vec3f &center, const Vec3f &up)
	{
		auto dir = Vec3f(center - eye);
		m_worldview.set_orientation(&eye, &dir, &up);
		m_worldview = m_worldview.inverse();
	}

	void perspective(const Rectf &viewport, float fovx, float near, float far)
	{
		auto aspect = viewport.sx / viewport.sy;
		m_viewscreen.set_projection(&fovx, &aspect, &near, &far);
	}

	void frustum(const Rectf &viewport, double near, double far)
	{
		auto aspect = viewport.sx / viewport.sy;
		m_viewscreen = Mat4f::projection(-aspect, aspect, 1.0, -1.0, near, far);
	}

private:
	Mat4f m_worldview;
	Mat4f m_viewscreen;
};
}  // namespace spu::sb6
