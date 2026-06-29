//
// Copy :
//
#include <gsys/canvas/copy.h>

namespace spu::gs_canvas {

void Copy::init(const Attrs &attrs)
{
	Attrs def_attrs = {
	        {"path", "canvas/copy/copy.us"},
	};
	GsCanvas::init(def_attrs + attrs);

	Attrs unif_attrs = {
	        {"u_target",            &u_target},
                {"u_color0_2d_array",   &u_color0},
	        {"u_color0_cube",       &u_color0},
                {"u_color0_cube_array", &u_color0},
	        {"u_color0_3d",         &u_color0},
                {"u_color0_rect",       &u_color0},
	};
	auto &shader = getShader();
	shader.addUniforms(unif_attrs);
}

void Copy::render()
{
	struct ScopedCompareMode {
		uint32_t texture_id = 0;
		uint32_t compare_mode = 0;

		ScopedCompareMode(uint32_t id) : texture_id(id)
		{
			spu_texture_get(texture_id, "compare_mode", &compare_mode);
			if (compare_mode != GL_NONE) {
				spu_texture_set(texture_id, "compare_mode", GL_NONE);
			}
		}
		~ScopedCompareMode()
		{
			if (compare_mode != GL_NONE) {
				spu_texture_set(texture_id, "compare_mode", compare_mode);
			}
		}
	};
	ScopedCompareMode comapre_mode(u_color0);
	u_target = u_color0 >> 16;
	GsCanvas::render();
}
}  // namespace spu::gs_canvas
