//
// PBRInspector :
//
#pragma once
#include <gsys/painter/pbr.h>
#include <gsys/node/gui/tweakbar.h>
#include <gsys/shaders/decorator/material/pbr_material_debug.us>
#include <gsys/decorator/material.h>

namespace spu::gs_painter {

class PBRInspector : public gs_node::gui::Tweakbar {
public:
	using base_t = gs_node::gui::Tweakbar;

	PBRInspector(GsPainter *painter) : m_painter(painter)
	{
		base_t::setName(padstr(painter->prettyName(), 24));  // tentative

		auto pbr_painter = dynamic_cast<gs_painter::PBR *>(m_painter);
		if (pbr_painter && pbr_painter->def_pbr_debug) {
			const std::vector<gs_node::gui::Menu::Item> c_debug_items = {
			        {"none",            e_pbr_debug_none           },
			        {"single",          e_pbr_debug_single         },
			        {"ibl",             e_pbr_debug_ibl            },
			        {"albedo",          e_pbr_debug_albedo         },
			        {"metallic",        e_pbr_debug_metallic       },
			        {"roughness",       e_pbr_debug_roughness      },
			        {"ao",              e_pbr_debug_ao             },
			        {"normal",          e_pbr_debug_normal         },
			        {"single_ndf",      e_pbr_debug_single_ndf     },
			        {"single_g",        e_pbr_debug_single_g       },
			        {"single_f",        e_pbr_debug_single_f       },
			        {"single_specular", e_pbr_debug_single_specular},
			        {"ibl_f",           e_pbr_debug_ibl_f          },
			        {"ibl_irrad",       e_pbr_debug_ibl_irrad      },
			        {"ibl_brdf",        e_pbr_debug_ibl_brdf       },
			        {"ibl_reflect",     e_pbr_debug_ibl_reflect    },
			        {"light_radiance",  e_pbr_debug_light_radiance },
			};
			base_t::addMenus("debug mode", {c_debug_items}, {&m_debugMode});
		}

		base_t::addStdButton("override materials", &m_isOverride.material);
		base_t::addStdButton("override textures", &m_isOverride.texture);
		base_t::addStdButton("override lightmap", &m_isOverride.lightmap);

		base_t::addStdSlider("roughness", 0.01, 0.99, &ub_material.roughness);
		base_t::addStdSlider("metallic ", 0.01, 0.99, &ub_material.metallic);
		base_t::addStdSlider("ao ", 0.01, 0.99, &ub_material.ao);
		base_t::addStdSlider("gamma", 0.0, 4.0, &ub_material.gamma);
		base_t::addStdColorSlider("albedo", &m_albedo);
		base_t::bake();

		changeMaterialState();
		m_isConstructorBarrier = false;
	}
	void update() override
	{
		if (m_isConstructorBarrier) {
			return;
		}

		base_t::update();
		auto *default_drawcall = GsDrawcall::getDefault();
		if (*default_drawcall != m_defaultDrawcall || base_t::isFocus()) {
			m_defaultDrawcall = *default_drawcall;
			changeMaterialState();

			if (isReloadTiming()) {
				m_orgDrawcalls = m_painter->getDrawcalls();
				ub_material = m_painter->getDrawcalls().at(0).ub_material;
				m_isReady = true;
				doReload();
			}
			if (isRestoreTiming()) {
				m_painter->getDrawcalls() = m_orgDrawcalls;
			}
			if (m_isReady) {
				updatePBRDrawcall();
				doUpdate();
			}
			m_prevIsOverride = m_isOverride;
		}
	}

protected:
	GsPainter *m_painter = nullptr;

	UB_MATERIAL ub_material;
	Vec4f m_albedo = eone<Vec4f>();
	int32_t m_debugMode = 0;

	virtual void doReload() {}
	virtual void doUpdate() {}

private:
	struct IsOverride {
		int32_t material = false;
		int32_t texture = false;
		int32_t lightmap = false;
		bool clean() const { return material == false && texture == false && lightmap == false; }
	};
	IsOverride m_isOverride;
	IsOverride m_prevIsOverride;
	bool m_isConstructorBarrier = true;
	bool m_isReady = false;

	GsDrawcall m_defaultDrawcall;
	std::vector<GsDrawcall> m_orgDrawcalls;

	void updatePBRDrawcall()
	{
		auto &drawcalls = m_painter->getDrawcalls();
		const auto *default_drawcall = GsDrawcall::getDefault();
		for (auto &drawcall: drawcalls) {
			drawcall.ub_material.debug_mode = m_debugMode;

			if (m_isOverride.material) {
				drawcall.flags.blend = true;
				drawcall.ub_material.albedo = m_albedo;
				drawcall.ub_material.metallic = ub_material.metallic;
				drawcall.ub_material.roughness = ub_material.roughness;
				drawcall.ub_material.ao = ub_material.ao;
				drawcall.ub_material.gamma = ub_material.gamma;
			}

			if (m_isOverride.texture) {
				drawcall.albedomap = default_drawcall->albedomap;
				drawcall.armmap = default_drawcall->armmap;
				drawcall.normalmap = default_drawcall->normalmap;
				drawcall.heightmap = default_drawcall->heightmap;
			}
			if (m_isOverride.lightmap) {
				drawcall.lightmap = default_drawcall->lightmap;
			}
		}
	}

	void changeMaterialState()
	{
		auto state = m_isOverride.material ? e_active : e_disabled;
		std::vector<const void *> ptrs = {
		        &ub_material.roughness,
		        &ub_material.metallic,
		        &ub_material.ao,
		        &ub_material.gamma,
		};
		base_t::changeState(ptrs, state);
	}

	bool isReloadTiming() const
	{
		return m_orgDrawcalls.size() < m_painter->getDrawcalls().size()
		    || (!m_isOverride.clean() && m_prevIsOverride.clean());
	}

	bool isRestoreTiming() const { return m_isOverride.clean() && !m_prevIsOverride.clean(); }
};
}  // namespace spu::gs_painter
