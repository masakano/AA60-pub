//
// Gathered uniform source preprocessing :
//
#pragma once
#include <ssys/attrs.h>
#include <cstdint>
#include <string>
#include <vector>

namespace spu::libspu::spu_shader {

struct GatheredUniformSourceDesc {
	std::string symbol;
	uint32_t type = 0;
	uint32_t nelem = 1;
	uint32_t offset = 0;
	uint32_t size = 0;
	uint32_t stride = 0;
};

struct GatheredUniformSourceResult {
	std::string block_symbol = "UB_GATHERED";
	uint32_t block_size = 0;
	std::vector<GatheredUniformSourceDesc> uniforms;

	bool empty() const { return uniforms.empty(); }
};

void preprocessGatheredUniformSources(Attrs &attrs, GatheredUniformSourceResult &result);

}  // namespace spu::libspu::spu_shader
