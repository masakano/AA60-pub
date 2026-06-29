//
// Color32 :
//
#pragma once
#include <spu/spu.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

namespace spu::dds {

class Color32 {
public:
	Color32() = default;

	Color32(uint8_t R, uint8_t G, uint8_t B, uint8_t A) : r(R), g(G), b(B), a(A) {}
	explicit Color32(uint32_t U) : u(U) {}

	operator uint32_t() const { return u; }

	// private:
	union {
		struct {
			uint8_t r: 8;
			uint8_t g: 8;
			uint8_t b: 8;
			uint8_t a: 8;
		};
		uint32_t u;
	};
};

class Color16 {
public:
	Color16() = default;
	Color16(const Color16 &c) : u(c.u) {}
	explicit Color16(uint16_t u) : u(u) {}
	operator uint16_t() const { return u; }

	// private:
	union {
		struct {
			uint16_t b: 5;
			uint16_t g: 6;
			uint16_t r: 5;
		};
		uint16_t u;
	};
};
}  // namespace spu::dds

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
