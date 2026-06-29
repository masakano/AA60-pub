//
// DisplacedPBRInspector :
//
#pragma once
#include <gsys/painter/displaced_pbr.h>
#include "pbr_inspector.h"

namespace spu::gs_painter {

class DisplacedPBRInspector : public PBRInspector {
public:
	using base_t = PBRInspector;

	DisplacedPBRInspector(DisplacedPBR *painter)
	        : PBRInspector(painter), ub_fbm_height(painter->ub_fbm_height)
	{
		base_t::addStdSlider("map_scale", 0.0, 8.0, &ub_material.map_scale, 4.0);
		base_t::addStdSlider("height_scale", 0.0, 8.0, &ub_material.height_scale, 4.0);
		base_t::addStdButton("use fbm", &ub_fbm_height.use);
		base_t::addStdSlider("fbm lacunarity", 0.0, 4.0, &ub_fbm_height.lacunarity);
		base_t::addStdSlider("fbm decay", 0.0, 1.0, &ub_fbm_height.decay);
		base_t::addStdSlider("fbm rige_rate", 0.0, 1.0, &ub_fbm_height.ridge_rate);

		base_t::bake();
		changeTerrainState();
	}

	void doUpdate() override
	{
		changeTerrainState();
		for (auto &drawcall: m_painter->getDrawcalls()) {
			drawcall.ub_material.map_scale = ub_material.map_scale;
			drawcall.ub_material.height_scale = ub_material.height_scale;
		}
	}

private:
	pbr::UB_FBM_HEIGHT &ub_fbm_height;

	void changeTerrainState()
	{
		auto state = ub_fbm_height.use ? e_active : e_disabled;
		std::vector<const void *> ptrs = {
		        &ub_fbm_height.lacunarity,
		        &ub_fbm_height.decay,
		        &ub_fbm_height.ridge_rate,
		};
		base_t::changeState(ptrs, state);
	}
};
}  // namespace spu::gs_painter
