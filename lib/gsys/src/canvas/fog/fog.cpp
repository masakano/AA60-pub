//
// Fog :
//
#include "fog_inspector.h"
#include <gsys/canvas/copy.h>
#include <gsys/canvas/gauss.h>

namespace spu::gs_canvas {

void Fog::createFogCanvas()
{
	auto *current = getCurrent();

	auto half_viewport = current->viewport(0);
#if 0
	half_viewport.sx /= 2;
	half_viewport.sy /= 2;
#endif
	const char *def_shadowmap_probability_path = "<decorator/shadowmap/shadowmap_probability_simple.us>";

	Attrs init_attrs = {
	        {"viewport0",                      half_viewport                 },
	        {"color0.iformat",                 GL_RGBA32F                    },
	        {"color0.target",                  GL_TEXTURE_2D                 },
	        {"color0.min_filter",              GL_LINEAR_MIPMAP_LINEAR       },
	        {"color0.mag_filter",              GL_LINEAR                     },
	        {"color0.wrap_s",                  GL_CLAMP                      },
	        {"color0.wrap_t",                  GL_CLAMP                      },
	        {"color0.max_level",               4                             },
	        {"color0.auto_mipmap",             1                             },
	        {"path",	                   "canvas/fog/fog.us"           },
	        {"reloadable",                     1                             },
	        {"def_shadowmap_probability_path", def_shadowmap_probability_path},
	};
	if (m_shadowmap) {
		init_attrs.prepend(m_shadowmap->getShaderAttrs());
	}
	m_fogCanvas.init(init_attrs);

	Attrs unif_attrs = {
	        {"ub_fbm_fog",   &ub_fbm_fog  },
                {"u_worldview",  &u_worldview },
	        {"u_viewworld",  &u_viewworld },
                {"u_screenview", &u_screenview},
	        {"u_esec",       &u_esec      },
                {"u_near",       &u_near      },
                {"u_far",        &u_far       },
	        {"u_step",       &u_step      },
                {"ub_shadowmap", &ub_shadowmap},
	        {"u_shadowmap",  &u_shadowmap },
                {"u_lightmap",   &u_lightmap  },
	        {"u_irradmap",   &u_irradmap  },
	};
	m_fogCanvas.getShader().addUniforms(unif_attrs);
}

void Fog::init(const Attrs &attrs)
{
	m_shadowmap = attrs.get<gs_canvas::Shadowmap *>("shadowmap", nullptr);
	if (!m_shadowmap) {
		aux_message(0, "no shadowmap specified (disabled)\n");
	}
	createFogCanvas();

	auto viewport0 = Recti(getCurrent()->viewport(0));
	auto viewport1 = Recti(m_fogCanvas.viewport(0));

	Attrs canvas_attrs = {
	        {"path",        "canvas/fog/fog_blend.us"},
                {"def_width0",  viewport0.sx             },
	        {"def_height0", viewport0.sy             },
                {"def_width1",  viewport1.sx             },
	        {"def_height1", viewport1.sy             },
	};
	GsCanvas::init(canvas_attrs);
}

void Fog::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new FogInspector(this);
	}
}

void Fog::render()
{
	auto *current = getCurrent();

	u_esec += getSeconds().delta();
	u_worldview = current->worldview(0);
	u_viewworld = current->viewworld(0);
	u_screenview = current->viewscreen().inverse();

	auto *default_drawcall = GsDrawcall::getDefault();
	u_lightmap = default_drawcall->lightmap.id();
	u_irradmap = default_drawcall->irradmap.id();

	if (m_shadowmap) {
		ub_shadowmap = m_shadowmap->makeUniform();
		u_shadowmap = m_shadowmap->getBuffer("depth").id();
	}
	else {
		ub_shadowmap.enable = false;
	}

	m_fogCanvas.u_color0 = u_color0;  // redirect
	m_fogCanvas.u_depth = u_depth;    // redirect
	m_fogCanvas.takeover();
	m_fogCanvas.begin();
	m_fogCanvas.render();
	m_fogCanvas.end();

	u_color1 = m_fogCanvas.getBuffer("color0").id();  // fog #0
	GsCanvas::render();
}
}  // namespace spu::gs_canvas
