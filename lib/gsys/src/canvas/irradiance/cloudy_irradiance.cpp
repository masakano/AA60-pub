//
// CloudyIrradiance :
//
#include <gsys/canvas/lightmap.h>
#include "cloudy_irradiance_inspector.h"

namespace spu::gs_canvas {

void CloudyIrradiance::Sun::update(UB_LIGHT &ub_light)
{
	if (is_auto_direction) {
		auto delta = 1.0f / 60.0f * 4.0f;
		if ((hour += delta * speed) > 20.0) {
			hour = 4.0;
		}
		auto orientation = Quatf((hour - 6.0f) * pi() / 12.0f, axis);
		direction = orientation * -ex();
	}
	else {
		direction = ub_light.sources[0].direction;
	}

	auto sun_height = direction.y;
	auto refraction_rate = 1.0f - sqrtf(std::max(0.0f, sun_height));
	auto sun_light = 1.0f - Vec3f(0.0, 0.25, 0.5) * refraction_rate;
	emission = sun_light * (0.75f * std::min(1.0f, std::max(0.0f, (0.03125f + sun_height) / 0.0625f)));
	ambient = sun_light * (0.1875f * std::min(1.0f, std::max(0.0f, (0.375f + sun_height) / 0.25f)));

	ub_light.ambient = ambient;
	ub_light.sources[0].emission = emission;
	ub_light.sources[0].direction = direction;
	ub_light.sources[0].position = direction * 1024;  // tentative
	ub_light.sources[0].type = e_ub_light_parallel;
}

void CloudyIrradiance::init(const Attrs &attrs)
{
	Irradiance::init(attrs);

	// uniform
	{
		auto &ub = ub_cloudy_irradiance;
		ub.cloud_bottom = 8.0;
		ub.cloud_top = 10.0;
		ub.granularity = 20.0;
		ub.octaves = 4.0;
		ub.lacunarity = 2.0;
		ub.gain = 0.5;
		ub.density_min = 0.2;
		ub.density_max = 0.5;
		ub.density_scale = 0.25;
		ub.sky_cutoff_angle = 0.95;
		ub.cloud_fade_angle = 0.1;
		ub.steps = 8;
	}

	// skydoem
	{
		auto viewport = Rectf(0, 0, c_skydome_width, c_skydome_width);
		auto path = "canvas/irradiance/skydome.us";

		Attrs canvas_attrs = {
		        {"viewport0",          viewport     },
                        {"path",               path         },
		        {"color0.target",      GL_TEXTURE_2D}, // rayleigh
		        {"color0.iformat",     GL_RGBA32F   },
                        {"color0.depth",       2            },
		        {"color0.max_level",   0            },
                        {"color0.auto_mipmap", 0            },
		        {"color1.target",      GL_TEXTURE_2D}, // mie
		        {"color1.iformat",     GL_RGBA32F   },
                        {"color1.depth",       2            },
		        {"color1.max_level",   0            },
                        {"color1.auto_mipmap", 0            },
		};
		m_skydomeCanvas.init(canvas_attrs);
		Attrs unif_attrs = {
		        {"ub_cloudy_irradiance", &ub_cloudy_irradiance}, // use sundir only
		};
		m_skydomeCanvas.getShader().addUniforms(unif_attrs);
	}

	// lightmap
	{
		Rectf viewport = {0, 0, c_lightmap_width, c_lightmap_width / 2};
		const char *path = "canvas/irradiance/cloudy_irradiance_lightmap.us";

		Attrs canvas_attrs = {
		        {"viewport0",          viewport               },
		        {"path",               path                   },
		        {"color0.target",      GL_TEXTURE_2D          },
		        {"color0.iformat",     GL_RGBA32F             },
		        {"color0.wrap_s",      GL_REPEAT              },
		        {"color0.wrap_t",      GL_CLAMP_TO_EDGE       },
		        {"color0.min_filter",  GL_LINEAR_MIPMAP_LINEAR},
		        {"color0.mag_filter",  GL_LINEAR              },
		        {"color0.auto_mipmap", 1                      },
		};
		m_lightmapCanvas.init(canvas_attrs);

		Attrs unif_attrs = {
		        {"ub_cloudy_irradiance", &ub_cloudy_irradiance                    },
		        {"u_skydomemapR",        &m_skydomeCanvas.getBuffer("color0").id()},
		        {"u_skydomemapM",        &m_skydomeCanvas.getBuffer("color1").id()},
		};
		m_lightmapCanvas.getShader().addUniforms(unif_attrs);
	}

	// irradmap
	{
		auto viewport = Rectf(0, 0, c_irradmap_width, c_irradmap_width / 2);
		const char *path = "canvas/irradiance/cloudy_irradiance_irradmap.us";

		Attrs canvas_attrs = {
		        {"viewport0",          viewport        },
                        {"path",               path            },
		        {"color0.target",      GL_TEXTURE_2D   },
                        {"color0.iformat",     GL_RGBA32F      },
		        {"color0.wrap_s",      GL_REPEAT       },
                        {"color0.wrap_t",      GL_CLAMP_TO_EDGE},
		        {"color0.min_filter",  GL_LINEAR       },
                        {"color0.mag_filter",  GL_LINEAR       },
		        {"color0.auto_mipmap", 0               },
                        {"color0.max_level",   0               },
		};
		m_irradmapCanvas.init(canvas_attrs);
		Attrs unif_attrs = {
		        {"ub_cloudy_irradiance", &ub_cloudy_irradiance                    },
		        {"u_skydomemapR",        &m_skydomeCanvas.getBuffer("color0").id()},
		        {"u_skydomemapM",        &m_skydomeCanvas.getBuffer("color1").id()},
		};
		m_irradmapCanvas.getShader().addUniforms(unif_attrs);
	}
}

void CloudyIrradiance::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new CloudyIrradianceInspector(this);
	}
}

void CloudyIrradiance::update()
{
	if (getSeconds().count() % m_interval == 0) {
		m_sun.update(ub_light);

		auto offset = getSeconds().current() * m_speed;
		auto &ub = ub_cloudy_irradiance;
		ub.sundir = m_sun.direction;
		ub.offset = vec3f_t(offset, 0, -offset);  // need direction

		ub.cloud_top = std::max(ub.cloud_top, ub.cloud_bottom);
		ub.density_max = std::max(ub.density_min, ub.density_max);

		m_skydomeCanvas.begin();
		m_skydomeCanvas.render();
		m_skydomeCanvas.end();

		m_lightmapCanvas.begin();
		m_lightmapCanvas.render();
		m_lightmapCanvas.end();

		m_irradmapCanvas.begin();
		m_irradmapCanvas.render();
		m_irradmapCanvas.end();

		// m_touchCount++;
		Irradiance::update();
	}
}

void CloudyLightmap::init(const Attrs &attrs)
{
	m_irradiance = new CloudyIrradiance(attrs.select("irradiance."));
	Lightmap::init(attrs);
}

}  // namespace spu::gs_canvas
