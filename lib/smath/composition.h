//
// Composition :
//
#pragma once
#include "mat4f.h"
#include <deque>

namespace spu {

struct Composition {
	Composition() : m_viewports(8, ezero<Rectf>()), m_scissors(8, ezero<Rectf>()), m_worldviews(2) {}

	void takeover(const Composition &composition);
	void takeover(
	        const Composition &composition, uint32_t src_worldview_index, uint32_t dst_worldview_index);

	const std::deque<Mat4f> &getWorldviews() const { return m_worldviews; }
	std::deque<Mat4f> &getWorldviews() { return m_worldviews; }

	const Mat4f &getViewscreen() const { return m_viewscreen; }
	Mat4f &getViewscreen() { return m_viewscreen; }

	const std::vector<Rectf> &getViewports() const { return m_viewports; }
	std::vector<Rectf> &getViewports() { return m_viewports; }

	const std::vector<Rectf> &getScissors() const { return m_scissors; }
	std::vector<Rectf> &getScissors() { return m_scissors; }

	void adjustViewscreen(uint32_t worldview_index, const std::vector<Vec3f> &points);
	void adjustDepth(uint32_t worldview_index, const std::vector<Vec3f> &points);
	void adjustAspect(uint32_t viewport_index);
	void adjustCamera(uint32_t worldview_index, const Range3f &range, const Vec3f &up, const Vec3f &dir);
	void report(const char *str) const;

	optional_t<Vec3f> intersectToPlane(
	        uint32_t worldview_index, const Plane3f &plane, const Vec2f &cursor_screen);

	Mat4f screenfrag(uint32_t viewport_index) const;
	Mat4f fragscreen(uint32_t viewport_index) const { return screenfrag(viewport_index).inverse(); }

	const Mat4f &worldview(uint32_t worldview_index) const { return m_worldviews.at(worldview_index); }
	const Mat4f &viewscreen() const { return m_viewscreen; }
	const Rectf &viewport(uint32_t viewport_index) const { return m_viewports.at(viewport_index); }
	const Rectf &scissor(uint32_t viewport_index) const { return m_scissors.at(viewport_index); }

	Mat4f viewworld(uint32_t worldview_index) const
	{
		return m_worldviews.at(worldview_index).unitary_inverse();
	}
	Mat4f worldscreen(uint32_t worldview_index) const
	{
		return m_viewscreen * m_worldviews.at(worldview_index);
	}
	Vec2f cursorInScreen(uint32_t viewport_index, const int16_t cursor[2]) const
	{
		return fragscreen(viewport_index).ortho3(Vec2f(cursor));
	}

	static Composition cubeComposition(const Vec3f &center, float near, float far);

private:
	std::vector<Rectf> m_viewports;
	std::vector<Rectf> m_scissors;
	std::deque<Mat4f> m_worldviews;
	Mat4f m_viewscreen;
};
}  // namespace spu
