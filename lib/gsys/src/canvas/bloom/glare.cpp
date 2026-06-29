//
// Glare :
//
#include "glare_inspector.h"
#include <gsys/canvas/gauss.h>

namespace spu::gs_canvas {

void Glare::init(const Attrs &attrs)
{
	// auto c = getCurrent()->getComposition();  // copy
	auto c = Composition(*getCurrent());  // copy
	c.getViewports().at(0) = attrs.get("viewport0", c.viewport(0));

	Vec4f half_viewport = c.viewport(0);
	Vec4f quad_viewport = c.viewport(0);

	half_viewport.sx /= 2;
	half_viewport.sy /= 2;

	quad_viewport.sx /= 4;
	quad_viewport.sy /= 4;

	// parent
	{
		const Attrs def_attrs = {
		        {"def_blend_max", 1},
		};
		Tonemap::init(def_attrs + attrs);
	}

	// half gauss
	{
		// auto path = "canvas/bloom/first_gauss.us";
		Attrs attrs = {
		        {"viewport0",          half_viewport   },
		        //{"path",               path            },
		        {"color0.target",      GL_TEXTURE_2D   },
		        {"color0.iformat",     GL_RGBA32F      },
		        {"color0.min_filter",  GL_LINEAR       },
		        {"color0.mag_filter",  GL_LINEAR       },
		        {"color0.wrap_s",      GL_CLAMP_TO_EDGE},
		        {"color0.wrap_t",      GL_CLAMP_TO_EDGE},
		        {"color0.auto_mipmap", 0               },
		};
		m_gauss2dH.init(attrs);
	}

	// quad gauss
	{
		Attrs attrs = {
		        {"viewport0",          quad_viewport   },
                        {"color0.target",      GL_TEXTURE_2D   },
		        {"color0.iformat",     GL_RGBA32F      },
                        {"color0.min_filter",  GL_LINEAR       },
		        {"color0.mag_filter",  GL_LINEAR       },
                        {"color0.wrap_s",      GL_CLAMP_TO_EDGE},
		        {"color0.wrap_t",      GL_CLAMP_TO_EDGE},
                        {"color0.auto_mipmap", 0               },
		};
		m_gauss2dQ.init(attrs);
		m_gauss2dQ.getRenderstate().setBlendType("constant_alpha");
		m_gauss2dQ.getRenderstate().blend_color = {0.5, 0.5, 0.5, 0.5};
	}
}

void Glare::set(const Attrs &attrs)
{
	attrs.peek("u_variance", "use 'variance' instead");
	attrs.peek("u_footstep", "use 'footstep' instead");
	attrs.peek("u_luminance_threshold", "use 'bias' instead");
	attrs.peek("luminance_bias", "use 'bias' instead");

	attrs.apply("variance", m_variance);
	attrs.apply("footstep", m_footstep);
	attrs.apply("gain", m_gain);
	attrs.apply("bias", m_bias);

	Tonemap::set(attrs);
}

void Glare::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new GlareInspector(this);
	}
}

void Glare::render()
{
	auto do_bloom = [&]() {
		m_gauss2dH.u_color0 = u_color0;
		m_gauss2dH.u_variance = m_variance;
		m_gauss2dH.ub_connect.sources[0].gain = m_gain;
		m_gauss2dH.ub_connect.sources[0].bias = m_bias;
		m_gauss2dH.begin();
		m_gauss2dH.render();
		m_gauss2dH.end();

		m_gauss2dQ.u_color0 = m_gauss2dH.getBuffer("color0").id();
		m_gauss2dQ.u_variance = m_variance * 2.0;
		m_gauss2dQ.begin();
		m_gauss2dQ.render();
		m_gauss2dQ.end();
	};

	// dir #0
	{
		m_gauss2dH.u_footstep = {m_footstep, m_footstep, m_footstep, m_footstep};
		m_gauss2dQ.u_footstep = {m_footstep, m_footstep, m_footstep, m_footstep};
		m_gauss2dQ.getRenderstate().flags.blend = false;
		do_bloom();
	}
	// dir #1
	{
		m_gauss2dH.u_footstep = {m_footstep, -m_footstep, m_footstep, -m_footstep};
		m_gauss2dQ.u_footstep = {m_footstep, -m_footstep, m_footstep, -m_footstep};
		m_gauss2dQ.getRenderstate().flags.blend = true;
		do_bloom();
	}

	// combine
	{
		u_color1 = m_gauss2dQ.getBuffer("color0").id();
		Tonemap::render();
	}
}
}  // namespace spu::gs_canvas
