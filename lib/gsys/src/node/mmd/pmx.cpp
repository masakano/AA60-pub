//
// PMXMorph :
//
#include "pmx.h"
#include <gsys/painter/mmd.h>
#include <gsys/painter/stdout.h>
#include <gsys/util/lambert_to_pbr.h>
#include <smath/color_chart.h>

namespace spu::gs_node::mmd {

namespace {
template<class T0, class T1> void morph(T0 &target, float rate, const T1 &factor, uint32_t type)
{
	auto src_rate = type == PMXMorph::MaterialFactor::e_mul ? 1.0f - rate : 1.0;
	target = T1(target) * src_rate + factor * rate;
}

void morph_each_material(gs_painter::MMDMaterial &mat, PMXMorph::MaterialFactor &factor, float rate)
{
	auto &ub = mat.ub_mmd_material;
	morph(ub.diffuse, rate, factor.diffuse, factor.type);
	morph(ub.ambient, rate, factor.ambient, factor.type);
	morph(ub.specular, rate, factor.specular, factor.type);
	morph(ub.specular_power, rate, factor.specular_power, factor.type);
	morph(ub.edge_color, rate, factor.edge_color, factor.type);
	morph(ub.edge_width, rate, factor.edge_width, factor.type);
	morph(ub.diffusemod.add, rate, factor.diffusemap_factor, factor.type);
	morph(ub.spheremod.add, rate, factor.spheremap_factor, factor.type);
	morph(ub.toonmod.add, rate, factor.toonmap_factor, factor.type);
}
}  // namespace

void PMXMorph::update(MMDActor *actor)
{
	auto rate = m_value;
	if (rate == 0.0) {
		return;
	}
	MMDMorph::update(actor);

	// bone
	{
		for (auto &factor: m_boneFactors) {
			auto *bone = actor->getBones()[factor.index];
			auto delta = lerp(Transformf(), factor.transform, rate);
			bone->rotate(delta.q);
			bone->translate(delta.t);
		}
	}
	// material
	{
		auto *painter = dynamic_cast<gs_painter::MMD *>(actor->getPainter());
		if (painter) {
			auto &materials = painter->getMMDMaterials();
			for (auto &factor: m_materialFactors) {
				if (factor.index == -1) {
					for (auto &mat: materials) {
						morph_each_material(mat, factor, rate);
					}
				}
				else {
					morph_each_material(materials[factor.index], factor, rate);
				}
			}
		}
	}
	// group
	{
		for (auto &factor: m_groupFactors) {
			auto *factor_morph = dynamic_cast<PMXMorph *>(actor->getMorphs()[factor.index]);
			if (!factor_morph->m_groupFactors.empty()) {
				aux_printf(
				        "skip chained group: [%s] -> [%s]\n", name().c_str(),
				        factor_morph->name().c_str());
			}
			else {
				factor_morph->animate(rate * factor.weight);
				factor_morph->update(actor);
			}
		}
	}
}

void PMXActor::replacePainter(GsPainter *painter)
{
	const auto mmd_dir = m_path.parent_path();
	mmd::PMXFile desc(m_path);
	desc.load();

	// Infos
	{
		setName(desc.info.name());
		setCommon(painter, desc, true);
		MMDActor::replacePainter(painter);
	}

	// textures
	{
		for (auto texture: desc.textures) {  // copy
			auto texture_path = mmd_dir / texture;
			m_textures.push_back(loadTexture(texture_path));
		}
	}

	// drawcall (common)
	{
		auto first = 0;
		auto &drawcalls = painter->getDrawcalls();
		drawcalls.clear();

		for (auto &dmat: desc.materials) {
			GsDrawcall drawcall;

			if (dmat.diffusemap_index != -1) {
				drawcall.albedomap = m_textures[dmat.diffusemap_index];
			}

			drawcall.ub_material.albedo = srgb_to_linear(dmat.diffuse);
			drawcall.ub_material.albedo.a = 1.0;
			drawcall.ub_material.min_alpha = 0;

			auto diffuse = srgb_to_linear(dmat.diffuse);
			auto ambient = srgb_to_linear(dmat.ambient);
			auto specular = srgb_to_linear(dmat.specular);

			GsLambertToPBR ltb(diffuse, ambient, specular, dmat.specular_power);

			drawcall.ub_material.albedo = diffuse;
			drawcall.ub_material.metallic = ltb.metallic();
			drawcall.ub_material.roughness = ltb.roughness();
			drawcall.ub_material.ao = ltb.ao();
			drawcall.ub_material.invisible = dmat.flags.invisible;

			drawcall.flags.cull_face = !dmat.flags.both_face;
			drawcall.flags.ccw = true;
			drawcall.flags.depth_test = true;
			drawcall.flags.blend = true;

			drawcall.coms[0].mode = GL_TRIANGLES;
			drawcall.coms[0].first = first;
			drawcall.coms[0].count = dmat.count;
			drawcall.poly_offset = {4.0, 1.0};  // need parameterize

			drawcalls.push_back(drawcall);
			first += dmat.count;
		}
	}
	auto *mmd_painter = dynamic_cast<gs_painter::MMD *>(painter);
	if (mmd_painter) {
		auto &mmd_materials = mmd_painter->getMMDMaterials();
		for (auto &dmat: desc.materials) {
			gs_painter::MMDMaterial mat;
			auto &ub = mat.ub_mmd_material;

			ub.diffuse = srgb_to_linear(dmat.diffuse);
			ub.specular = srgb_to_linear(dmat.specular);
			ub.ambient = srgb_to_linear(dmat.ambient);
			ub.specular_power = dmat.specular_power;
			ub.use_shadowmap = dmat.flags.receive_self_shadow;
			ub.edge_color = srgb_to_linear(dmat.edgeColor);
			ub.edge_width = dmat.edge_size;
			ub.edge_flag = dmat.flags.draw_edge;

			// Texture
			if (dmat.diffusemap_index != -1) {
				mat.m_diffusemap = m_textures[dmat.diffusemap_index];
				ub.diffusemap_mode = e_mmd_texture_mul;
			}

			// ToonTexture
			if (dmat.toonmap_index != -1) {
				switch (dmat.toon_mode) {
				case mmd::PMXMaterial::e_common: {
					std::filesystem::path texture_path = string_printf(
					        "%s/toon%02d.bmp", "painter/mmd", dmat.toonmap_index + 1);
					mat.m_toonmap = loadTexture(texture_path);
					break;
				}
				case mmd::PMXMaterial::e_custom: {
					mat.m_toonmap = m_textures[dmat.toonmap_index];
					break;
				}
				default: assert(0);
				}

				Attrs texture_attrs = {
				        {"wrap_s", GL_CLAMP_TO_EDGE},
				        {"wrap_t", GL_CLAMP_TO_EDGE},
				};
				mat.m_toonmap.set(texture_attrs);
				ub.toonmap_mode = e_mmd_texture_mul;
			}

			// SpTexture
			if (dmat.spheremap_index != -1) {
				ub.spheremap_mode = dmat.sphere_mode;
				mat.m_spheremap = m_textures[dmat.spheremap_index];
				if (dmat.sphere_mode == e_mmd_sub_texture) {
					aux_printf("uunsupported sphere_mode: %d\n", dmat.sphere_mode);
					ub.spheremap_mode = e_mmd_texture_none;
				}
			}
			mmd_materials.push_back(mat);
		}
	}
}

}  // namespace spu::gs_node::mmd
