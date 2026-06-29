//
// GsDemoPage :
//
#include <gsys/canvas/gs_demo_page.h>
#include <gsys/painter/plane.h>
#include <gsys/painter/water_plane.h>
//#include <gsys/painter/terrain.h>
#include <gsys/node/plane.h>

namespace spu {

GsNode *gs_canvas::GsDemoPage::createFloor(const Attrs &attrs, const std::vector<GsNode *> &reflect_nodes)
{
	const auto *candidate = "?plane:water:plate:grass:terrain:pedestal";
	auto name = hash32_t(attrs.get("name", candidate));
	auto painter_attrs = attrs.select("painter.");
	auto composite_attrs = attrs.unselect({"painter."});

	GsNode *node = nullptr;
	if (name == "plane") {
		auto painter = createPainter("plane", painter_attrs);
		node = new gs_node::Plane(composite_attrs);
		node->replacePainter(painter);
	}
	else if (name == "water") {
		auto painter = createPainter("water", painter_attrs);
		node = new gs_node::Plane(composite_attrs);
		node->replacePainter(painter);
	}
	else if (name == "plate") {
		auto painter = createPainter("pbr", painter_attrs);
		Attrs pre_attrs = {
		        {"shape",                                "plate"  },
		        {"painter.drawcall.texture.albedo.path", "checker"},
		        {"painter.drawcall.ub_material.ao",      0.1      },
		};
		composite_attrs.prepend(pre_attrs);
		node = createNode(composite_attrs, painter);
	}
	else if (name == "grass") {

		auto painter = createPainter("grass_fur", painter_attrs);

		const auto c_ndiv = 128;
		auto mesh_grid = Vec4i(c_ndiv, c_ndiv, 1, 1);
		Attrs post_attrs = {
		        {"shape", "plate"},
			{"mesh_grid", mesh_grid   },
		};
		composite_attrs.append(post_attrs);
		node = createNode(composite_attrs, painter);
		node->setProperty("lod", 1);
	}
	else if (name == "terrain") {
		auto painter = createPainter("terrain", painter_attrs);
		const auto c_ndiv = 64;
		auto mesh_grid = Vec4i(c_ndiv, c_ndiv, 1, 1);
		auto mapnode = Mat4f().scale(2.0).rot("x", pi() / 2);
		auto maptexc = Mat4f().scale({0.5, 0.5, 1.0}).trans({0.5, 0.5, 0.0});

		Attrs pre_attrs = {
		        {"mesh_grid", mesh_grid},
		        {"mapnode",   &mapnode },
		        {"maptexc",   &maptexc },
		};
		Attrs post_attrs = {
		        {"name",  "terrain"},
		        {"shape", "plate"  },
		};
		composite_attrs.prepend(pre_attrs).append(post_attrs);
		node = createNode(composite_attrs, painter);
	}

	else if (name == "pedestal") {
		auto painter = createPainter("pbr", painter_attrs);
		Attrs pre_attrs = {
		        {"path",                         "assets/models/poly/a09.obj"},
		        {"gen_normal",                   1                           },
		        {"gen_texcoord",                 1                           },
		        {"gen_tangent",                  1                           },
		        {"all_flat",                     1                           },
		        {"texcoord_scale",               vec4f_t(3.0)                },
		        {"drawcall.texture.albedo.path", "assets/textures/skala.jpg" },
		};
		composite_attrs.prepend(pre_attrs);
		node = createNode(composite_attrs, painter);
		auto &transform = node->getASubstance();
		transform = Mat4f().scale(0.5).rot("x", pi() / 2);
		transform.c[3].y = -0.125;  // need theory
	}
	else {
		aux_error(true, "unknown floor [%s]\n", name);
	}

	// post process
	auto *painter = node->getPainter();
	painter->getShaders().at("depth").dispose();  // no blocker
	painter->setRelatedNodes(reflect_nodes);
	return node;
}
}  // namespace spu
