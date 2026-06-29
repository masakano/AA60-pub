//
// WaterPlane :
//
#include "water_plane_inspector.h"
#include <ssys/random_generator.h>

namespace spu::gs_painter {

void WaterPlane::init(const Attrs &attrs)
{
	// painter
	{
		const char *radiance_path = "painter/plane/radiance.us";
		const char *def_plane_frag_path = "<water_radiance.us>";
		Attrs init_attrs = {
		        {"path.radiance",       radiance_path      },
		        {"def_plane_frag_path", def_plane_frag_path},
		};
		Plane::init(attrs + init_attrs);
	}

	// fakewater map
	{
		Attrs attrs = {
		        {"iformat",     GL_R32F                },
                        {"wrap_s",      GL_MIRRORED_REPEAT     },
		        {"wrap_t",      GL_MIRRORED_REPEAT     },
                        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
		        {"mag_filter",  GL_LINEAR              },
                        {"auto_mipmap", 1                      },
		};
		m_waterHeightmap.init("painter/plane/water_height.hdr", attrs);
	}

	// realwater generate
	{
		float wind_speed = 4.0;
		float wind_direction = 90.0;
		RealWaveGenerator generator;
		u_real_wave_count = generator.generate(ub_real_wave, wind_speed, wind_direction);
	}

	// uniforms
	{
		Attrs unif_attrs = {
		        {"u_use_realwater",      &u_use_realwater        },

		        // fake water
		        {"u_water_heightmap",    &m_waterHeightmap.id()  },
		        {"u_water_height_scale", &u_water_height_scale   },
		        {"u_water_maptexcoords", &u_water_maptexcoords[0]},

		        // real water
		        {"ub_real_wave",         &ub_real_wave           },
		        {"u_real_wave_count",    &u_real_wave_count      },
		        {"u_time",               &u_time                 },
		};
		Plane::addUniforms(unif_attrs);
	}

	// default
	{
		auto &drawcall = getADrawcall();
		Attrs texture_attrs = {
		        {"iformat", GL_SRGB8_ALPHA8},
		};
		drawcall.albedomap.init("painter/plane/water1.jpg", texture_attrs);
		drawcall.ub_material.roughness = 0.3;
		drawcall.ub_material.metallic = 0.1;
		drawcall.flags.cull_face = false;
		drawcall.flags.depth_test = true;
		drawcall.flags.blend = true;
	}
}

void WaterPlane::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new WaterPlaneInspector(this);
	}
}

void WaterPlane::update()
{
	// fakewater update
	if (u_use_realwater == 0) {
		static RandomGenerator<float> frand = {0.5, 2.0};
		static float trans[4];

		const float rot[4] = {
		        0.0f,
		        pi() / 7.0f,
		        pi() / 3.0f,
		        pi() / 1.3f,
		};
		const float dx[4] = {
		        +0.00020,
		        -0.00034,
		        +0.00044,
		        -0.00026,
		};
		const float scale[4] = {
		        0.3,
		        0.5,
		        0.7,
		        0.9,
		};

		Mat4f unit;
		for (auto i = 0; i < 4; i++) {
			float x = (trans[i] += dx[i] * frand() * 2);
			u_water_maptexcoords[i] = unit.rot("z", rot[i]).scale(scale[i]).trans({x, 0, 0});
		}
	}
	else {
		u_time = getSeconds().current() * 4.0;  // need parameterize
	}
	Plane::update();
}
}  // namespace spu::gs_painter
