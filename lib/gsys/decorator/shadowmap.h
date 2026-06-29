//
// Shadowmap :
//
#pragma once
#include <gsys/painter.h>
#include <gsys/canvas/shadowmap.h>

namespace spu::gs_decorator {

class Shadowmap : public GsDecorator {
public:
	UB_SHADOWMAP ub_shadowmap;
	UB_SHADOWMAP_PATTERN ub_shadowmap_pattern;
	uint32_t u_shadowmap = 0;

	Shadowmap(GsPainter *painter, const Attrs &attrs);
	~Shadowmap() = default;
	// void set(const Attrs &attrs) override;
	Attrs uniforms() const override;

protected:
	gs_canvas::Shadowmap *m_shadowmap = nullptr;
	struct Cache {
		int32_t blocker_pattern = 0;
		int32_t receiver_pattern = 0;
	} m_cache;

	void doUse(uint32_t id) override;
};
}  // namespace spu::gs_decorator
