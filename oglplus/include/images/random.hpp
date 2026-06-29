//
//$<<Header>>$
//

#pragma once

#include <images/image.hpp>

namespace spu::oglplus::images {

class RandomRedUByte : public Image {
public:
	RandomRedUByte(int32_t width, int32_t height = 1, int32_t depth = 1);
};

class RandomRGBUByte : public Image {
public:
	RandomRGBUByte(int32_t width, int32_t height = 1, int32_t depth = 1);
};

}  // namespace spu::oglplus::images
#include <images/random.ipp>
