//
//$<<Header>>$
//

#pragma once

#include <images/image.hpp>

namespace spu::oglplus::images {

class Squares : public Image {
public:
	Squares(int32_t width, int32_t height, float ratio = 0.8F, int32_t xrep = 2, int32_t yrep = 2);
};

}  // namespace spu::oglplus::images
#include <images/squares.ipp>
