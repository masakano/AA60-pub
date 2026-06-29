//
// Tess :
//
#pragma once
#include <gsys/painter.h>

namespace spu::gs_decorator {
class Tess : public GsDecorator {
public:
	float u_tess_min_len = 0.02;
	float u_tess_max = 32;
	Tess(GsPainter *painter, const Attrs &attrs);
	Attrs uniforms() const override;

protected:
	bool m_doPatch = true;
	void doUse(uint32_t id) override;
};
}  // namespace spu::gs_decorator
