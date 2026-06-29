//
// MMDMaterial :
//
#pragma once
#include <gsys/painter.h>
#include <gsys/painter/vertex.h>
#include <gsys/shaders/painter/mmd/mmd_common.us>

namespace spu::gs_painter {

class MMDMaterial {
public:
	mmd::UB_MMD_MATERIAL ub_mmd_material = {
	        .diffusemod = {eone<Vec4f>(), ezero<Vec4f>()},
	        .spheremod = {eone<Vec4f>(), ezero<Vec4f>()},
	        .toonmod = {eone<Vec4f>(), ezero<Vec4f>()},
	        .edge_color = {0, 0, 0, 1},
	        .screen_size = {1280, 720},
	        .edge_width = 1.0,
	        .edge_flag = 0,
	        .specular_power = 1.0,
	        .use_shadowmap = 1,
	        .diffusemap_mode = e_mmd_texture_none,
	        .spheremap_mode = e_mmd_texture_none,
	        .toonmap_mode = e_mmd_texture_none,
	};
	SpuTexture m_diffusemap = GsObject::defaultWhiteTexture();
	SpuTexture m_toonmap = GsObject::defaultWhiteTexture();
	SpuTexture m_spheremap = GsObject::defaultWhiteTexture();
};

class MMD : public GsPainter {
public:
	using Vertex = VertexP3N3T2;

	mmd::UB_MMD_MATERIAL ub_mmd_material;
	UB_LIGHT ub_light;
	uint32_t u_diffusemap = 0;
	uint32_t u_toonmap = 0;
	uint32_t u_spheremap = 0;

	MMD() = default;

	explicit MMD(const Attrs &attrs);
	void init(const Attrs &attrs) override;
	std::vector<MMDMaterial> &getMMDMaterials() { return m_materials; }
	const std::vector<MMDMaterial> &getMMDMaterials() const { return m_materials; }

	PAINTER_VERTEX_FUNCS;

protected:
	void doUse(uint32_t id) override;

private:
	std::vector<MMDMaterial> m_materials;
	void useSubRadiance(uint32_t id);
	void useRadiance(uint32_t id);
};
}  // namespace spu::gs_painter
