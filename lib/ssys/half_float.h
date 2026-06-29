//
// HalfFloat : 16bit float
//
#pragma once
#include "ssys.h"
#include <cfenv>

namespace spu {

class HalfFloat {
public:
	HalfFloat() noexcept = default;
	explicit HalfFloat(uint16_t h) noexcept : h(h) {}
	explicit HalfFloat(float f) { h = to_half(f); }

	explicit operator uint16_t() const noexcept { return h; }
	explicit operator float() const noexcept { return from_half(h); }

private:
	uint16_t h = 0;
	uint16_t to_half(float value) const;
	float from_half(uint16_t value) const;
};
}  // namespace spu
