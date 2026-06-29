//
// Mcube :
//
#pragma once
#include <gsys/painter.h>

namespace spu::gs_painter {

class Mcube : public GsPainter {
public:
	Mat4f m_gridvolume;
	Mat4f u_gridworld;
	Vec4i u_grid_box_size;

	float u_volume_threshold = 0.5;
	float u_volume_scale = 1.0;
	uint32_t u_edges = 0;
	uint32_t u_vertices = 0;
	uint32_t u_volume = 0;

	explicit Mcube(const Attrs &attrs);
	~Mcube();
	void init(const Attrs &attrs) override;
	// PAINTER_NO_VERTEX_FUNCS;

protected:
	void doUse(uint32_t id) override;
};
}  // namespace spu::gs_painter
