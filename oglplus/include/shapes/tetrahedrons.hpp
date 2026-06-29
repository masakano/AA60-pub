//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class Tetrahedrons : public Shape {
private:
	double m_side{1.0};
	uint32_t m_divisions{10};

public:
	Tetrahedrons()

	        = default;

	Tetrahedrons(double side, uint32_t divisions) : m_side(side), m_divisions(divisions)
	{
		assert(m_side > 0.0);
		assert(divisions > 0);
	}

	uint32_t faceWinding() const { return GL_CW; }

	uint32_t positions(std::vector<float> &dest) const
	{
		const uint32_t n = m_divisions + 1;
		uint32_t k = 0;
		dest.resize(n * n * n * 3 + 3);

		dest[k++] = float(0);
		dest[k++] = float(0);
		dest[k++] = float(0);

		double step = m_side / m_divisions;
		for (auto z = 0u; z != n; ++z) {
			for (auto y = 0u; y != n; ++y) {
				for (auto x = 0u; x != n; ++x) {
					dest[k++] = float(x * step);
					dest[k++] = float(y * step);
					dest[k++] = float(z * step);
				}
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t texCoordinates(std::vector<float> &dest) const
	{
		const uint32_t n = m_divisions + 1;
		uint32_t k = 0;
		dest.resize(n * n * n * 3 + 3);

		dest[k++] = float(0);
		dest[k++] = float(0);
		dest[k++] = float(0);

		double step = 1.0 / m_divisions;
		for (auto z = 0u; z != n; ++z) {
			for (auto y = 0u; y != n; ++y) {
				for (auto x = 0u; x != n; ++x) {
					dest[k++] = float(x * step);
					dest[k++] = float(y * step);
					dest[k++] = float(z * step);
				}
			}
		}
		assert(k == dest.size());
		return 3;
	}

	void boundingSphere(Sphere3f &) const {}

	using IndexArray = std::vector<uint32_t>;

	IndexArray indices(WithAdjacencyTag = WithAdjacencyTag()) const;

	uint32_t restartIndex() const { return 0xfffffffe; }

	std::vector<spu::SpuCommand> instructions(WithAdjacencyTag = WithAdjacencyTag()) const;
};

}  // namespace spu::oglplus::shapes
#include <shapes/tetrahedrons.ipp>
