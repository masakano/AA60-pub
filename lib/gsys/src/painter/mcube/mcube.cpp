//
// Mcube :
//
#include "mcube_tab.h"
#include <gsys/painter/mcube.h>
#include <gsys/canvas.h>
#include <gsys/decorator/instance.h>
#include <gsys/decorator/material.h>
#include <gsys/painter/stdout.h>

namespace spu::gs_painter {

Mcube::Mcube(const Attrs &attrs) { init(attrs); }

Mcube::~Mcube()
{
	spu_texture_delete(u_edges);
	spu_texture_delete(u_vertices);
}

void Mcube::init(const Attrs &attrs)
{
	Vec4i grid_size = attrs.get("grid.size", vec4i_t(64, 64, 64, 1));

	u_grid_box_size = grid_size - 1;

	// parent
	{
		Attrs def_attrs = {
		        {"path",  "painter/mcube/mcube.us"                                 },
		        {"nelem", u_grid_box_size.x * u_grid_box_size.y * u_grid_box_size.z},
		};
		Attrs all_attrs = def_attrs + attrs;

		GsPainter::init(all_attrs);
		getDecorators().push_back(new gs_decorator::Instance(this, all_attrs));
		getDecorators().push_back(new gs_decorator::Material(this, all_attrs));

		Attrs unif_attrs = {
		        {"u_gridworld",        &u_gridworld       },
		        {"u_grid_box_size",    &u_grid_box_size   },
		        {"u_edges",            &u_edges           },
		        {"u_vertices",         &u_vertices        },
		        {"u_volume",           &u_volume          },
		        {"u_volume_threshold", &u_volume_threshold},
		        {"u_volume_scale",     &u_volume_scale    },
		};
		addUniforms(unif_attrs);
	}

	// renderstate
	{
		auto &drawcall = getADrawcall();
		drawcall.flags.depth_test = true;
		drawcall.flags.fill = true;
		drawcall.flags.cull_face = false;
		drawcall.flags.blend = false;
	}

	// edge texture
	{
		Attrs attrs = {
		        {"target",      GL_TEXTURE_1D       },
                        {"iformat",     GL_R32I             },
                        {"width",       256                 },
		        {"max_level",   0                   },
                        {"auto_mipmap", 0                   },
                        {"data",        spu::volume::c_edges},
		};
		u_edges = spu_texture_new(attrs);
	}
	// vertices texture
	{
		Attrs attrs = {
		        {"target",      GL_TEXTURE_2D          },
		        {"iformat",     GL_R32I                },
		        {"width",       16                     },
		        {"height",      256                    },
		        {"max_level",   0                      },
		        {"auto_mipmap", 0                      },
		        {"data",        spu::volume::c_vertices},
		};
		u_vertices = spu_texture_new(attrs);
	}
	m_gridvolume = Mat4f().scale(2.0 / Vec3f(grid_size)).trans(Vec3f(-1.0));
	// m_intStride = 1;  // dummy
}

void Mcube::doUse(uint32_t id)
{
	if (id == 0) {
		assert(instanceCount() == 1);
		// auto &c = current->getComposition();
		auto volumeworld = *(const Mat4f *)instancePtr();
		// auto volumeview = c.worldview() * volumeworld;
		// u_gridview = volumeview * m_gridvolume;
		u_gridworld = volumeworld * m_gridvolume;
	}
	auto &coms = getDrawcalls().at(id).coms;
	for (auto &com: coms) {
		com.mode = GL_POINTS;
	}
	GsPainter::doUse(id);
}
}  // namespace spu::gs_painter
