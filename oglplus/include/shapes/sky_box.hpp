//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class SkyBox : public Shape {
private:
public:
	uint32_t faceWinding() const { return GL_CW; }

	using VertexAttribFunc = uint32_t (SkyBox::*)(std::vector<float> &) const;

	uint32_t positions(std::vector<float> &dest) const
	{
		const float positions[8 * 3] = {-1, -1, -1, +1, -1, -1, -1, +1, -1, +1, +1, -1,
		                                -1, -1, +1, +1, -1, +1, -1, +1, +1, +1, +1, +1};
		dest.assign(positions, positions + 8 * 3);
		return 3;
	}

	void boundingSphere(Sphere3f &bounding_sphere) const { bounding_sphere = Sphere3f(ezero(), 1.0); }

	using IndexArray = std::vector<uint16_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const
	{
		const uint16_t indices[6 * 5] = {1, 3, 5, 7, 9, 4, 6, 0, 2, 9, 2, 6, 3, 7, 9,
		                                 4, 0, 5, 1, 9, 5, 7, 4, 6, 9, 0, 2, 1, 3, 9};
		return IndexArray(indices, indices + 6 * 5);
	}

	uint16_t restartIndex() const { return 9; }  // tricky

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const
	{
		spu::SpuCommand com;
		com.target = GL_ELEMENT_ARRAY_BUFFER;
		com.mode = GL_TRIANGLE_STRIP;
		com.first = 0;
		com.count = 6 * 5;
		com.flags = 0;
		return {com};
	}
};

}  // namespace spu::oglplus::shapes
