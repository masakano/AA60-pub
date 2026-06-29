//
// Terrain :
//
#pragma once
#include <gsys/node/manifold_2d.h>

namespace spu::gs_node {
class Terrain : public Manifold2D {
public:
	static constexpr hash32_t e_spread = "spread";

	explicit Terrain(const char *name = nullptr) : Manifold2D(name) {}
	explicit Terrain(const Attrs &attrs) : Terrain() { init(attrs); }

	void init(const Attrs &attrs) override;
	void startInspector() override;
	void update() override;
protected:
	Range1f m_heightRange = {0.0, 1.0};
	uint32_t m_updatedHeightmap = 0;
	void updateSubstances();
	void updateHeight();
	Range3f calcSpreadRange(const Mat4f &worldscreen) const;
	float calcFar(const Mat4f &viewfrag, const Vec3f &normal_view) const;
	int32_t calcLevel(const Mat4f &nodefrag, int32_t max_level) const;
	bool doBuild(const std::vector<Mat4f> &nodeworlds) override;
};
}  // namespace spu::gs_node
