//
// SSAO :
//
#include "ssao_inspector.h"
#include <ssys/random_generator.h>
#include <smath/poisson_disk.h>

namespace spu::gs_canvas {
namespace {

uint32_t create_samples(Vec3f samples[256], uint32_t quality)
{
	const float c_steps[] = {
	        // 3.0f, 3.5f, 4.5f, 5.5f,
	        4.0f,
	        5.0f,
	        6.0f,
	        8.0f,
	};
	const auto step = c_steps[quality];

	PoissonDisk<Vec3f> pdisk;
	pdisk.exec(1.0, 1.0 / step);  // ad-hoc

	auto points = pdisk.points();
	auto count = 0u;

	for (auto &p0: points) {
		if (p0.z > 0) {
			samples[count++] = p0;
			if (count == 256) {
				break;
			}
		}
	}

// #define MAINTENANCE
#ifdef MAINTENANCE
	{
		printf("points=%ld:%d\n", points.size(), count);
		File file("output.dat", "w");
		for (auto i = 0u; i < count; i++) {
			auto &p = samples[i];
			file.printf("%f %f %f\n", p.x, p.y, p.z);
		}
	}
#endif
	aux_message(0, "SSAO quality=%d samples=%ld (step=%.2f)\n", quality, points.size(), step);
	return count;
}
}  // namespace

void SSAO::init(const Attrs &attrs)
{
	// SSAO
	{
		Attrs ssao_attrs = {
		        {"color0.iformat",     GL_RGBA32F           },
                        {"color0.target",      GL_TEXTURE_2D        },
		        {"color0.min_filter",  GL_NEAREST           },
                        {"color0.mag_filter",  GL_NEAREST           },
		        {"color0.wrap_s",      GL_CLAMP             },
                        {"color0.wrap_t",      GL_CLAMP             },
		        {"color0.auto_mipmap", 0                    },
                        {"path",               "canvas/ssao/ssao.us"},
		};

		m_ssaoCanvas.init(ssao_attrs);
		u_sample_count = create_samples(ub_samples, m_quality);
		Attrs unif_attrs = {
		        {"u_texture_size",  &u_texture_size },
                        {"u_depth",         &u_depth        },
		        {"u_texcview",      &u_texcview     },
                        {"u_viewtexc",      &u_viewtexc     },
		        {"u_sample_count",  &u_sample_count },
                        {"u_sample_radius", &u_sample_radius},
		        {"ub_samples",      &ub_samples[0]  },
		};
		auto &shader = m_ssaoCanvas.getShader();
		shader.addUniforms(unif_attrs);
	}
	// blend
	{
		Attrs def_attrs = {
		        {"path", "canvas/ssao/ssao_blend.us"},
		};
		GsCanvas::init(def_attrs + attrs);

		Attrs unif_attrs = {
		        {"u_ssao_only",  &u_ssao_only },
		        {"u_ssao_lerp",  &u_ssao_lerp },
		        {"u_ssao_power", &u_ssao_power},
		};
		auto &shader = getShader();
		shader.addUniforms(unif_attrs);
	}
}

void SSAO::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new SSAOInspector(this);
	}
}

void SSAO::render()
{
	auto *current = getCurrent();

	u_texcview = (Mat4f::screentexc() * current->viewscreen()).inverse();
	u_viewtexc = u_texcview.inverse();
	u_texture_size = Vec2f(current->viewport(0).sx, current->viewport(0).sy);

	// update
	{
		if (m_prevQuality != m_quality) {
			u_sample_count = create_samples(ub_samples, m_quality);
			m_prevQuality = m_quality;
		}
	}

	// ssao
	{
		m_ssaoCanvas.begin();
		m_ssaoCanvas.render();
		m_ssaoCanvas.end();
	}

	// blend
	{
		u_color3 = m_ssaoCanvas.getBuffer("color0").id();  // color#3 vacancy
		GsCanvas::render();
	}
}

void SSAO::set(const Attrs &attrs)
{
	m_ssaoCanvas.set(attrs);
	GsCanvas::set(attrs);
}
}  // namespace spu::gs_canvas
