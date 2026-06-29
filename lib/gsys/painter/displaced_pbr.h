//
// DisplacedPBR :
//
#pragma once
#include "pbr.h"
#include <gsys/shaders/painter/pbr/ub_fbm_height.us>

namespace spu::gs_painter {

class DisplacedPBR : public PBR {
public:
	pbr::UB_FBM_HEIGHT ub_fbm_height;

	explicit DisplacedPBR(const char *name = nullptr) : PBR(name) {}
	explicit DisplacedPBR(const Attrs &attrs) : PBR() { init(attrs); }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void startInspector() override;

protected:
	bool def_use_mapworld = false;
	Mat4f m_mapworld = Mat4f().rot("x", pi() / 2);
	Mat4f u_worldmap;
	void doUse(uint32_t id) override;
};

}  // namespace spu::gs_painter
