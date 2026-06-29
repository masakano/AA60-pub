//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class PointAndVector : public Shape {
private:
	double m_x{1}, m_y{0}, m_z{0};

public:
	PointAndVector() = default;

	PointAndVector(double x, double y, double z) : m_x(x), m_y(y), m_z(z) {}

	uint32_t faceWinding() const { return GL_CW; }

	using VertexAttribFunc = uint32_t (PointAndVector::*)(std::vector<float> &) const;

	uint32_t positions(std::vector<float> &dest) const
	{
		float positions[3] = {0, 0, 0};
		dest.assign(positions, positions + 3);
		return 3;
	}

	uint32_t normals(std::vector<float> &dest) const
	{
		float normals[3] = {float(m_x), float(m_y), float(m_z)};
		dest.assign(normals, normals + 3);
		return 3;
	}

	void boundingSphere(Sphere3f &bounding_sphere) const
	{
		bounding_sphere = Sphere3f(ezero(), float(std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z)));
	}

	using IndexArray = std::vector<uint16_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const { return IndexArray(); }

	uint16_t restartIndex() const { return 0xffff; }

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const
	{
		spu::SpuCommand com;
		com.target = GL_ARRAY_BUFFER;
		com.mode = GL_POINTS;
		com.first = 0;
		com.count = 1;
		com.flags = 0;

		return {com};
	}
};
}  // namespace spu::oglplus::shapes
