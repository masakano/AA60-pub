//
// Sketch :
//
#pragma once
#include "pbr.h"
#include <ssys/random_generator.h>

namespace spu::gs_painter {

class Sketch : public PBR {
public:
	int32_t u_rand_texture;
	float u_roughness = 0.5;
	float u_specular = 4.0;
	float u_brightness = 64.0;

	explicit Sketch(const char *name = nullptr) : PBR(name) {}
	explicit Sketch(const Attrs &attrs) : PBR() { init(attrs); }

	void init(const Attrs &attrs) override
	{
		Attrs def_attrs = {
		        {"path.radiance", "painter/pbr/sketch_radiance.us"},
		};

		PBR::init(def_attrs + attrs);
		m_randTexture = generateTexture();
		u_rand_texture = m_randTexture.id();

		Attrs unif_attrs = {
		        {"u_rand_texture", &u_rand_texture},
		        {"u_roughness",    &u_roughness   },
		        {"u_specular",     &u_specular    },
		        {"u_brightness",   &u_brightness  },
		};
		addUniforms(unif_attrs);
	}

protected:
	SpuTexture m_randTexture;

	SpuTexture generateTexture()
	{
		const auto c_width = 1024;
		const auto c_height = 1024;
		const auto c_count = 8;  // stich count

		RandomGenerator<float, std::normal_distribution<float>> frand;
		std::vector<float> pix0(c_width * c_height);
		std::vector<float> pix1(c_width * c_height);
		const void *pix = pix0.data();

		for (auto &p1: pix1) {
			p1 = frand();
		}

		for (auto &p0: pix0) {
			int32_t n = &p0 - &pix0[0];
			int32_t x0 = n / c_width;
			int32_t y0 = n % c_width;

			p0 = 0;
			for (auto i = 0; i < c_count; i++) {
				int32_t x1 = (x0 + i) % c_width;
				int32_t y1 = (y0 + i) % c_height;
				p0 += pix1[x1 + y1 * c_width];
			}
			p0 /= c_count;
		}

		Attrs attrs = {
		        {"target",      GL_TEXTURE_2D},
		        {"iformat",     GL_R32F      },
		        {"width",       c_width      },
		        {"height",      c_height     },
		        {"data",        pix          },
		        {"wrap_s",      GL_REPEAT    },
		        {"wrap_t",      GL_REPEAT    },
		        {"min_filter",  GL_LINEAR    },
		        {"mag_filter",  GL_LINEAR    },
		        {"auto_mipmap", 0            },
		};
		return SpuTexture(attrs);
	}
};
}  // namespace spu::gs_painter
