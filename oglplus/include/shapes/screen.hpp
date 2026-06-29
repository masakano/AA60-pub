//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class Screen : public Shape {
public:
	uint32_t faceWinding() const { return GL_CW; }

	uint32_t normals(std::vector<float> &dest) const
	{
		auto k = 0u;
		dest.resize(12);

		for (auto i = 0u; i != 4; ++i) {
			dest[k++] = float(0);
			dest[k++] = float(0);
			dest[k++] = float(1);
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t tangents(std::vector<float> &dest) const
	{
		auto k = 0u;
		dest.resize(12);

		for (auto i = 0u; i != 4; ++i) {
			dest[k++] = float(1);
			dest[k++] = float(0);
			dest[k++] = float(0);
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t bitangents(std::vector<float> &dest) const
	{
		auto k = 0u;
		dest.resize(12);

		for (auto i = 0u; i != 4; ++i) {
			dest[k++] = float(0);
			dest[k++] = float(1);
			dest[k++] = float(0);
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t positions(std::vector<float> &dest) const
	{
		auto k = 0u;
		dest.resize(8);

		dest[k++] = float(-1);
		dest[k++] = float(-1);

		dest[k++] = float(-1);
		dest[k++] = float(+1);

		dest[k++] = float(+1);
		dest[k++] = float(-1);

		dest[k++] = float(+1);
		dest[k++] = float(+1);

		assert(k == dest.size());
		return 2;
	}

	uint32_t texCoordinates(std::vector<float> &dest) const
	{
		auto k = 0u;
		dest.resize(8);

		dest[k++] = float(0);
		dest[k++] = float(0);

		dest[k++] = float(0);
		dest[k++] = float(1);

		dest[k++] = float(1);
		dest[k++] = float(0);

		dest[k++] = float(1);
		dest[k++] = float(1);

		assert(k == dest.size());
		return 2;
	}

	void boundingSphere(Sphere3f &bounding_sphere) const { bounding_sphere = Sphere3f(ezero(), float(1)); }

	using IndexArray = std::vector<uint32_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const { return IndexArray(); }

	uint32_t restartIndex() const { return 0xfffffffe; }  // not -1 (represents no restart)

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const
	{
		spu::SpuCommand com;
		com.target = GL_ARRAY_BUFFER;
		com.mode = GL_TRIANGLE_STRIP;
		com.first = uint32_t(0);
		com.count = 4;
		com.flags = 0;

		return {com};
	}
};

}  // namespace spu::oglplus::shapes
