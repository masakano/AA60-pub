//
//$<<Header>>$
//

#pragma once

#include <smath/geometry.h>
#include <spu++/spu++.h>

namespace spu::oglplus::shapes {

struct Shape {
	struct DefaultTag {};
	struct WithAdjacencyTag {};
	struct QuadsTag {};
	struct PatchesTag {};
	struct EdgesTag {};

	uint32_t positions(std::vector<float> &) const { return 0; }
	uint32_t normals(std::vector<float> &) const { return 0; }
	uint32_t tangents(std::vector<float> &) const { return 0; }
	uint32_t bitangents(std::vector<float> &) const { return 0; }
	uint32_t texCoordinates(std::vector<float> &) const { return 0; }

	template<typename T> uint32_t materialNumbers(std::vector<T> &) const { return 0; }
};
}  // namespace spu::oglplus::shapes
