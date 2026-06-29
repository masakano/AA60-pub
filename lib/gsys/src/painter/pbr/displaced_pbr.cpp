//
// DisplacedPBR :
//
#include "displaced_pbr_inspector.h"

namespace spu::gs_painter {

void DisplacedPBR::init(const Attrs &attrs)
{
	Attrs init_attrs = {
	        {"path.radiance", "painter/pbr/displaced_radiance.us"},
	        {"path.depth",    "painter/pbr/displaced_depth.us"   },
	};
	PBR::init(init_attrs + attrs);

	Attrs shader_attrs = {
	        {"u_worldmap",    &u_worldmap   },
	        {"ub_fbm_height", &ub_fbm_height},
	};
	PBR::addUniforms(shader_attrs);

	ub_fbm_height = {
	        .use = false,
	        .lacunarity = 2,
	        .decay = 0.6,
	        .ridge_rate = 0.5,
	};
}

void DisplacedPBR::set(const Attrs &attrs)
{
	attrs.peek("mapworld_scale", "user 'ub_material.map_scale' instead");
	m_mapworld = *attrs.get("mapworld", &m_mapworld);
	GsPainter::set(attrs);
}

void DisplacedPBR::doUse(uint32_t id)
{
	auto map_scale = getDrawcalls().at(id).ub_material.map_scale;
	u_worldmap = m_mapworld.scale(1.0f / map_scale).inverse();
	GsPainter::doUse(id);
}

void DisplacedPBR::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new DisplacedPBRInspector(this);
	}
}

}  // namespace spu::gs_painter
