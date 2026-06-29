//
//$<<Header>>$
//

#pragma once

#include <images/image.hpp>

namespace spu::oglplus::images {

class CheckerRedBlack : public Image {
public:
	CheckerRedBlack(int32_t width, int32_t height, int32_t xrep = 4, int32_t yrep = 4);
};

}  // namespace spu::oglplus::images
#include <images/checker.ipp>
