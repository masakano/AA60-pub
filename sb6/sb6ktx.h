//
//
//
#pragma once
#include <ssys/attrs.h>

namespace spu::sb6::ktx {

struct header {
	uint8_t identifier[12];
	uint32_t endianness;
	uint32_t gltype;
	uint32_t gltypesize;
	uint32_t glformat;
	uint32_t glinternalformat;
	uint32_t glbaseinternalformat;
	uint32_t pixelwidth;
	uint32_t pixelheight;
	uint32_t pixeldepth;
	uint32_t arrayelements;
	uint32_t faces;
	uint32_t miplevels;
	uint32_t keypairbytes;
};

union key_valuepair {
	uint32_t size;
	uint8_t rawbytes[4];
};
uint32_t load(const char *filename, const Attrs &aux_attrs = spu::Attrs());
}  // namespace spu::sb6::ktx
