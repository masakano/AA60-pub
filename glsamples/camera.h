//
// Camera :
//
#pragma once
#include <spu++/spu++.h>

namespace spu {

class Camera {
public:
	Camera() = default;
	Camera(SpuGesture *gesture) { m_gesture = gesture; }

	void update()
	{
		auto &curr = m_gesture->curr();
		auto &prev = m_gesture->prev();

		if (curr.mouse_R && !prev.mouse_R) {
			m_anchorRot = m_rot;
		}
		else if (curr.mouse_R && prev.mouse_R) {
			auto *anchor = m_gesture->anchorR();
			Vec2f delta = {
			        float(curr.cursor[0] - anchor[0]) / float(curr.winsize[0]) * pi(),
			        float(curr.cursor[1] - anchor[1]) / float(curr.winsize[1]) * pi(),
			};
			m_rot = m_anchorRot + delta;
		}
		else if (m_isAuto) {
			// auto dsec = getSeconds().delta();
			auto dsec = 1.0 / 60;
			m_rot += dsec * 0.50;
		}
		m_offset += (curr.wheel - prev.wheel) * 0.1f;
		m_worldview = Mat4f().rot("x", m_rot.y).rot("y", m_rot.x).trans({0.0, 0.0, -m_offset});
	}

	void setViewscreen(
	        double left, double right, double bottom, double top, double near, double far,
	        bool is_perspective = true)
	{
		m_viewscreen = Mat4f::projection(left, right, bottom, top, near, far, is_perspective);
	}

	void setViewscreen(const Rectf &viewport, float fovy, float near, float far)
	{
		auto aspect = viewport.sx / viewport.sy;
		m_viewscreen.set_projection(&fovy, &aspect, &near, &far);
	}

	void setWorldview(const Vec3f &eye, const Vec3f &center, const Vec3f &up)
	{
		auto dir = Vec3f(center - eye);
		m_worldview.set_orientation(&eye, &dir, &up);
		m_worldview = m_worldview.inverse();
	}

	const Mat4f &worldview() const { return m_worldview; }
	const Mat4f &viewscreen() const { return m_viewscreen; }
	Mat4f worldscreen() const { return m_viewscreen * m_worldview; }
	Vec3f position() const { return Vec3f(0, 0, -m_offset); }

	SpuGesture *getGesture() const { return m_gesture; }
	void setGesture(SpuGesture *gesture) { m_gesture = gesture; }
	void setIsAuto(bool is_auto) { m_isAuto = is_auto; }

private:
	SpuGesture *m_gesture = nullptr;
	;
	Mat4f m_viewscreen;
	Mat4f m_worldview;
	float m_offset = 5.0;
	Vec2f m_rot = {0, 0};
	Vec2f m_anchorRot = {0, 0};
	bool m_isAuto = false;
};

}  // namespace spu
