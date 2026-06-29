//
// Raycast :
//
#include <gsys/painter/raycast.h>
#include <gsys/canvas.h>
#include <gsys/painter/stdout.h>
#include <gsys/decorator/material.h>

namespace spu::gs_painter {

Raycast::Raycast(const Attrs &attrs) { init(attrs); }

void Raycast::init(const Attrs &attrs)
{
	// parent
	{
		auto def_use_clut = attrs.get("clut.iformat", 0) != 0 ? 1 : 0;

		Attrs attrs = {
		        {"path",         "painter/raycast/raycast.us"},
		        {"def_use_clut", def_use_clut                },
		        {"a.a_position", 2                           },
		};
		GsPainter::init(attrs);
		GsPainter::getDecorators().push_back(new gs_decorator::Material(this, attrs));

		std::vector<vec2f_t> points = {
		        {-1.0, -1.0},
                        {+1.0, -1.0},
                        {+1.0, +1.0},
                        {+1.0, +1.0},
                        {-1.0, +1.0},
                        {-1.0, -1.0},
		};
		SpuArray::send(points.data(), points.size());
	}
	// shader
	{
		Attrs unif_attrs = {
		        {"u_depth",        &u_depth       },
		        {"u_volume",       &u_volume      },
		        {"u_clut",         &u_clut        },
		        {"u_occlusion",    &u_occlusion   },
		        {"u_viewvolume",   &u_viewvolume  },
		        {"u_fragview",     &u_fragview    },
		        {"u_volume_scale", &u_volume_scale},
		};
		addUniforms(unif_attrs);
	}

	// renderstate
	{
		auto &drawcall = getADrawcall();

		drawcall.flags.depth_test = true;
		drawcall.flags.blend = true;
		drawcall.flags.cull_face = false;
		drawcall.depth_func = GL_LEQUAL;

		// conventional
		drawcall.blend_func
		        = {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};
	}
}

void Raycast::doUse(uint32_t id)
{
	if (id == 0) {
		assert(instanceCount() == 1);
		const auto *current = GsCanvas::getCurrent();
		const auto *transform = (const Mat4f *)instancePtr();

		u_depth = current->getBuffer("depth").id();
		u_viewvolume = (current->worldview(0) * *transform).inverse();
		u_fragview = (current->screenfrag(0) * current->viewscreen()).inverse();
	}
	GsPainter::doUse(id);
}
}  // namespace spu::gs_painter
