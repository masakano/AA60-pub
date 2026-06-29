//
// Dof :
//
#pragma once
#include <gsys/canvas/tonemap.h>

namespace spu::gs_canvas {

/// depth of field
class Dof : public Tonemap {
public:
	explicit Dof(const char *name = nullptr) : Tonemap(name) {}
	explicit Dof(const Attrs &attrs) : Dof() { init(attrs); }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void render() override;
	void startInspector() override;

protected:
	friend class DofInspector;
	GsCanvas m_gauss0;
	GsCanvas m_gauss1;

	Mat4f u_texcview;
	Vec2f u_texture_size;
	float u_aperture_radius = 0.0;
	float u_focal_distance = 1.0;
	int32_t u_show_focal_point = 0;

	Vec4f u_footstep = {1, 0, 0, 1};
	uint32_t u_target = GL_TEXTURE_2D;
};
}  // namespace spu::gs_canvas
