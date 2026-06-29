//
// PMDActor :
//
#include "pmd.h"
#include <gsys/painter/mmd.h>
#include <gsys/painter/stdout.h>
#include <gsys/util/lambert_to_pbr.h>
#include <smath/color_chart.h>

namespace spu::gs_node::mmd {

void PMDActor::replacePainter(GsPainter *painter)
{
	const auto mmd_dir = m_path.parent_path();
	mmd::PMDFile desc(m_path);
	desc.load();

	// hdaer
	setName(desc.header.name());
	setCommon(painter, desc, false);
	MMDActor::replacePainter(painter);

	// drawcall (common)
	{
		auto first = 0;
		auto &drawcalls = painter->getDrawcalls();
		drawcalls.clear();

		for (const auto &dmat: desc.materials) {
			GsDrawcall drawcall;

			if (!dmat.texture_name.empty()) {
				const auto names = extract_from_string(dmat.texture_name, "*");
				const auto texture_path = mmd_dir / names[0];
				drawcall.albedomap = loadTexture(texture_path);
			}

			drawcall.coms[0].mode = GL_TRIANGLES;
			drawcall.coms[0].first = first;
			drawcall.coms[0].count = dmat.count;
			drawcall.flags.blend = true;  // need check
			drawcall.flags.ccw = true;
			drawcall.ub_material.min_alpha = false;  // need check

			auto diffuse = srgb_to_linear(dmat.diffuse);
			auto ambient = srgb_to_linear(dmat.ambient);
			auto specular = srgb_to_linear(dmat.specular);

			GsLambertToPBR ltb(diffuse, ambient, specular, dmat.specular_power);

			drawcall.ub_material.albedo = diffuse;
			drawcall.ub_material.metallic = ltb.metallic();
			drawcall.ub_material.roughness = ltb.roughness();
			drawcall.ub_material.ao = ltb.ao();
			drawcall.poly_offset = {4.0, 1.0};  // need parameterize

			drawcalls.push_back(drawcall);
			first += dmat.count;
		}
	}

	// MMDMaterial
	auto *mmd_painter = dynamic_cast<gs_painter::MMD *>(painter);
	if (mmd_painter) {
		std::vector<SpuTexture> toonmaps;
		for (const auto &texture_name: desc.toonmap_names) {
			auto texture_path = mmd_dir / texture_name;
			Attrs texture_attrs = {
			        {"wrap_s", GL_CLAMP_TO_EDGE},
			        {"wrap_t", GL_CLAMP_TO_EDGE},
			};
			if (std::filesystem::exists(texture_path)) {
				toonmaps.push_back(loadTexture(texture_path, texture_attrs));
			}
			else {
				const std::filesystem::path c_default_path = "painter/mmd";
				texture_path = c_default_path / texture_name;
				toonmaps.push_back(loadTexture(texture_path, texture_attrs));
			}
		}

		auto &mmd_materials = mmd_painter->getMMDMaterials();
		for (const auto &dmat: desc.materials) {
			gs_painter::MMDMaterial mat;
			auto &ub = mat.ub_mmd_material;
			ub.diffuse = srgb_to_linear(dmat.diffuse);
			ub.specular = srgb_to_linear(dmat.specular);
			ub.ambient = srgb_to_linear(dmat.ambient);
			ub.specular_power = dmat.specular_power;
			ub.edge_flag = dmat.edge_flag;
			ub.edge_width = dmat.edge_flag == 0 ? 0.0F : 1.0F;

			if (!dmat.texture_name.empty()) {
				auto names = extract_from_string(dmat.texture_name, "*");
				std::filesystem::path diffusemap_name;
				std::filesystem::path spheremap_name;

				switch (names.size()) {
				case 1: diffusemap_name = mmd_dir / names[0]; break;
				case 2:
					diffusemap_name = mmd_dir / names[0];
					spheremap_name = mmd_dir / names[1];

					break;
				default: assert(0);
				}

				mat.m_diffusemap = loadTexture(diffusemap_name);
				ub.diffusemap_mode = e_mmd_texture_mul;

				if (!spheremap_name.empty()) {
					mat.m_spheremap = loadTexture(spheremap_name);
					auto ext = spheremap_name.extension();
					if (ext == ".sph") {
						ub.spheremap_mode = e_mmd_texture_mul;
					}
					else if (ext == ".spa") {
						ub.spheremap_mode = e_mmd_texture_add;
					}
				}
			}
			if (dmat.toon_index != 255) {
				mat.m_toonmap = toonmaps[dmat.toon_index];
				ub.toonmap_mode = e_mmd_texture_mul;
			}
			mmd_materials.push_back(mat);
		}
	}
}

}  // namespace spu::gs_node::mmd
