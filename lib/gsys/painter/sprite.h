//
// Sprite :
//
#pragma once

#include <gsys/painter.h>

namespace spu::gs_painter {

class Sprite : public GsPainter {
public:
	struct Vertex {
		Vec3f p = {0, 0, 0};
		Vec4f c = {1, 1, 1, 1};
		Vec4f t = {0, 0, 1, 1};      // ( u0, v0, u1, v1)
		Vec4f s = {-1, -1, +1, +1};  // (dx0,dy0,dx1,dy1)

		operator Vec3f() const { return p; }
	};

	explicit Sprite(const char *name = nullptr) : GsPainter(name) {}
	explicit Sprite(const Attrs &attrs) : Sprite() { init(attrs); }

	void init(const Attrs &attrs) override;
	void update() override;

	Mat4f u_nodeview;
	Mat4f u_viewscreen;
	Vec4f u_albedo = eone<Vec4f>();
	float u_min_alpha = 0.0;
	uint32_t u_albedomap = 0;

	PAINTER_VERTEX_FUNCS_WITHOUT_MESH;

protected:
	void doUse(uint32_t id) override;

private:
};
}  // namespace spu::gs_painter
