//
// App :
//
#pragma once
#include <spu++/spu++.h>

namespace spu {
static constexpr int c_win_sx = 1280;
static constexpr int c_win_sy = 720;

class App {
public:
	SpuFrame m_frame;
	Vec4f m_bgcolor0 = {1.0, 1.0, 1.0, 1.0};
	Vec4f m_whitecolor = {1, 1, 1, 1};
	Rectf m_viewport0 = {0, 0, 1280, 720};
	Rectf m_viewport1 = {0, 0, 1280, 720};

	App()
	{
		m_frame.reset(0);

		SpuRenderstate renderstate(-1);
		renderstate.flags.blend = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
		renderstate.use();
	}

	virtual ~App() = default;

	virtual void init() = 0;
	virtual int32_t width() const = 0;
	virtual int32_t height() const = 0;
	virtual void doDisplay() = 0;

	void display()
	{
		Attrs frame_attrs0 = {
		        {"bgcolor0",  m_bgcolor0 },
		        {"viewport0", m_viewport0},
		};
		m_frame.set(frame_attrs0);
		m_frame.clear();

		auto ox = (1280 - width()) / 2;
		auto oy = (720 - height()) / 2;
		m_viewport1 = Rectf(ox, oy, width(), height());
		Attrs frame_attrs1 = {
		        {"viewport0", m_viewport1},
		};
		m_frame.set(frame_attrs1);
		doDisplay();
	}
};
}  // namespace spu
