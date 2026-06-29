//
// Pointset :
//
#pragma once

#include <gsys/painter.h>
#include <gsys/painter/vertex.h>

namespace spu::gs_painter {

class Pointset : public GsPainter {
public:
	using Vertex = VertexP4;
	/*
	struct Vertex {
	        Vec4f p;

	        Vertex() = default;

	        explicit Vertex(const Vec4f &p) : p(p) {}
	        explicit Vertex(const Mesh::Vertex &v) : Vertex(Vec4f(v.p, 1)) {}
	        operator Mesh::Vertex() const noexcept { return Mesh::Vertex(p); }
	};
	*/
	static constexpr int32_t def_local_size_x = 256;

	explicit Pointset(const char *name = nullptr) : GsPainter(name) {}
	explicit Pointset(const Attrs &attrs) : Pointset() { init(attrs); }

	void init(const Attrs &attrs) override;
	void startInspector() override;

	Mat4f u_viewfrag;
	Mat4f u_viewworld;
	Vec3f u_rayorg = ezero();
	Vec3f u_raydir = ezero();
	uint32_t u_j = 0;
	uint32_t u_k = 0;

	PAINTER_VERTEX_FUNCS;

protected:
	SpuComputeArray m_sortArray;
	bool m_isAutoSort = false;
	bool m_isParticleset = false;
	uint32_t m_nelem = 0;
	uint32_t m_nelem2 = 0;

	void doUse(uint32_t id) override;
	void doRender() override;
	void gpuSort();
};
}  // namespace spu::gs_painter
