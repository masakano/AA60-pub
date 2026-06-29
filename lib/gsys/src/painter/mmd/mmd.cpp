//
// MMD :
//
#include <gsys/canvas/shadowmap.h>
#include <gsys/decorator/instance.h>
#include <gsys/decorator/shadowmap.h>
#include <gsys/painter/mmd.h>

namespace spu::gs_painter {

MMD::MMD(const Attrs &attrs) { MMD::init(attrs); }

void MMD::init(const Attrs &attrs)
{
	// base
	{
		const char *radiance_path = attrs.get("path.radiance", "painter/mmd/radiance.us");
		const char *depth_path = attrs.get("path.depth", "painter/mmd/depth.us");
		const char *sub_radiance_path = attrs.get("path.sub_radiance", "painter/mmd/edge_radiance.us");

		Attrs def_attrs = {
		        {"def_use_animation", 1                },
		        {"def_use_morphing",  1                },
		        {"path.radiance",     radiance_path    },
		        {"path.depth",        depth_path       },
		        {"path.sub_radiance", sub_radiance_path},
		        {"a.a_position",      3                },
		        {"a.a_normal",        3                },
		        {"a.a_texcoord",      2                },
		};

		GsPainter::init(def_attrs + attrs);
		getDecorators().push_back(new gs_decorator::Instance(this, attrs));
		getDecorators().push_back(new gs_decorator::Shadowmap(this, attrs));
	}

	// uniform
	{
		Attrs unif_attrs = {
		        {"ub_mmd_material", &ub_mmd_material},
		        {"ub_light",        &ub_light       },
		        {"u_diffusemap",    &u_diffusemap   },
		        {"u_spheremap",     &u_spheremap    },
		        {"u_toonmap",       &u_toonmap      },
		};
		addUniforms(unif_attrs);
	}
}

void MMD::useSubRadiance(uint32_t id)
{
	auto current = GsCanvas::getCurrent();
	auto &viewport = current->viewport(0);
	ub_mmd_material.screen_size = {viewport.sx, viewport.sy};

	auto &drawcall = getDrawcalls().at(id);
	auto &material = getMMDMaterials().at(id);

	if (material.ub_mmd_material.edge_flag) {
		ub_mmd_material.edge_color = material.ub_mmd_material.edge_color;
		ub_mmd_material.edge_width = material.ub_mmd_material.edge_width;

		drawcall.flags.cull_face = true;
		drawcall.flags.ccw = !drawcall.flags.ccw;
		drawcall.flags.fill_offset = true;
		drawcall.poly_offset = {4.0, 1.0};  // should be parameterized
	}
	else {
		drawcall.ub_material.invisible = true;
	}
}

void MMD::useRadiance(uint32_t id)
{
	if (id == 0) {
		auto *current = GsCanvas::getCurrent();
		auto &worldview = current->worldview(0);

		ub_light = current->ub_light;
		for (auto &source: ub_light.sources) {
			source.position = worldview.ortho3(source.position);
			source.direction = worldview.rot3(source.direction);
		}
	}

	auto &drawcall = getDrawcalls().at(id);
	auto &material = m_materials.at(id);

	ub_mmd_material = material.ub_mmd_material;
	ub_mmd_material.diffuse.a *= drawcall.ub_material.albedo.a;  // for debugDraw()

	u_diffusemap = material.m_diffusemap.id();
	u_spheremap = material.m_spheremap.id();
	u_toonmap = material.m_toonmap.id();
}

void MMD::doUse(uint32_t id)
{
	auto &shader_type = getShaderType();
	if (shader_type == e_sub_radiance) {
		useSubRadiance(id);
	}
	else if (shader_type == e_radiance) {
		useRadiance(id);
	}
	GsPainter::doUse(id);
}
}  // namespace spu::gs_painter
