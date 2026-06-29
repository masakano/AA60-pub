//
// CubeShadowmapInspector :
//
#pragma once
#include "shadowmap_inspector.h"
#include <gsys/canvas/shadowmap.h>

namespace spu {
namespace canvas {

class CubeShadowmapInspector : public ShadowmapInspector {
public:
	CubeShadowmapInspector(CubeShadowmap *shadowmap) : ShadowmapInspector(shadowmap) {}

	void update(int64_t dusec) override
	{
		m_birdview.begin(true);

		for (auto &node: m_shadowmap->getNodes()) {
			if (!node->hasDepthShader()) {
				continue;
			}

			auto &instance = node->getInstance();
			auto &ranges = node->getRanges();

			for (auto slot = 0; instance.getTransformPtr(slot, 0); slot++) {
				for (auto index = 0; instance.getTransformPtr(slot, index); index++) {
					Mat4f instanceworld
					        = instance.get(instance.getTransformPtr(slot, index));
					m_birdview.setColor("red");
					m_birdview.addPrim(instanceworld * ranges[slot]);
				}
			}
		}

		for (auto &c: m_shadowmap->getCompositions()) {
			m_birdview.setColor("lightgreen");
			auto worldshadow = c.lightshadow * c.worldlight;
			m_birdview.addPrim(worldshadow);
		}
		m_birdview.end();
		ShadowmapInspector::update(dusec);
	}
};
}  // namespace canvas
}  // namespace spu
