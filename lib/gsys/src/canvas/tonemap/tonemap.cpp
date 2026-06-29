//
// Tonemap :
//
#include "spu/spu.h"
#include "tonemap_inspector.h"
namespace spu::gs_canvas {

void Tonemap::init(const Attrs &attrs)
{
	// parent
	{
		Attrs def_attrs = {
		        {"path", "canvas/tonemap/gather.us"},
		};
		GsCanvas::init(def_attrs + attrs);

		Attrs unif_attrs = {
		        {"u_luminance_texture",            &u_luminance_texture           },
		        {"u_tonemap_type",                 &u_tonemap_type                },
		        {"u_tonemap_is_adaptive_exposure", &u_tonemap_is_adaptive_exposure},
		        {"u_tonemap_is_vignette",          &u_tonemap_is_vignette         },
		};
		getShader().addUniforms(unif_attrs);
	}

	// size
	{
		m_size = attrs.get("size", 256);
		m_maxLevel = 0;
		for (auto size = m_size; size > 1; size /= 2) {
			aux_error(size % 2, "size (%d) must be power of 2\n");
			m_maxLevel++;
		}
	}

	// capture
	{
		auto viewport = Rectf(0, 0, m_size, m_size);
		Attrs attrs = {
		        {"viewport0",          viewport                   },
                        {"path",               "canvas/tonemap/capture.us"},
		        {"color0.target",      GL_TEXTURE_2D              },
                        {"color0.iformat",     GL_RGBA32F                 },
		        {"color0.mag_filter",  GL_LINEAR                  },
                        {"color0.min_filter",  GL_LINEAR                  },
		        {"color0.max_level",   m_maxLevel                 },
                        {"color0.auto_mipmap", 0                          },
		};
		m_captureCanvas.init(attrs);
	}

	// ambient texture
	{
		const auto bgcolor = eone<Vec4f>();
		Attrs attrs = {
		        {"target",      GL_TEXTURE_2D},
                        {"iformat",     GL_RGBA32F   },
                        {"width",       1            },
                        {"height",      1            },
		        {"auto_mipmap", 0            },
                        {"max_level",   0            },
                        {"data",        bgcolor.f    },
		};
		m_ambientTexture.init(attrs);
		u_ambient_texture = m_ambientTexture.id();
	}

	// luminance texture
	{
		const auto bgcolor = eone<Vec4f>();
		Attrs attrs = {
		        {"target",      GL_TEXTURE_2D},
                        {"iformat",     GL_R32F      },
                        {"width",       1            },
                        {"height",      1            },
		        {"auto_mipmap", 0            },
                        {"max_level",   0            },
                        {"data",        bgcolor.f    },
		};
		m_luminanceTexture.init(attrs);
		u_luminance_texture = m_luminanceTexture.id();
	}

	// compute shader
	{
		Attrs unif_attrs = {
		        {"u_captured_texture",      &u_captured_texture     },
		        {"u_ambient_texture",       &u_ambient_texture      },
		        {"u_luminance_texture",     &u_luminance_texture    },
		        {"u_tonemap_min_luminance", &u_tonemap_min_luminance},
		        {"u_tonemap_adapt_rate",    &u_tonemap_adapt_rate   },
		        {"u_tonemap_curve_bias",    &u_tonemap_curve_bias   },
		};
		m_adaptArray.getShader().init("canvas/tonemap/adapt.us");
		m_adaptArray.getShader().addUniforms(unif_attrs);
	}
	// change default
	ub_connect.sources[0].gain = 0.1;  // need fix
}

void Tonemap::adapt()
{
	// capture
	{
		m_captureCanvas.u_color0 = u_color0;
		m_captureCanvas.u_color1 = m_captureCanvas.getBuffer("color0").id();
		m_captureCanvas.begin();

		auto c_save = Composition(m_captureCanvas);

		for (auto level = 0; level <= m_maxLevel; level++) {
			m_captureCanvas.set("level", level);
			m_captureCanvas.ub_connect.sources[1].level = level;
			m_captureCanvas.sync(0);  // override params
			m_captureCanvas.render();
			m_captureCanvas.getViewports().at(0).sx /= 2;
			m_captureCanvas.getViewports().at(0).sy /= 2;
		}
		m_captureCanvas.end();
		*(Composition *)&m_captureCanvas = c_save;
		m_captureCanvas.set("level", 0);
	}

	// adaptive luminance
	{
		m_adaptArray.getShader().set("u_captured_texture.level", &m_maxLevel);  // 1x1
		u_captured_texture = m_captureCanvas.getBuffer("color0").id();

		auto dt = getSeconds().delta();
		if (dt > 0) {
			u_tonemap_adapt_rate = powf(m_adaptSpeed, dt);  // 10% in 1sec
		}
		m_adaptArray.compute();
	}

// #define MAINTENANCE
#ifdef MAINTENANCE
	{
		Vec4f color0;
		Vec4f color1;
		float color2;
		int32_t loc[4] = {0, 0, 0, m_maxLevel};

		m_captureCanvas.getBuffer("color0").recv(&color0, GL_RGBA32F, loc);
		m_ambientTexture.recv(&color1, GL_RGBA32F);
		m_luminanceTexture.recv(&color2, GL_R32F);

		spu_printf(
		        0, "exposure: (%8.5f,%8.5f,%8.5f) : (%8.5f,%8.5f,%8.5f) : %8.5f\n", color0.r, color0.g,
		        color0.b, color1.r, color1.g, color1.b, color2);
	}
#endif
}

void Tonemap::set(const Attrs &attrs)
{
	attrs.peek("adaptive_exposure", "use 'u_tonemap_is_adaptive_exposure' instead");
	GsCanvas::set(attrs);
}

void Tonemap::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new TonemapInspector(this);
	}
}

void Tonemap::render()
{
	assert((u_color0 >> 16) == 0 || (u_color0 >> 16) == GL_TEXTURE_2D);
	if (u_tonemap_is_adaptive_exposure) {
		adapt();
	}
	GsCanvas::render();
}
}  // namespace spu::gs_canvas
