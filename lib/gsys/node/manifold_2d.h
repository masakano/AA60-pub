//
// Manifold2D :
//
#pragma once
#include <gsys/node.h>
#include <smath/mesh.h>

namespace spu {
class Mesh;
}

namespace spu::gs_node {

class Manifold2D : public GsNode {
public:
	using modifier_t = std::function<Mesh::Vertex(const Vec2f &, const Mat4f &, const Mat4f &)>;

	explicit Manifold2D(const char *name = nullptr) : GsNode(name) {}
	explicit Manifold2D(const Attrs &attrs) : Manifold2D() { init(attrs); }

	void init(const Attrs &attrs) override;
	void replacePainter(GsPainter *painter) override;
	void createVertices(modifier_t modifier, const Mat4f &mapnode, const Mat4f &maptexc);

protected:
	Vec4i m_meshGrid;
	Attrs m_attrs;
	Mesh m_mesh;

	void createIndices(int32_t patch_vertices, int32_t step);
	modifier_t getDefaultModifier(const std::string &shape);
};
}  // namespace spu::gs_node
