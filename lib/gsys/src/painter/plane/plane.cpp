//
// Plane :
//
#include "plane_inspector.h"
#include <gsys/decorator/material.h>
#include <gsys/decorator/shadowmap.h>
#include <gsys/canvas/shadowmap.h>  // debug

namespace spu::gs_painter {

void Plane::init(const Attrs &attrs)
{
	// painter
	{
		const char *radiance_path = "painter/plane/radiance.us";
		const char *def_plane_frag_path = "<plane_radiance.us>";
		Attrs def_attrs = {
		        {"path.radiance",       radiance_path      },
		        {"def_plane_frag_path", def_plane_frag_path},
		        {"path.depth",          nullptr            },
		        {"a.a_position",        4                  },
		};
		GsPainter::init(def_attrs + attrs);
		getDecorators().push_back(new gs_decorator::Material(this, attrs));
		getDecorators().push_back(new gs_decorator::Shadowmap(this, attrs));
	}

	// canvas
	{
		auto &viewport = GsCanvas::getCurrent()->viewport(0);
		Attrs canvas_attrs = {
		        {"viewport0",          viewport             },
		        {"color0.target",      GL_TEXTURE_2D        },
		        {"color0.iformat",     GL_RGBA32F           },
		        {"color0.max_level",   0                    },
		        {"color0.auto_mipmap", 0                    },
		        {"depth.target",       GL_RENDERBUFFER      },
		        {"depth.iformat",      GL_DEPTH_COMPONENT32F},
		};
		m_canvas.init(canvas_attrs);
	}

	// uniforms
	{
		Attrs unif_attrs = {
		        {"ub_plane_composition", &ub_plane_composition             },
		        {"u_reflectmap",         &m_canvas.getBuffer("color0").id()},
		};
		GsPainter::addUniforms(unif_attrs);
	}

	// drawcall
	{
		auto &drawcall = getADrawcall();
		drawcall.flags.cull_face = false;
		drawcall.flags.depth_test = true;
		drawcall.flags.blend = true;
	}

	// experimental
	{
		auto *current = attrs.get("canvas", GsCanvas::getCurrent());
		auto viewscreen = current->viewscreen();

		float far;
		viewscreen.get_projection(nullptr, nullptr, nullptr, &far);
		getRange() = Range3f(Vec3f(-far, 0, -far), Vec3f(+far, 0, +far));
	}
}
void Plane::set(const Attrs &attrs)
{
	m_mapworld = *attrs.get("mapworld", &m_mapworld);
	GsPainter::set(attrs);
}

void Plane::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new PlaneInspector(this);
	}
}

void Plane::update()
{
	if (!relatedNodes().empty()) {
		m_drawfunc.func = [&]() {
			for (auto &node: relatedNodes()) {
				auto is_flip = node->getProperty("flip");
				node->setProperty("flip", !is_flip);
				node->render();
				node->setProperty("flip", is_flip);
			}
		};
	}
	if (m_drawfunc.func) {
		auto *current = GsCanvas::getCurrent();
		if (calcScreenConvex(*current)) {
			m_canvas.takeover();
			auto c_save = Composition(m_canvas);
			reflect(m_canvas);
			m_canvas.begin();
			m_canvas.clear();
			m_drawfunc.func();
			m_canvas.end();
			*(Composition*)&m_canvas = c_save;
		}
	}
}

void Plane::doRender()
{
	auto map_scale = getADrawcall().ub_material.map_scale;  // NEED FIX

	auto *current = GsCanvas::getCurrent();
	auto &ub = ub_plane_composition;
	auto eye = current->viewworld(0).c[2];
	auto mapworld = m_mapworld.scale(1.0 / map_scale);

	m_plane = mapworld * Plane3f(ezero(), ez());

	if (dot(eye, m_plane.eq) < -cos(pi() / 6)) return;  // ad-hoc

	ub.worldview = current->worldview(0);
	ub.viewscreen = current->viewscreen();
	ub.screenview = ub.viewscreen.inverse();
	ub.viewmap = (current->worldview(0) * mapworld).inverse();
	ub.plane_view = (current->worldview(0) * m_plane).eq;

	if (calcScreenConvex(*current)) {
		drawConvex();
	}
}

void Plane::drawConvex()
{
	auto &points = m_convex.points();
	if (points.size() < 3) return; /* do nothing */

	SpuArray::send(points.data(), points.size());
	getADrawcall().coms[0].mode = GL_POLYGON;
	GsPainter::doRender();
}

bool Plane::calcScreenConvex(Composition &composition)
{
	auto plane_screen = composition.worldscreen(0) * m_plane;
	std::vector<Vec3f> inside_points;
	if (Mat4f().inside(plane_screen, &inside_points)) {
		m_convex = Convex2f(inside_points, true);
		return true;
	}
	return false;
}

void Plane::reflect(Composition &composition)
{
	auto screenfrag = Mat4f::texcfrag(composition.viewport(0)) * Mat4f::screentexc();
	auto range = Range3f(screenfrag.ortho3(m_convex.points()));
	auto reflect_viewport = Rectf(range);
	auto reflect_viewscreen = composition.viewscreen().shift(composition.viewport(0), reflect_viewport);
	auto reflect_worldview = composition.worldview(0) * getReflectMatrix(m_plane);

	composition.getWorldviews().front() = reflect_worldview;
	composition.getViewscreen() = reflect_viewscreen;
	composition.getViewports().at(0) = reflect_viewport;
	composition.getScissors().at(0) = reflect_viewport;
}

/*
plane:    nx * x + ny * y + nz * z + nw = 0
distance: D = nx * x0 + ny * y0 + nz * z0 + nw

v1 = v0 - D * n * 2

          | (nx * x0 + ny * y0 + nz * z0 + nw) * nx * 2 |
   = v0 - | (nx * x0 + ny * y0 + nz * z0 + nw) * ny * 2 |
          | (nx * x0 + ny * y0 + nz * z0 + nw) * nz * 2 |

          | nx*2    0    0 0 | | nx ny nz nw | |x0|
   = v0 - |    0 ny*2    0 0 | | nx ny nz nw | |y0|
          |    0    0 nz*2 0 | | nx ny nz nw | |z0|
          |    0    0    0 1 | |  0  0  0  1 | | 1|

     | 1 0 0 0 |   | nx*2    0    0 0 | | nx ny nz nw |
M  = | 0 1 0 0 | - |    0 ny*2    0 0 | | nx ny nz nw |
     | 0 0 1 0 |   |    0    0 nz*2 0 | | nx ny nz nw |
     | 0 0 0 1 |   |    0    0    0 1 | |  0  0  0  0 |
*/
Mat4f Plane::getReflectMatrix(const Plane3f &plane) const
{
	auto eq = plane.eq;
	auto E = Mat4f();
	auto A = Mat4f().scale(eq * 2);
	auto B = Mat4f(eq, eq, eq, ezero<Vec4f>()).transpose4();

	return E - A * B;
}
}  // namespace spu::gs_painter
