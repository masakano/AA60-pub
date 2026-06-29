//
//$<<Header>>$
//

#pragma once

#include <images/image.hpp>

namespace spu::oglplus::images {

class SortNWMap : public Image {
private:
	using T = uint16_t;
	static uint32_t pot(uint32_t n);
	static uint32_t next_log(uint32_t n);
	static uint32_t num_steps(uint32_t size);

public:
	SortNWMap(uint32_t size);
};

}  // namespace spu::oglplus::images
#include <images/sort_nw.ipp>
