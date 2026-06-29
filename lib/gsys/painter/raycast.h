//
// Raycast :
//
#pragma once
#include <gsys/painter.h>

namespace spu::gs_painter {
class Raycast : public GsPainter {
public:
	uint32_t u_volume = 0;
	uint32_t u_clut = 0;
	uint32_t u_occlusion = 0;
	uint32_t u_depth = 0;
	float u_volume_scale = 1.0;

	Mat4f u_viewvolume;
	Mat4f u_fragview;

	explicit Raycast(const Attrs &attrs);
	~Raycast() {}
	void init(const Attrs &attrs) override;

protected:
	void doUse(uint32_t id) override;
};
}  // namespace spu::gs_painter
