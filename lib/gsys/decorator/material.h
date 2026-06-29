//
// Material :
//
#pragma once

#include <gsys/painter.h>
#include <gsys/shaders/default/ub_material.us>
#include <gsys/shaders/default/ub_light.us>
#include <gsys/shaders/default/ub_lightmap.us>

namespace spu::gs_decorator {

class Material : public GsDecorator {
public:
	Material(GsPainter *painter, const Attrs &attrs);

	Mat4f m_worldlightmap;

	UB_LIGHT ub_light;
	UB_MATERIAL ub_material;
	UB_LIGHTMAP ub_lightmap;

	uint32_t u_albedomap = 0;
	uint32_t u_specularmap = 0;
	uint32_t u_emissionmap = 0;
	uint32_t u_armmap = 0;
	uint32_t u_normalmap = 0;
	uint32_t u_heightmap = 0;
	uint32_t u_lightmap = 0;
	uint32_t u_irradmap = 0;
	uint32_t u_brdfmap = 0;

	Attrs uniforms() const override;
	void set(const Attrs &attrs) override;
	void doUse(uint32_t id) override;
};
}  // namespace spu::gs_decorator
