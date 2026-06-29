//
// Temporal :
//
#pragma once
#include <gsys/canvas/copy.h>

namespace spu::gs_canvas {

class Temporal : public Copy {
public:
	explicit Temporal(const char *name = nullptr) : Copy(name) {}
	explicit Temporal(const Attrs &attrs) : Temporal() { init(attrs); }

	void init(const Attrs &attrs) override
	{
		Copy::init(attrs);

		Attrs accum_canvas_attrs = {
		        {"viewport0",          viewport(0)         },
                        {"color0.target",      GL_TEXTURE_2D       },
		        {"color0.iformat",     GL_RGBA16F          },
                        {"color0.min_filter",  GL_LINEAR           },
		        {"color0.mag_filter",  GL_LINEAR           },
                        {"color0.max_level",   0                   },
		        {"color0.auto_mipmap", 0                   },
                        {"path",               "canvas/temporal.us"},
		};
		m_accum.init(accum_canvas_attrs);
	}
	void set(const Attrs &attrs) override { m_accum.set(attrs); }
	void render() override
	{
		m_accum.u_color0 = u_color0;
		m_accum.u_color1 = m_accum.getBuffer("color0").id();
		m_accum.begin();
		m_accum.render();
		m_accum.end();

		u_color0 = m_accum.getBuffer("color0").id();

		Copy::render();
		u_color0 = m_accum.u_color0;  // restore
	}

protected:
	gs_canvas::Copy m_accum;
};
}  // namespace spu::gs_canvas
