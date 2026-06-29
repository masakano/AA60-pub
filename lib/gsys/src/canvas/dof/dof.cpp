//
// Dof :
//
#include "dof_inspector.h"

namespace spu::gs_canvas {

void Dof::init(const Attrs &attrs)
{
	auto c = Composition(*getCurrent());  // copy
	c.getViewports().at(0) = attrs.get("viewport0", c.viewport(0));

	auto add_unif = [&](SpuShader &shader) {
		Attrs unif_attrs = {
		        {"u_texcview",         &u_texcview        },
		        {"u_aperture_radius",  &u_aperture_radius },
		        {"u_focal_distance",   &u_focal_distance  },
		        {"u_footstep",         &u_footstep        },
		        {"u_color0_2d_array",  &u_color0          },
		        {"u_target",           &u_target          },
		        {"u_texture_size",     &u_texture_size    },
		        {"u_show_focal_point", &u_show_focal_point},
		};
		shader.addUniforms(unif_attrs);
	};

	// gauss #0
	{
		Attrs def_attrs = {
		        {"path",           "canvas/dof/dof.us"},
		        {"color0.iformat", GL_RGBA32F         },
		};
		m_gauss0.init(def_attrs + attrs);
		add_unif(m_gauss0.getShader());
	}

	// gauss #1
	{
		Attrs def_attrs = {
		        {"path",           "canvas/dof/dof.us"},
		        {"color0.iformat", GL_RGBA32F         },
		};
		m_gauss1.init(def_attrs + attrs);
		add_unif(m_gauss1.getShader());
	}
	u_texture_size = {c.viewport(0).sx, c.viewport(0).sy};

	// tonemap
	Tonemap::init(attrs);
}

void Dof::set(const Attrs &attrs)
{
	m_gauss0.set(attrs);
	m_gauss1.set(attrs);
	Tonemap::set(attrs);
}

void Dof::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new DofInspector(this);
	}
}

void Dof::render()
{
	// global
	auto *current = getCurrent();
	u_texcview = (Mat4f::screentexc() * current->viewscreen()).inverse();
	u_focal_distance = std::max(0.001f, u_focal_distance);

	//               (1,0)       (0,1)       (1,0)      (0,1)
	//  u_color0 -> m_gauss0 -> m_gauss1 -> m_gauss0 -> m_gauss1 -> tonemap
	//
	// gauss #0
	{
		u_footstep = {1, 0, 0, 0};
		u_target = u_color0 >> 16;

		m_gauss0.u_color0 = u_color0;
		m_gauss0.u_depth = u_depth;
		m_gauss0.begin();
		m_gauss0.render();
		m_gauss0.end();
	}

	// gauss #1
	{
		u_footstep = {0, 1, 0, 0};
		u_target = GL_TEXTURE_2D;

		m_gauss1.u_color0 = m_gauss0.getBuffer("color0").id();
		m_gauss1.u_depth = u_depth;
		m_gauss1.begin();
		m_gauss1.render();
		m_gauss1.end();
	}

	// gauss #2
	{
		u_footstep = {1, 0, 0, 0};
		u_target = GL_TEXTURE_2D;

		m_gauss0.u_color0 = m_gauss1.getBuffer("color0").id();
		m_gauss0.u_depth = u_depth;
		m_gauss0.begin();
		m_gauss0.render();
		m_gauss0.end();
	}

	// gauss #3
	{
		u_footstep = {0, 1, 0, 0};
		u_target = GL_TEXTURE_2D;

		m_gauss1.u_color0 = m_gauss0.getBuffer("color0").id();
		m_gauss1.u_depth = u_depth;
		m_gauss1.begin();
		m_gauss1.render();
		m_gauss1.end();
	}

	// tonemap
	{
		auto u_color0_save = u_color0;
		u_color0 = m_gauss1.getBuffer("color0").id();
		Tonemap::render();
		u_color0 = u_color0_save;
	}
}

}  // namespace spu::gs_canvas
