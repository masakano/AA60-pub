//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class Cube : public Shape {
private:
	double m_sx{1.0}, m_sy{1.0}, m_sz{1.0};
	double m_ox{0.0}, m_oy{0.0}, m_oz{0.0};

public:
	Cube()

	        = default;

	Cube(double w, double h, double d) : m_sx(w), m_sy(h), m_sz(d) {}

	Cube(double w, double h, double d, double x, double y, double z)
	        : m_sx(w), m_sy(h), m_sz(d), m_ox(x), m_oy(y), m_oz(z)
	{
	}

	uint32_t faceWinding() const { return GL_CW; }

	using VertexAttribFunc = uint32_t (Cube::*)(std::vector<float> &) const;

	uint32_t positions(std::vector<float> &dest) const;
	uint32_t normals(std::vector<float> &dest) const;
	uint32_t tangents(std::vector<float> &dest) const;
	uint32_t texCoordinates(std::vector<float> &dest) const;

	void boundingSphere(Sphere3f &bounding_sphere) const
	{
		bounding_sphere = Sphere3f(
		        Vec3f(m_ox, m_oy, m_oz), float(std::sqrt(m_sx * m_sx + m_sy * m_sy + m_sz * m_sz)));
	}

	using IndexArray = std::vector<uint16_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const { return IndexArray(); }

	uint16_t restartIndex() const { return 0xffff; }

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const;

	IndexArray indices(EdgesTag) const;

	std::vector<spu::SpuCommand> instructions(EdgesTag) const;
};

}  // namespace spu::oglplus::shapes
#include <shapes/cube.ipp>
