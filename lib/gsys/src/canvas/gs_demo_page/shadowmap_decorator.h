//
// ShadowmapDecorator :
//
#pragma once

#include <gsys/canvas/gs_demo_page.h>
#include <gsys/canvas/shadowmap.h>

namespace spu::gs_canvas::gs_demo_page {

class ShadowmapDecorator : public GsPage::IDecorator {
public:
	ShadowmapDecorator(GsDemoPage &demo_page, const Attrs &attrs) : m_demoPage(demo_page)
	{
		constexpr const char *shadowmap_name_candidates
		        = "?shadowmap:cascade_shadowmap:penumbra_shadowmap:mv_cascade_shadowmap:none";
		constexpr const char *shadowmap_mode_candidates = "?vsm:none:simple:smooth:pcf";

		auto shadowmap_blocker_pattern = attrs.get("shadowmap.blocker_pattern", 32);
		auto shadowmap_receiver_pattern = attrs.get("shadowmap.receiver_pattern", 32);
		auto shadowmap_size = attrs.get("shadowmap.size", 1024);
		auto shadowmap_name = attrs.get("shadowmap.name", shadowmap_name_candidates);
		auto shadowmap_mode = attrs.get("shadowmap.mode", shadowmap_mode_candidates);
		auto shadowmap_poly_offset_factor = attrs.get("shadowmap.poly_offset.factor", 0.09f);
		auto shadowmap_poly_offset_units = attrs.get("shadowmap.poly_offset.units", 0.0f);

		auto is_shadowmap_inspector = attrs.get("shadowmap.inspector", 1);

		auto viewport = Rectf(0, 0, shadowmap_size, shadowmap_size);
		Attrs init_attrs = {
		        {"mode",      shadowmap_mode},
		        {"viewport0", viewport      },
		};

		Attrs set_attrs = {
		        {"blocker_pattern",    shadowmap_blocker_pattern   },
		        {"receiver_pattern",   shadowmap_receiver_pattern  },
		        {"poly_offset.factor", shadowmap_poly_offset_factor},
		        {"poly_offset.units",  shadowmap_poly_offset_units },
		};

		Shadowmap *shadowmap = nullptr;

		if (strcmp(shadowmap_name, "none") == 0) {
			shadowmap = nullptr;
		}
		else if (strcmp(shadowmap_name, "shadowmap") == 0) {
			shadowmap = new Shadowmap(init_attrs);
		}
		else if (strcmp(shadowmap_name, "cascade_shadowmap") == 0) {
			shadowmap = new CascadeShadowmap(init_attrs);
		}
		else if (strcmp(shadowmap_name, "penumbra_shadowmap") == 0) {
			shadowmap = new PenumbraShadowmap(init_attrs);
		}
		else if (strcmp(shadowmap_name, "mv_cascade_shadowmap") == 0) {
			Attrs aux_attrs = {
			        {"def_use_multi_viewport", 1},
			};
			shadowmap = new CascadeShadowmap(init_attrs + aux_attrs);
		}
		else {
			assert(0);
		}

		if (shadowmap) {
			if (is_shadowmap_inspector) {
				shadowmap->startInspector();
			}
			shadowmap->set(set_attrs);
		}
		m_demoPage.replaceShadowmap(shadowmap);
	}

	void begin() override {}
	void end() override {}
	void set(const Attrs &) override {}

private:
	GsDemoPage &m_demoPage;
};
}  // namespace spu::gs_canvas::gs_demo_page
