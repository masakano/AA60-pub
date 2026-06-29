//
// Birdview :
//
#pragma once
#include "base.h"
#include <gsys/canvas.h>
#include <gsys/node/camera.h>

namespace spu::gs_node::gui {
class Birdview : public Base {
public:
	explicit Birdview(const char *name = nullptr) : Base(name) {}
	explicit Birdview(const Attrs &attrs) : Birdview() { init(attrs); }

	void init(const Attrs &attrs) override;
	void update() override;

	const Camera &getCamera() const { return m_camera; }
	Camera &getCamera() { return m_camera; }

protected:
	GsCanvas m_canvas;
	Camera m_camera;
	virtual void coreDraw() {}
	void doRender() override;
};
}  // namespace spu::gs_node::gui
