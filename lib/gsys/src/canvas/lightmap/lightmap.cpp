//
// Lightmap :
//
#include <gsys/canvas/lightmap.h>
#include <gsys/canvas/gauss.h>
#include <gsys/drawcall.h>

namespace spu::gs_canvas {

void Lightmap::init(const Attrs &attrs)
{
	Attrs def_attrs = {
	        {"path", "canvas/lightmap/lightmap.us"},
	};
	GsCanvas::init(def_attrs + attrs);

	Attrs unif_attrs = {
	        {"ub_lightmap",             &ub_lightmap},
	        {"u_irradmap_sphere",       &u_irradmap },
	        {"u_irradmap_sphere_array", &u_irradmap },
	        {"u_lightmap_sphere",       &u_lightmap },
	        {"u_lightmap_cube",         &u_lightmap },
	        {"u_lightmap_sphere_array", &u_lightmap },
	        {"u_lightmap_sphere_array", &u_lightmap },
	        {"u_lightmap_cube_array",   &u_lightmap },
	};
	getShader().addUniforms(unif_attrs);

	auto &renderstate = getRenderstate();
	renderstate.flags.depth_test = true;
	renderstate.flags.blend = false;
	renderstate.depth_func = GL_LEQUAL;
}

void Lightmap::update()
{
	irradiance()->update();
	if (lastUpdateCount() != irradiance()->lastUpdateCount()) {
		GsCanvas::update();

		ub_light.ambient = irradiance()->ub_light.ambient;
		auto &dst = ub_light.sources[0];
		auto &src = irradiance()->ub_light.sources[0];
		auto lightmapworld = m_worldlightmap.unitary_inverse();

		dst.position = lightmapworld.ortho3(src.position);
		dst.direction = lightmapworld.rot3(src.direction);
		dst.type = src.type;
		dst.radius = src.radius;
		dst.decay = src.decay;
		dst.exponent = src.exponent;

		auto *drawcall = GsDrawcall::getDefault();
		drawcall->lightmap = irradiance()->lightmap();
		drawcall->irradmap = irradiance()->irradmap();

		auto canvas = getCurrent();
		canvas->ub_light.ambient = ub_light.ambient;
		canvas->ub_light.sources[0] = ub_light.sources[0];
	}
}

void Lightmap::render()
{
	if (lastUpdateCount() == 0) {
		aux_message(0, "force lightmap update...\n");
		update();  // safety
	}
	auto *current = getCurrent();
	if (current) {
		const auto *default_drawcall = GsDrawcall::getDefault();

		u_lightmap = default_drawcall->lightmap.id();
		u_irradmap = default_drawcall->irradmap.id();

		auto lightmap_target = u_lightmap >> 16;
		auto irradmap_target = u_irradmap >> 16;
		assert(irradmap_target == GL_TEXTURE_2D || irradmap_target == GL_TEXTURE_2D_ARRAY);

		ub_lightmap.lightmap_target = lightmap_target;
		ub_lightmap.irradmap_target = irradmap_target;
		ub_lightmap.screenview = current->viewscreen().inverse();
		ub_lightmap.viewlightmap = m_worldlightmap * current->viewworld(0);
	}
	GsCanvas::render();
}

void Lightmap::set(const Attrs &attrs)
{
	// reject legacy
	attrs.peek("irradiance_powers", "use 'irradiance.powers' instead");
	attrs.peek("irradiance_width", "use 'irradiance.width' instead");

	irradiance()->set(attrs.select("irradiance."));
	m_worldlightmap = *attrs.get("worldlightmap", &m_worldlightmap);
	GsCanvas::set(attrs);
}

void Lightmap::startInspector() { irradiance()->startInspector(); }

}  // namespace spu::gs_canvas
