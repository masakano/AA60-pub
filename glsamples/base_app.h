//
// BaseApp :
//
#pragma once
#include <spu++/dds/dds.h>
#include <spu++/spu_page.h>
#include "camera.h"
#include "conditional_draw.h"
#include "multisample_control.h"
#include "vertex.h"

namespace spu {

inline const Vec4f c_black = {0.0, 0.0, 0.0, 1.0};
inline const Vec4f c_white = {1.0, 1.0, 1.0, 1.0};
inline const Vec4f c_gray = {0.2, 0.2, 0.2, 0.2};
inline const Vec4f c_sky = {0.0, 0.5, 1.0, 1.0};
inline const Vec4f c_orange = {1.0, 0.5, 0.0, 1.0};

class BaseApp : public SpuPage {
public:
	BaseApp(const char *name, bool depth_test = true, const Vec4f &bgcolor0 = Vec4f(0.1, 0.1, 0.1, 1.0),
	        float bgdepth = 1.0, int32_t stencil = -1);
	~BaseApp();

	void init(const Attrs &attrs) override;
	void begin() override;

	std::vector<Rectf> makeViewports(int32_t nx, int32_t ny, int32_t border = 0) const;
	Camera &getCamera() { return m_camera; }

	void loadShader(SpuShader &shader, const Attrs &shader_attrs, const Attrs &unif_attrs) const;
	uint32_t loadDDS(const char *path, const Attrs &attrs = Attrs());

private:
	Camera m_camera;
	std::vector<uint32_t> m_ddsTextures;
};

}  // namespace spu
