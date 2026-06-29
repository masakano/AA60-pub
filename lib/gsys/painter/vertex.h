//
// VertexP4 :
//
#pragma once
#include <smath/mesh.h>
namespace spu::gs_painter {

struct VertexP4 {
	Vec4f p;

	VertexP4() = default;

	explicit VertexP4(const Vec4f &p) : p(p) {}
	explicit VertexP4(const Mesh::Vertex &v) : VertexP4(Vec4f(v.p, 1)) {}
	operator Mesh::Vertex() const { return Mesh::Vertex(p); }
	operator Vec3f() const { return p; }
};

struct VertexP3N3T2 {
	float px, py, pz;
	float nx, ny, nz;
	float tx, ty;

	VertexP3N3T2() = default;

	VertexP3N3T2(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty) noexcept
	        : px(px), py(py), pz(pz), nx(nx), ny(ny), nz(nz), tx(tx), ty(ty)
	{
	}
	explicit VertexP3N3T2(const Mesh::Vertex &v) noexcept
	        : VertexP3N3T2(v.p.x, v.p.y, v.p.z, v.n.x, v.n.y, v.n.z, v.t.x, v.t.y)
	{
	}
	operator Mesh::Vertex() const { return Mesh::Vertex({px, py, pz}, {tx, ty, 1}, {nx, ny, nz}); }
	operator Vec3f() const { return {px, py, pz}; }
};

struct VertexP3T2N3 {
	float px, py, pz;
	float tx, ty;
	float nx, ny, nz;

	VertexP3T2N3() = default;

	VertexP3T2N3(float px, float py, float pz, float tx, float ty, float nx, float ny, float nz)
	        : px(px), py(py), pz(pz), tx(tx), ty(ty), nx(nx), ny(ny), nz(nz)
	{
	}
	explicit VertexP3T2N3(const Mesh::Vertex &v)
	        : VertexP3T2N3(v.p.x, v.p.y, v.p.z, v.t.x, v.t.y, v.n.x, v.n.y, v.n.z)
	{
	}
	operator Mesh::Vertex() const { return Mesh::Vertex({px, py, pz}, {tx, ty, 1}, {nx, ny, nz}); }
	operator Vec3f() const { return {px, py, pz}; }
};

}  // namespace spu::gs_painter
