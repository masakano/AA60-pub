//
// GsDemoPage :
//
#include <gsys/canvas/gs_demo_page.h>
#include <gsys/painter/sketch.h>
#include <gsys/painter/pbr.h>
#include <gsys/painter/displaced_pbr.h>
#include <gsys/painter/fur_pbr.h>
#include <gsys/painter/toon.h>
#include <gsys/painter/pointset.h>
#include <gsys/painter/plane.h>
#include <gsys/painter/water_plane.h>

namespace spu {

GsPainter *gs_canvas::GsDemoPage::createGrassFurPainter(Attrs &composite_attrs)
{
#if 0
	Attrs def_init_attrs = {
	        {"patch_vertices", 4},
	};
	composite_attrs.prepend(def_init_attrs);

	
	Attrs def_set_attrs = {
	        {"set.shader.u_tess_min_len", 0.002},
	};
	composite_attrs.prepend(def_set_attrs);
#endif
	Attrs def_drawcall_attrs = {
	        {"drawcall.ub_material.metallic",  0.1                         },
	        {"drawcall.ub_material.roughness", 0.9                         },
	        {"drawcall.texture.albedo.path",   "assets/textures/grass0.jpg"},
	};
	composite_attrs.prepend(def_drawcall_attrs);
	auto painter_attrs = composite_attrs.unselect({"set.", "drawcall.", "prop."});
	return new gs_painter::FurPBR(painter_attrs);
}


GsPainter *gs_canvas::GsDemoPage::createTerrainPainter(Attrs &composite_attrs)
{
#if 0
	return nullptr;
#else
	/*
	Attrs def_init_attrs = {
	        {"patch_vertices", 4},
	};
	composite_attrs.prepend(def_init_attrs);
	*/

	const char *albedo_path = "assets/textures/rugged_terrain/diffuse.exr";
	const char *height_path = "assets/textures/rugged_terrain/height.exr";

	Attrs def_drawcall_attrs = {
	        {"drawcall.texture.albedo.path",       albedo_path},
                {"drawcall.texture.height.path",       height_path},
	        {"drawcall.texture.height.gray_scale", 0          },
                {"drawcall.texture.height.iformat",    GL_R32F    },
	        {"drawcall.ub_material.height_scale",  1.0        },
                {"drawcall.ub_material.roughness",     1.0        },
	        {"drawcall.ub_material.metallic",      0.0        },
                {"drawcall.ub_material.ao",            0.5        },
	        {"drawcall.ub_material.gamma",         2.0        },
	};
	composite_attrs.prepend(def_drawcall_attrs);
	auto painter_attrs = composite_attrs.unselect({"set.", "drawcall.", "prop."});
	return new gs_painter::DisplacedPBR(painter_attrs);
#endif
}

GsPainter *gs_canvas::GsDemoPage::createWaterPainter(Attrs &composite_attrs)
{
	Attrs pre_drawcall_attrs = {
	        {"drawcall.texture.albedo.path",   "assets/textures/water1.jpg"},
	        {"drawcall.ub_material.roughness", 0.3                         },
	        {"drawcall.ub_material.metallic",  0.1                         },
	};
	composite_attrs.prepend(pre_drawcall_attrs);
	auto painter_attrs = composite_attrs.unselect({"set.", "drawcall.", "prop."});
	return new gs_painter::WaterPlane(painter_attrs);
}

GsPainter *gs_canvas::GsDemoPage::createPlanePainter(Attrs &composite_attrs)
{
	Attrs def_drawcall_attrs = {
	        {"drawcall.texture.complete_path",
	         "assets/textures/old-plank-flooring3_Unreal-Engine/old-plank-flooring3_"},
	};
	composite_attrs.prepend(def_drawcall_attrs);
	auto painter_attrs = composite_attrs.unselect({"set.", "drawcall.", "prop."});
	return new gs_painter::Plane(painter_attrs);
}

GsPainter *gs_canvas::GsDemoPage::createPainter(const char *def_name, const Attrs &attrs)
{
	auto composite_attrs = attrs;
	auto name = hash32_t(composite_attrs.pick("name", def_name));

	Attrs def_attrs = {
	        {"shadowmap", getShadowmap()},
	};
	composite_attrs.prepend(def_attrs);
	auto painter_attrs = composite_attrs.unselect({"set.", "drawcall.", "prop."});

	GsPainter *painter = nullptr;
	if (name == "pbr") {
		painter = new gs_painter::PBR(painter_attrs);
	}
	else if (name == "displaced_pbr") {
		painter = new gs_painter::DisplacedPBR(painter_attrs);
	}
	else if (name == "pointset") {
		painter = new gs_painter::Pointset(painter_attrs);
	}
#if 0
	else if (name == "tess") {
		painter = new gs_painter::Tess(painter_attrs);
	}
#endif
	else if (name == "fur") {
		painter = new gs_painter::FurPBR(painter_attrs);
	}
	else if (name == "toon") {
		painter = new gs_painter::Toon(painter_attrs);
	}
	else if (name == "sketch") {
		painter = new gs_painter::Sketch(painter_attrs);
	}
	else if (name == "grass_fur") {
		painter = createGrassFurPainter(composite_attrs);
	}
	else if (name == "terrain") {
		painter = createTerrainPainter(composite_attrs);
	}
	else if (name == "plane") {
		painter = createPlanePainter(composite_attrs);
	}
	else if (name == "water") {
		painter = createWaterPainter(composite_attrs);
	}
	else {
		aux_error(
		        true, "unknown painter [%s]. candidate: pbr:displaced_pbr:pointset:fur:toon:sketch\n",
		        name.c_str());
	}
	painter->set(composite_attrs.select("set."));

	auto prop_attrs = composite_attrs.select("prop.");
	for (auto &key: {"render", "debug_render", "lazy", "flip"}) {
		auto value = prop_attrs.getf<int32_t>(key);
		if (value.hit) {
			painter->setProperty(key, value.value);
		}
	}

	auto drawcall_attrs = composite_attrs.select("drawcall.");
	for (auto &drawcall: painter->getDrawcalls()) {
		drawcall.set(drawcall_attrs);
	}

	return painter;
}

}  // namespace spu
