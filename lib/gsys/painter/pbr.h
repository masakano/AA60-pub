//
// PBR :
//
#pragma once
#include <gsys/painter.h>
#include <gsys/painter/vertex.h>

namespace spu::gs_painter {
class PBRInspector;
class PBR : public GsPainter {
public:
	using Vertex = VertexP3N3T2;
	bool def_pbr_debug = false;

	explicit PBR(const char *name = nullptr) : GsPainter(name) {}
	explicit PBR(const Attrs &attrs) : PBR() { init(attrs); }

	void init(const Attrs &attrs) override;
	void startInspector() override;

	PAINTER_VERTEX_FUNCS;

protected:
	friend class PBRInspector;
};
}  // namespace spu::gs_painter
