//
// Shadowcast :
//
#pragma once

#include <gsys/canvas.h>

namespace spu::gs_node::volume {

class Shadowcast : public SpuComputeArray {
public:
	static constexpr auto def_local_size = 8;

	UB_LIGHT ub_light;
	Mat4f u_worldvolume;
	uint32_t u_volume = 0;
	uint32_t u_occlusion = 0;

	explicit Shadowcast(const Vec4i &grid_size)
	{
		// shader
		{
			Attrs shader_attrs = {
			        {"def_local_size", def_local_size},
			};

			Attrs unif_attrs = {
			        {"u_volume",      &u_volume     },
			        {"u_occlusion",   &u_occlusion  },
			        {"u_worldvolume", &u_worldvolume},
			        {"ub_light",      &ub_light     },
			};
			getShader().init("compute/volume/shadowcast.us", shader_attrs);
			getShader().addUniforms(unif_attrs);
		}

		// array
		{
			assert(grid_size.x % def_local_size == 0);
			assert(grid_size.y % def_local_size == 0);
			assert(grid_size.z % def_local_size == 0);

			getDim().x = grid_size.x / def_local_size;
			getDim().y = grid_size.y / def_local_size;
			getDim().z = grid_size.z / def_local_size;
		}
	}
};
}  // namespace spu::gs_node::volume
