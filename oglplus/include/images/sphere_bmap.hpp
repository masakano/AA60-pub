//
//$<<Header>>$
//

#pragma once

#include <images/image.hpp>

namespace spu::oglplus::images {

class SphereBumpMap : public Image {
public:
	SphereBumpMap(int32_t width, int32_t height, int32_t xrep = 1, int32_t yrep = 1);
};

}  // namespace spu::oglplus::images
#include <images/sphere_bmap.ipp>
