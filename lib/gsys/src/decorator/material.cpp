//
// Material :
//
#include <gsys/decorator/material.h>
#include <gsys/canvas.h>

namespace spu::gs_decorator {

Material::Material(GsPainter *painter, const Attrs &attrs) : GsDecorator(painter, attrs)
{
	if (painter) {
		painter->addUniforms(uniforms());
	}
}

Attrs Material::uniforms() const
{
	Attrs unif_attrs = {
	        {"ub_light",                &ub_light     },
	        {"ub_material",             &ub_material  },
	        {"ub_lightmap",             &ub_lightmap  },
	        {"u_albedomap",             &u_albedomap  },
	        {"u_specularmap",           &u_specularmap},
	        {"u_emissionmap",           &u_emissionmap},
	        {"u_armmap",                &u_armmap     },
	        {"u_normalmap",             &u_normalmap  },
	        {"u_heightmap",             &u_heightmap  },
	        {"u_lightmap_cube",         &u_lightmap   },
	        {"u_lightmap_sphere",       &u_lightmap   },
	        {"u_lightmap_cube_array",   &u_lightmap   },
	        {"u_lightmap_sphere_array", &u_lightmap   },
	        {"u_irradmap_sphere",       &u_irradmap   },
	        {"u_irradmap_sphere_array", &u_irradmap   },
	        {"u_brdfmap",               &u_brdfmap    },
	};
	return unif_attrs;
}

void Material::set(const Attrs &attrs)
{
	attrs.peek("u_worldlightmap", "use 'worldlightmap' instead");
	m_worldlightmap = *attrs.get("worldlightmap", &m_worldlightmap);
}

void Material::doUse(uint32_t id)
{
	if (m_painter) {
		const auto *current = GsCanvas::getCurrent();
		auto &worldview = current->getWorldviews().front();

		if (id == 0) {
			ub_light = current->ub_light;
			for (auto &source: ub_light.sources) {
				source.position = worldview.ortho3(source.position);
				source.direction = worldview.rot3(source.direction);
			}
		}
		auto &drawcall = m_painter->getDrawcalls().at(id);

		ub_material = drawcall.ub_material;
		ub_material.point_coord = drawcall.flags.point_sprite && drawcall.coms[0].mode == GL_POINTS;

		u_albedomap = drawcall.albedomap.id();
		u_specularmap = drawcall.specularmap.id();
		u_emissionmap = drawcall.emissionmap.id();
		u_armmap = drawcall.armmap.id();
		u_normalmap = drawcall.normalmap.id();
		u_heightmap = drawcall.heightmap.id();

		const auto *default_drawcall = GsDrawcall::getDefault();

		u_lightmap = default_drawcall->lightmap.id();
		u_irradmap = default_drawcall->irradmap.id();
		u_brdfmap = default_drawcall->brdfmap.id();

		if (!GsObject::isDefaultTexture(drawcall.lightmap.id())) u_lightmap = drawcall.lightmap.id();
		if (!GsObject::isDefaultTexture(drawcall.irradmap.id())) u_irradmap = drawcall.irradmap.id();
		if (!GsObject::isDefaultTexture(drawcall.brdfmap.id())) u_brdfmap = drawcall.brdfmap.id();
		if (GsObject::isDefaultTexture(u_irradmap)) u_irradmap = u_lightmap;

		ub_lightmap.viewlightmap = m_worldlightmap * worldview.inverse();
		ub_lightmap.lightmap_target = (u_lightmap >> 16) & 0xffff;
		ub_lightmap.irradmap_target = (u_irradmap >> 16) & 0xffff;
	}
	GsDecorator::doUse(id);
}
}  // namespace spu::gs_decorator
