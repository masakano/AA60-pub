//
// Copy :
//
#pragma once
#include <gsys/canvas.h>

namespace spu::gs_canvas {

class Copy : public GsCanvas {
public:
	explicit Copy(const char *name = nullptr) : GsCanvas(name) {}
	explicit Copy(const Attrs &attrs) : Copy() { init(attrs); }
	void init(const Attrs &attrs) override;
	void render() override;
	uint32_t u_target = GL_TEXTURE_2D;
};
}  // namespace spu::gs_canvas
