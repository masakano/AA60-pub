//
// ShadowmapBirdview :
//
#pragma once

#include <gsys/canvas/shadowmap.h>
#include <gsys/node/gui/tweakbar.h>
#include <gsys/painter/stdout.h>

namespace spu::gs_canvas {

class ShadowmapBirdview : public gs_node::gui::Birdview {
public:
	ShadowmapBirdview(const Attrs &attrs);

protected:
	Shadowmap *m_shadowmap = nullptr;
	Range3f m_range;
	void coreDraw() override;
};

class ShadowmapInspector : public gs_node::gui::Tweakbar {
public:
	using base_t = gs_node::gui::Tweakbar;
	ShadowmapInspector(Shadowmap *shadowmap);
	void update() override;

protected:
	Shadowmap *m_shadowmap = nullptr;
	Tweakbar m_birdviewTweakbar;
	int32_t m_isBirdview = 0;
	void birdviewUpdate();
	void coreUpdate();
};
}  // namespace spu::gs_canvas
