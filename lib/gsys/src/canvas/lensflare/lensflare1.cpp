//
// Lensflare1 :
//
#include "lensflare1_inspector.h"
#include <gsys/canvas/lensflare1.h>

namespace spu::gs_canvas {

void Lensflare1::init(const Attrs &attrs)
{
	auto *current = getCurrent();
	auto viewport0 = attrs.get("viewport0", current->viewport(0));

	auto full_viewport = viewport0;
	auto half_viewport = viewport0;
	half_viewport.sx /= 2;
	half_viewport.sy /= 2;

	// gauss
	{
		Attrs attrs = {
		        {"viewport0",          half_viewport   },
                        {"color0.target",      GL_TEXTURE_2D   },
		        {"color0.iformat",     GL_RGBA32F      },
                        {"color0.min_filter",  GL_LINEAR       },
		        {"color0.mag_filter",  GL_LINEAR       },
                        {"color0.wrap_s",      GL_CLAMP_TO_EDGE},
		        {"color0.wrap_t",      GL_CLAMP_TO_EDGE},
                        {"color0.auto_mipmap", 0               },
		};
		m_gaussF.init(attrs);
	}

	// gauss
	{
		Attrs attrs = {
		        {"viewport0",          full_viewport   },
                        {"color0.target",      GL_TEXTURE_2D   },
		        {"color0.iformat",     GL_RGBA32F      },
                        {"color0.min_filter",  GL_LINEAR       },
		        {"color0.mag_filter",  GL_LINEAR       },
                        {"color0.wrap_s",      GL_CLAMP_TO_EDGE},
		        {"color0.wrap_t",      GL_CLAMP_TO_EDGE},
                        {"color0.auto_mipmap", 0               },
		};
		m_gaussH.init(attrs);
	}

	// halo
	{
		Attrs canvas_attrs{
		        {"path",               "canvas/lensflare/halo.us"},
                        {"viewport0",          full_viewport             },
		        {"color0.target",      GL_TEXTURE_2D             },
                        {"color0.iformat",     GL_RGBA32F                },
		        {"color0.mag_filter",  GL_LINEAR                 }, // necessary!
		        {"color0.min_filter",  GL_LINEAR                 },
                        {"color0.auto_mipmap", 0                         },
		        {"color0.wrap_s",      GL_CLAMP_TO_EDGE          },
                        {"color0.wrap_t",      GL_CLAMP_TO_EDGE          },
		};

		m_halo.init(canvas_attrs + attrs);

		Attrs unif_attrs = {
		        {"u_dispersal",    &u_dispersal   },
                        {"u_halo_width",   &u_halo_width  },
		        {"u_distortion",   &u_distortion  },
                        {"u_sun_pos_frag", &u_sun_pos_frag},
		        {"u_is_blur",      &u_is_blur     },
                        {"u_dirt_texture", &u_dirt_texture},
		};
		m_halo.getShader().addUniforms(unif_attrs);

		const char *path = "canvas/lensflare/dirt_lowc.jpg";
		Attrs tex_attrs = {
		        {"srgb", 1}
                };
		u_dirt_texture = spu_inventory_new("texture", path, tex_attrs);
	}
	{
		const Attrs def_attrs = {
		        {"def_blend_max", 1},
		};
		Tonemap::init(def_attrs + attrs);
	}
}

void Lensflare1::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new Lensflare1Inspector(this);
	}
}

void Lensflare1::render()
{
	auto *current = getCurrent();
	auto &ls = current->ub_light.sources[0];

	if (ls.type == e_ub_light_parallel) {
		auto lightdir_view = current->worldview(0) * Vec3f(ls.direction);  // direction
		auto viewtexc = Mat4f::screentexc() * current->viewscreen();
		u_sun_pos_frag = viewtexc.pers3(lightdir_view);
	}
	else {
		auto worldtexc = Mat4f::screentexc() * current->worldscreen(0);
		u_sun_pos_frag = worldtexc.pers3(Vec3f(ls.position));
	}

	// check
	u_is_blur = 0.0f < u_sun_pos_frag.x && u_sun_pos_frag.x < 1.0f && 0.0f < u_sun_pos_frag.y
	         && u_sun_pos_frag.y < 1.0f;

	// low blur
	{
		m_gaussH.u_color0 = u_color0;
		m_gaussH.u_variance = m_varianceH;
		m_gaussH.ub_connect.sources[0].bias = m_bias;
		m_gaussH.takeover();
		m_gaussH.begin();
		m_gaussH.render();
		m_gaussH.end();
	}

	// high blur
	{
		m_gaussF.u_color0 = u_color0;
		m_gaussF.u_variance = m_varianceF;
		m_gaussF.ub_connect.sources[0].bias = m_bias;
		m_gaussF.takeover();
		m_gaussF.begin();
		m_gaussF.render();
		m_gaussF.end();
	}

	// flare halo
	{
		m_halo.u_color0 = m_gaussH.getBuffer("color0").id();
		m_halo.u_color1 = m_gaussF.getBuffer("color0").id();

		m_halo.begin();
		m_halo.render();
		m_halo.end();
	}

	// blend
	{
		auto u_color1_save = u_color1;
		u_color1 = m_halo.getBuffer("color0").id();
		Tonemap::render();
		u_color1 = u_color1_save;
	}
}

void Lensflare1::set(const Attrs &attrs)
{
	attrs.peek("gain", "deprecated");
	attrs.apply("varianceH", m_varianceH);
	attrs.apply("varianceF", m_varianceF);
	attrs.apply("bias", m_bias);

	m_halo.set(attrs);
	Tonemap::set(attrs);
}
}  // namespace spu::gs_canvas
