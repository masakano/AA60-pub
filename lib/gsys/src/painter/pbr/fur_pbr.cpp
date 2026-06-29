//
//
//
#include "fur_pbr_inspector.h"
#include <ssys/random_generator.h>
#include <ssys/half_float.h>

namespace spu::gs_painter {

void FurPBR::init(const Attrs &attrs)
{
	Attrs def_attrs = {
	        {"path.sub_radiance", "painter/pbr/fur_pbr_sub_radiance.us"},
	};
	PBR::init(def_attrs + attrs);

	Attrs unif_attrs = {
	        {"ub_fur",         &ub_fur        },
	        {"u_rand_texture", &u_rand_texture},
	};
	addUniforms(unif_attrs);
	generateRandTexture();
	ub_fur = {
	        .height = 0.05,
	        .position_jitter_scale = 0.5,
	        .normal_jitter_scale = 0.5,
	        .height_jitter_scale = 1.0,
	};
}

void FurPBR::generateRandTexture()
{
	const auto c_width = 256;
	const auto c_height = 256;

	RandomGenerator<float, std::normal_distribution<float>> frand;
	std::vector<HalfFloat> pix(c_width * c_height * 4 * 3);
	for (auto &p: pix) {
		p = HalfFloat(frand());
	}

	Attrs attrs = {
	        {"target",     GL_TEXTURE_2D_ARRAY    },
	        {"iformat",    GL_RGBA16F             },
	        {"width",      c_width                },
	        {"height",     c_height               },
	        {"depth",      3                      },
	        {"data",       pix.data()             },
	        {"wrap_s",     GL_REPEAT              },
	        {"wrap_t",     GL_REPEAT              },
	        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
	        {"mag_filter", GL_LINEAR              },
	};
	u_rand_texture = spu_texture_new(attrs);
}
void FurPBR::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new FurPBRInspector(this);
	}
}

}  // namespace spu::gs_painter
