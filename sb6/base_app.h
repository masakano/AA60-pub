//
// BaseApp :
//
#pragma once

#include "sb6composition.h"
#include "sb6ktx.h"
#include "sb6object.h"

#include <spu++/spu_page.h>
#include <smath/geometry.h>
#include <imgui/imgui.h>

namespace spu {

inline void loadShader(SpuShader &shader, const Attrs &shader_attrs, const Attrs &unif_attrs)
{
	Attrs pre_attrs = {
	        {"use_unif_block", true},
	};
	shader.init(pre_attrs + shader_attrs);
	shader.addUniforms(unif_attrs);
}

inline void loadShader(SpuShader &shader, const char *path, const Attrs &shader_attrs, const Attrs &unif_attrs)
{
	Attrs pre_attrs = {
	        {"use_unif_block", true},
	};
	shader.init(path, pre_attrs + shader_attrs);
	shader.addUniforms(unif_attrs);
}

class BaseApp : public SpuPage {
public:
	const Vec4f c_zero = {0.00, 0.00, 0.00, 0.00};
	const Vec4f c_black = {0.00, 0.00, 0.00, 1.00};
	const Vec4f c_green = {0.00, 0.25, 0.00, 1.00};
	const Vec4f c_blue = {0.00, 0.00, 0.25, 1.00};
	const Vec4f c_gray = {0.1, 0.1, 0.1, 1.0};

	const Mat4f c_unit;
	bool m_isPaused = false;
	bool m_isMenu = true;

	BaseApp(const char *name) : SpuPage(name, false) {}  // no depth test

	virtual void menu() { m_isMenu = false; }
	void drawFullscreenQuad() { spu_array_draw(0, GL_TRIANGLE_STRIP); }
	void end() override;
};
}  // namespace spu
