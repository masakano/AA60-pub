//
// GsGauss3D :
//
#include <gsys/util/gauss_3d.h>

namespace spu {

void GsGauss3D::init(uint32_t dst_texture, uint32_t src_texture)
{
	uint32_t iformat;
	int32_t width;
	int32_t height;
	int32_t depth;

	m_srcTexture = src_texture;
	m_dstTexture = dst_texture;

	// texture
	{
		spu_texture_get(m_srcTexture, "iformat", &iformat);
		spu_texture_get(m_srcTexture, "width", &width);
		spu_texture_get(m_srcTexture, "height", &height);
		spu_texture_get(m_srcTexture, "depth", &depth);

		Attrs tex_attrs = {

		        {"target",      GL_TEXTURE_3D   },
		        {"iformat",     iformat         },
		        {"min_filter",  GL_LINEAR       },
		        {"mag_filter",  GL_LINEAR       },
		        {"max_level",   0               },
		        {"auto_mipmap", 0               },

		        {"wrap_s",      GL_CLAMP_TO_EDGE},
		        {"wrap_t",      GL_CLAMP_TO_EDGE},
		        {"wrap_r",      GL_CLAMP_TO_EDGE},

		        {"width",       width           },
		        {"height",      height          },
		        {"depth",       depth           },
		};
		m_texture = SpuTexture(tex_attrs);
		u_size = {width, height, depth, 1};

		// u_size.report("u_size");
	}

	// shader
	{
		Attrs shader_attrs = {
		        {"def_local_size", def_local_size},
		};
		Attrs unif_attrs = {
		        {"u_weight",  &u_weight[0]},
                        {"u_delta",   &u_delta    },
                        {"u_volume0", &u_volume0  },
		        {"u_volume1", &u_volume1  },
                        {"u_size",    &u_size.iv  },
		};
		m_array.getShader().init("canvas/gauss/gauss_3d.us", shader_attrs);
		m_array.getShader().addUniforms(unif_attrs);
	}

	// array
	{
		assert(width % def_local_size == 0);
		assert(height % def_local_size == 0);
		assert(depth % def_local_size == 0);

		m_array.getDim() = {width / def_local_size, height / def_local_size, depth / def_local_size, 1};

		setVariance(1.0);
	}
}

void GsGauss3D::setVariance(float variance)
{
	const int32_t N = 15;

	// patch
	if (u_size.x <= 16 || u_size.y <= 16 || u_size.z <= 16) {
		variance = 0;
	}

	if (variance == 0) {
		for (auto i = 0; i < N; i++) {
			u_weight[i] = i == N / 2 ? 1 : 0;
		}
	}
	else {
		float sum = 0.0;
		for (auto i = 0; i < N; i++) {
			float x = i - N / 2;
			u_weight[i] = expf(-x * x / (2 * variance * variance));
			sum += u_weight[i];
		}
		for (float& i: u_weight) {
			i /= sum;
		}
	}
}

void GsGauss3D::draw()
{
	// x #0 -> #1
	{
		u_volume0 = m_srcTexture;
		u_volume1 = m_texture.id();
		u_delta = {1, 0, 0, 0};
		m_array.compute();
	}

	// y #1 -> #0
	{
		u_volume0 = m_texture.id();
		u_volume1 = m_dstTexture;
		u_delta = {0, 1, 0, 0};
		m_array.compute();
	}
	// z #0 -> #1
	{
		u_volume0 = m_dstTexture;
		u_volume1 = m_texture.id();
		u_delta = {0, 0, 1, 0};

		m_array.compute();
	}
	// copy #1 -> #0
	spu_texture_copy(m_dstTexture, m_texture.id());
}
}  // namespace spu
