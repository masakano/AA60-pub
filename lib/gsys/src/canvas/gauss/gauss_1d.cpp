//
// Gauss1D :
//
#include <gsys/canvas/gauss.h>

namespace spu::gs_canvas {

void Gauss1D::init(const Attrs &attrs)
{
	Attrs def_attrs = {
	        {"path", "canvas/gauss/gauss.us"},
	};
	GsCanvas::init(def_attrs + attrs);

	Attrs unif_attrs = {
	        {"u_color0_2d_array", &u_color0      },
                {"u_footstep",        &u_footstep    },
                {"u_target",          &u_target      },
	        {"u_variance",        &u_variance    },
                {"u_texture_size",    &u_texture_size},
	};
	getShader().addUniforms(unif_attrs);
}
void Gauss1D::render()
{
	aux_error(ub_connect.sources[0].level > 0, "lod (%d) not supported\n", ub_connect.sources[0].level);

	auto *current = getCurrent();
	u_texture_size = {current->viewport(0).sx, current->viewport(0).sy};
	u_target = u_color0 >> 16;
	GsCanvas::render();
}
}  // namespace spu::gs_canvas
