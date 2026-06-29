//
// Lensflare2 :
//
#include "lensflare2_inspector.h"

namespace spu::gs_canvas {

void Lensflare2::init(const Attrs &attrs)
{
	// textures
	{
		Attrs tex_attrs = {
		        {"min_filter", GL_LINEAR       },
		        {"mag_filter", GL_LINEAR       },
		        {"wrap_s",     GL_CLAMP_TO_EDGE},
		        {"wrap_t",     GL_CLAMP_TO_EDGE},
		};

		Attrs clut_attrs = {
		        {"target",      GL_TEXTURE_1D},
                        {"auto_mipmap", 0            },
                        {"min_filter",  GL_LINEAR    }, // ssao
		        {"mag_filter",  GL_LINEAR    }, // ssao
		        {"wrap_s",      GL_REPEAT    }, // ssao
		};
		m_lensDirtTexture.init("canvas/lensflare2/lensdirt.png", tex_attrs);
		m_lensStarTexture.init("canvas/lensflare2/lensstar.png", tex_attrs);
		m_lensColorTexture.init("canvas/lensflare2/lenscolor.png", clut_attrs);

		u_lens_dirt_texture = m_lensDirtTexture.id();
		u_lens_star_texture = m_lensStarTexture.id();
		u_lens_color_texture = m_lensColorTexture.id();
	}
	Tonemap::init(attrs);

	// aux canvases
	auto *current = getCurrent();
	auto aux_viewport = attrs.get("viewport0", current->viewport(0));

	Attrs aux_attrs = {
	        {"viewport0",          aux_viewport    },
	        {"color0.target",      GL_TEXTURE_2D   },
	        {"color0.iformat",     GL_RGBA32F      },
	        {"color0.min_filter",  GL_LINEAR       },
	        {"color0.mag_filter",  GL_LINEAR       },
	        {"color0.wrap_s",      GL_CLAMP_TO_EDGE},
	        {"color0.wrap_t",      GL_CLAMP_TO_EDGE},
	        {"color0.auto_mipmap", 0               },
	        {"color0.max_level",   0               },
	};

	// gauss blur
	{
		Attrs attrs = {
		        {"path", "canvas/lensflare2/gauss1d.us"},
		};
		m_gaussCanvas.init(aux_attrs + attrs);

		Attrs unif_attrs = {
		        {"u_input_texture",  &u_input_texture },
		        {"u_blur_radius",    &u_blur_radius   },
		        {"u_blur_direction", &u_blur_direction},
		};
		m_gaussCanvas.getShader().addUniforms(unif_attrs);
	}

	// scale and bias
	{
		Attrs attrs = {
		        {"path", "canvas/lensflare2/scaleBias.us"},
		};
		m_scaleBiasCanvas.init(aux_attrs + attrs);

		Attrs unif_attrs = {
		        {"u_input_texture", &u_input_texture},
		        //{"u_scale",         &u_scale        },
		        //{"u_bias",          &u_bias         },
		};
		m_scaleBiasCanvas.getShader().addUniforms(unif_attrs);
		m_scaleBiasCanvas.getRenderstate().blend_func = {
		        GL_ONE,
		        GL_ONE,
		        GL_ONE,
		        GL_ONE,
		};
	}
	// bloom
	{
		m_bloomCanvas.init(aux_attrs);  // no shader
	}

	// lensflare
	{
		Attrs attrs = {
		        {"path", "canvas/lensflare2/lensflare.us"},
		};
		m_flareCanvas.init(aux_attrs + attrs);

		Attrs unif_attrs = {
		        {"u_input_texture",      &u_input_texture     },
		        {"u_lens_color_texture", &u_lens_color_texture},
		        {"u_lens_dirt_texture",  &u_lens_dirt_texture },
		        {"u_lens_star_texture",  &u_lens_star_texture },
		        {"u_flare_dispersal",    &u_flare_dispersal   },
		        {"u_flare_halo_width",   &u_flare_halo_width  },
		        {"u_flare_distortion",   &u_flare_distortion  },
		        {"u_flare_samples",      &u_flare_samples     },
		};
		m_flareCanvas.getShader().addUniforms(unif_attrs);
	}

	// gbuffer
	{
		const auto *current = getCurrent();
		aux_attrs.replace("viewport0", current->viewport(0));  // full size
		m_gbufferCanvas.init(aux_attrs);
	}
}

void Lensflare2::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new Lensflare2Inspector(this);
	}
}

void Lensflare2::set(const Attrs &attrs)
{
	attrs.peek("bloom_scale", "use 'bloom_gain' instead");
	attrs.peek("flare_scale", "use 'flare_gain' instead");

#if 1
	attrs.apply("bloom_scale", m_bloomGain);
	attrs.apply("bloom_bias", m_bloomBias);
	attrs.apply("bloom_blur_radius", m_bloomBlurRadius);

	attrs.apply("flare_gain", m_flareBlurRadius);
	attrs.apply("flare_bias", m_flareBias);
	attrs.apply("flare_blur_radius", m_flareBlurRadius);

	attrs.apply("flare_samples", m_flareSamples);
	attrs.apply("flare_dispersal", m_flareDispersal);
	attrs.apply("flare_halo_width", m_flareHaloWidth);
	attrs.apply("flare_distortion", m_flareDistortion);
	attrs.apply("flare_blur_radius", m_flareBlurRadius);
	attrs.apply("output_mode", m_outputMode);

#else
	m_bloomGain = attrs.get("bloom_scale", m_bloomGain);
	m_bloomBias = attrs.get("bloom_bias", m_bloomBias);
	m_bloomBlurRadius = attrs.get("bloom_blur_radius", m_bloomBlurRadius);

	m_flareGain = attrs.get("flare_gain", m_flareBlurRadius);
	m_flareBias = attrs.get("flare_bias", m_flareBias);
	m_flareBlurRadius = attrs.get("flare_blur_radius", m_flareBlurRadius);

	m_flareSamples = attrs.get("flare_samples", m_flareSamples);
	m_flareDispersal = attrs.get("flare_dispersal", m_flareDispersal);
	m_flareHaloWidth = attrs.get("flare_halo_width", m_flareHaloWidth);
	m_flareDistortion = attrs.get("flare_distortion", m_flareDistortion);
	m_flareBlurRadius = attrs.get("flare_blur_radius", m_flareBlurRadius);
	m_outputMode = attrs.get("output_mode", m_outputMode);
#endif
	Tonemap::set(attrs);
}

void Lensflare2::gaussBlur(GsCanvas &canvas, uint32_t input_texture, float blur_radius)
{
	// input_texture -> gauss
	{
		m_gaussCanvas.begin();
		u_blur_radius = blur_radius;
		u_blur_direction = {1.0f, 0.0f};
		u_input_texture = input_texture;
		m_gaussCanvas.render();
		m_gaussCanvas.end();
	}
	// gauss -> canvas
	{
		canvas.begin();
		u_blur_radius = blur_radius;
		u_blur_direction = {0.0f, 1.0f};
		u_input_texture = m_gaussCanvas.getBuffer("color0").id();
		m_gaussCanvas.takeover();
		m_gaussCanvas.render();
		canvas.end();
	}
}

void Lensflare2::render()
{
	//                       +- tonemap  ---------------> output
	//                       |
	// u_color0 -+- gbuffer -+- scaleBias -- bloom -----+
	//           ^           |                          |
	//           ^           +- scaleBias -- lensflare--+
	//           ^                                      v
	//           ^                                      v
	//           <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	//

	// gbuffer
	{
		auto &renderstate = m_scaleBiasCanvas.getRenderstate();

		m_gbufferCanvas.begin();
		m_gbufferCanvas.clear();
		m_scaleBiasCanvas.takeover();

		// u_color0
		u_input_texture = u_color0;
		m_scaleBiasCanvas.ub_connect.sources[0].gain = ub_connect.sources[0].gain;
		m_scaleBiasCanvas.ub_connect.sources[0].bias = 0.0;
		m_scaleBiasCanvas.render();

		renderstate.flags.blend = true;

		// bloom
		u_input_texture = m_bloomCanvas.getBuffer("color0").id();

		m_scaleBiasCanvas.ub_connect.sources[0].gain = 1.0;
		m_scaleBiasCanvas.ub_connect.sources[0].bias = 0.0;
		m_scaleBiasCanvas.render();

		// flare
		u_input_texture = m_flareCanvas.getBuffer("color0").id();
		m_scaleBiasCanvas.ub_connect.sources[0].gain = 1.0;
		m_scaleBiasCanvas.ub_connect.sources[0].bias = 0.0;
		m_scaleBiasCanvas.render();

		m_gbufferCanvas.end();

		renderstate.flags.blend = false;
	}

	// bloom scale bias
	{
		m_scaleBiasCanvas.getRenderstate().flags.blend = false;
		m_scaleBiasCanvas.begin();

		u_input_texture = m_gbufferCanvas.getBuffer("color0").id();

		m_scaleBiasCanvas.ub_connect.sources[0].gain = m_bloomGain;
		m_scaleBiasCanvas.ub_connect.sources[0].bias = m_bloomBias;
		m_scaleBiasCanvas.render();
		m_scaleBiasCanvas.end();

		gaussBlur(m_bloomCanvas, m_scaleBiasCanvas.getBuffer("color0").id(), m_bloomBlurRadius);
	}

	// flareflare scale bias
	{
		m_scaleBiasCanvas.begin();

		u_input_texture = m_gbufferCanvas.getBuffer("color0").id();
		m_scaleBiasCanvas.ub_connect.sources[0].gain = m_flareGain;
		m_scaleBiasCanvas.ub_connect.sources[0].bias = m_flareBias;
		m_scaleBiasCanvas.render();

		m_scaleBiasCanvas.end();
	}

	// lensflare
	{
		auto *canvas = getCurrent();
		auto worldview = canvas->worldview(0);
		auto camx = Vec3f(worldview.c[0]);
		auto camz = Vec3f(worldview.c[2]);
		auto camrot = dot(camx, ez()) + dot(camz, ey());

		camrot *= 4.0f;

		// clang-format off
		Mat4f starRotationMat = {
			cosf(camrot) * 0.5f, -sinf(camrot), 0.0f, 0.0f,
			sinf(camrot), cosf(camrot) * 0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		Mat4f sb1 = {
			2.0f, 0.0f, -1.0f, 0.0f,
			0.0f, 2.0f, -1.0f, 0.0f,
			0.0f, 0.0f, +1.0f, 0.0f,
			0.0f, 0.0f, +0.0f, 1.0f
		};

		Mat4f sb2 = {
		        0.5f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.5f, 0.5f, 0.0f,
		        0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
		// clang-format on

		starRotationMat = sb2 * starRotationMat * sb1;

		u_lens_star_matrix = starRotationMat;
		u_input_texture = m_scaleBiasCanvas.getBuffer("color0").id();

		u_flare_dispersal = m_flareDispersal;
		u_flare_halo_width = m_flareHaloWidth;
		u_flare_distortion = m_flareDistortion;

		m_flareCanvas.begin();
		m_flareCanvas.render();
		m_flareCanvas.end();
		gaussBlur(m_flareCanvas, m_flareCanvas.getBuffer("color0").id(), m_flareBlurRadius);
	}

	// gbuffer -> output
	{
		auto u_color0_save = u_color0;
		auto ub_connect_save = ub_connect;

		switch (m_outputMode) {
		case e_source_only: break;
		case e_bloom_only: u_color0 = m_bloomCanvas.getBuffer("color0").id(); break;
		case e_flare_only: u_color0 = m_flareCanvas.getBuffer("color0").id(); break;
		case e_all: u_color0 = m_gbufferCanvas.getBuffer("color0").id(); break;
		}
		// printf("u_color0=%08x: gbuffer=%08x\n", u_color0, m_gbufferCanvas.getBuffer("color0").id());

		ub_connect.sources[0].gain = 1.0;
		Tonemap::render();
		u_color0 = u_color0_save;
		ub_connect = ub_connect_save;
	}
}
}  // namespace spu::gs_canvas
