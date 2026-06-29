//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>
#include <map>

namespace spu::oglplus::shapes {

enum struct SubdivSphereInitialShape {
	Icosahedron = 0,
	Octohedron = 1,
	Tetrahedron = 2,
};

class SimpleSubdivSphere : public Shape {
private:
	uint32_t m_subdivs{2};
	std::vector<float> m_positions;
	std::vector<uint32_t> m_indices;

	using Edge = std::pair<uint32_t, uint32_t>;
	std::map<Edge, uint32_t> m_midpoints;

	uint32_t midpoint(uint32_t ia, uint32_t ib);
	void subdivide(uint32_t ia, uint32_t ib, uint32_t ic, uint32_t levels);
	void make_face(uint32_t ia, uint32_t ib, uint32_t ic, uint32_t levels);

	void init_icosah();
	void init_tetrah();
	void init_octoh();

public:
	using InitialShape = SubdivSphereInitialShape;

	SimpleSubdivSphere() { init_icosah(); }

	SimpleSubdivSphere(uint32_t subdivs) : m_subdivs(subdivs) { init_icosah(); }

	SimpleSubdivSphere(uint32_t subdivs, InitialShape init_shape);

	uint32_t faceWinding() const { return GL_CCW; }

	using VertexAttribFunc = uint32_t (SimpleSubdivSphere::*)(std::vector<float> &) const;

	uint32_t positions(std::vector<float> &dest) const
	{
		dest.assign(m_positions.begin(), m_positions.end());
		return 3;
	}

	void boundingSphere(Sphere3f &bounding_sphere) const { bounding_sphere = Sphere3f(ezero(), 1.0); }

	using IndexArray = std::vector<uint32_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const
	{
		return IndexArray(m_indices.begin(), m_indices.end());
	}

	uint32_t restartIndex() const { return 0xfffffffe; }

	std::vector<spu::SpuCommand> instructions(uint32_t mode) const;

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const
	{
		return instructions(GL_TRIANGLES);
	}
};

}  // namespace spu::oglplus::shapes
#include <shapes/subdiv_sphere.ipp>
