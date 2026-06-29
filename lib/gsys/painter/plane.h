//
// Plane :
//
#pragma once
#include <gsys/painter.h>
#include <gsys/painter/vertex.h>
#include <gsys/shaders/painter/plane/ub_plane_composition.us>
#include <smath/convex3f.h>

namespace spu::gs_painter {
class PlaneInspector;
class Plane : public GsPainter {
public:
	using Vertex = VertexP4;
	struct Drawfunc {
		std::function<void()> func = nullptr;
	};

	plane::UB_PLANE_COMPOSITION ub_plane_composition;

	Plane(const char *name = nullptr) : GsPainter(name) {}
	Plane(const Attrs &attrs) { init(attrs); }
	using GsObject::set;

	Drawfunc &getDrawfunc() { return m_drawfunc; }
	const Drawfunc &getDrawfunc() const { return m_drawfunc; }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void startInspector() override;
	void update() override;

	PAINTER_VERTEX_FUNCS;

protected:
	friend class PlaneInspector;
	Drawfunc m_drawfunc;
	Mat4f m_mapworld = Mat4f().rot("x", pi() / 2);
	Plane3f m_plane;
	Convex2f m_convex;
	GsCanvas m_canvas;

	bool calcScreenConvex(Composition &composition);
	void reflect(Composition &composition);
	void drawConvex();
	Mat4f getReflectMatrix(const Plane3f &plane) const;

	void doRender() override;
};
}  // namespace spu::gs_painter
