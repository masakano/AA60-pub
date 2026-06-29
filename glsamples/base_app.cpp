//
// BaseApp :
//
#include "base_app.h"

namespace spu {

BaseApp::BaseApp(const char *name, bool depth_test, const Vec4f &bgcolor0, float bgdepth, int32_t bgstencil)
        : SpuPage(name, depth_test, bgcolor0, bgdepth, bgstencil)
{
}

BaseApp::~BaseApp()
{
	for (auto &texture_id: m_ddsTextures) {
		spu_texture_delete(texture_id);
	}
}

void BaseApp::init(const Attrs &attrs)
{
	SpuPage::init(attrs);
	m_camera.setGesture(getGesture());
	m_camera.setIsAuto(attrs.get("auto", 0));
}

void BaseApp::begin()
{
	SpuPage::begin();
	m_camera.update();
}

std::vector<Rectf> BaseApp::makeViewports(int32_t nx, int32_t ny, int32_t border) const
{
	auto vp = viewport(0);
	auto s = Vec2f(vp.sx / nx, vp.sy / ny);
	auto w = s - Vec2f(border * 2);

	std::vector<Rectf> viewports;
	for (auto y = 0; y < ny; y++) {
		for (auto x = 0; x < nx; x++) {
			viewports.emplace_back(x * s.x + border, y * s.y + border, w.x, w.y);
		}
	}
	return viewports;
}

void BaseApp::loadShader(SpuShader &shader, const Attrs &shader_attrs, const Attrs &unif_attrs) const
{
	Attrs post_attrs = {
	        {"use_unif_block", true},
	};
	shader.init(shader_attrs + post_attrs);
	shader.addUniforms(unif_attrs);
}

uint32_t BaseApp::loadDDS(const char *path, const Attrs &attrs)
{
	dds::Image image(path, false);  // no flip
	m_ddsTextures.emplace_back(image.upload(attrs));
	return m_ddsTextures.back();
}

}  // namespace spu
