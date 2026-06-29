//
// BlockDXT1 :
//
#pragma once

#include "color.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

namespace spu::dds {

struct BlockDXT1 {
	Color16 col0;
	Color16 col1;

	union {
		uint8_t row[4];
		uint32_t indices;
	};

	void flip()
	{
		std::swap(row[0], row[3]);
		std::swap(row[1], row[2]);
	}
};

struct AlphaBlockDXT3 {
	union {
		struct {
			unsigned int alpha0: 4;
			unsigned int alpha1: 4;
			unsigned int alpha2: 4;
			unsigned int alpha3: 4;
			unsigned int alpha4: 4;
			unsigned int alpha5: 4;
			unsigned int alpha6: 4;
			unsigned int alpha7: 4;
			unsigned int alpha8: 4;
			unsigned int alpha9: 4;
			unsigned int alphaA: 4;
			unsigned int alphaB: 4;
			unsigned int alphaC: 4;
			unsigned int alphaD: 4;
			unsigned int alphaE: 4;
			unsigned int alphaF: 4;
		};
		uint16_t row[4];
	};

	void flip()
	{
		std::swap(row[0], row[3]);
		std::swap(row[1], row[2]);
	}
};

struct BlockDXT3 {
	AlphaBlockDXT3 alpha;
	BlockDXT1 color;
};

struct AlphaBlockDXT5 {
	union {
		struct {
			unsigned int alpha0: 8;  // 8
			unsigned int alpha1: 8;  // 16
			unsigned int bits0 : 3;  // 3 - 19
			unsigned int bits1 : 3;  // 6 - 22
			unsigned int bits2 : 3;  // 9 - 25
			unsigned int bits3 : 3;  // 12 - 28
			unsigned int bits4 : 3;  // 15 - 31
			unsigned int bits5 : 3;  // 18 - 34
			unsigned int bits6 : 3;  // 21 - 37
			unsigned int bits7 : 3;  // 24 - 40
			unsigned int bits8 : 3;  // 27 - 43
			unsigned int bits9 : 3;  // 30 - 46
			unsigned int bitsA : 3;  // 33 - 49
			unsigned int bitsB : 3;  // 36 - 52
			unsigned int bitsC : 3;  // 39 - 55
			unsigned int bitsD : 3;  // 42 - 58
			unsigned int bitsE : 3;  // 45 - 61
			unsigned int bitsF : 3;  // 48 - 64
		};
		uint64_t u;

		struct {
			uint8_t t_alpha0;
			uint8_t t_alpha1;
			uint8_t row[6];
		};
	};

	void flip()
	{
		// std::swap does not work for bitfield..
		uint32_t t;

		t = bits0;
		bits0 = bitsC;
		bitsC = t;
		t = bits1;
		bits1 = bitsD;
		bitsD = t;
		t = bits2;
		bits2 = bitsE;
		bitsE = t;
		t = bits3;
		bits3 = bitsF;
		bitsF = t;

		t = bits4;
		bits4 = bits8;
		bits8 = t;
		t = bits5;
		bits5 = bits9;
		bits9 = t;
		t = bits6;
		bits6 = bitsA;
		bitsA = t;
		t = bits7;
		bits7 = bitsB;
		bitsB = t;
	}
};
// static_assert(sizeof(AlphaBlockDXT5) == 8);

struct BlockDXT5 {
	AlphaBlockDXT5 alpha;
	BlockDXT1 color;
};
}  // namespace spu::dds

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
