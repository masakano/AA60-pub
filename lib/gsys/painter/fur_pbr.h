//
// 
//
#pragma once
#include <gsys/painter/pbr.h>
#include <gsys/shaders/painter/pbr/ub_fur.us>

namespace spu::gs_painter {
class FurPBR : public PBR {
public:
	explicit FurPBR(const char *name = nullptr) : PBR(name) {}
	explicit FurPBR(const Attrs &attrs) : FurPBR() { init(attrs); }

	void init(const Attrs &attrs) override;
	void startInspector() override;

protected:
	friend class FurPBRInspector;
	pbr::UB_FUR ub_fur;
	uint32_t u_rand_texture = 0;
	void generateRandTexture();
};
}  // namespace spu::mview
