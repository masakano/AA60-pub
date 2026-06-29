//
//$<<Header>>$
//

#pragma once

#include <images/image.hpp>

namespace spu::oglplus::images {

class BrushedMetalUByte : public Image {
private:
	static void _make_pixel(
	        uint8_t *b, const uint8_t *e, int32_t w, int32_t h, int32_t x, int32_t y, double /*c*/,
	        uint8_t r, uint8_t g);

	static void _make_scratch(
	        uint8_t *b, uint8_t *e, int32_t w, int32_t h, int32_t x, int32_t y, double dx, double dy);

public:
	BrushedMetalUByte(
	        int32_t width, int32_t height, uint32_t n_scratches, int32_t s_disp_min, int32_t s_disp_max,
	        int32_t t_disp_min, int32_t t_disp_max);
};

}  // namespace spu::oglplus::images
#include <images/brushed_metal.ipp>
