//
//
//
#pragma once
#include "ecc8to64_codebook.h"  // 生成された CODE_TABLE をインクルード

namespace spu::libspu::video::ecc8to64 {

inline int popcount64(uint64_t x) { return __builtin_popcountll(static_cast<unsigned long long>(x)); }

inline int hamming_distance64(uint64_t a, uint64_t b) { return popcount64(a ^ b); }

/// 8bit → 64bit 符号化
inline uint64_t encode(uint8_t value) { return CODE_TABLE[value]; }

/// 64bit → 8bit 復号（最大尤度復号）
/// 戻り値: 復元された 8bit 値
/// out_dist が非 null の場合、最小ハミング距離を返す
inline uint8_t decode(uint64_t received, int* out_dist = nullptr)
{
	auto best_idx = 0;
	auto best_dist = 64;  // 上限

	for (auto i = 0; i < 256; ++i) {
		auto code = CODE_TABLE[i];
		auto d = hamming_distance64(received, code);
		if (d < best_dist) {
			best_dist = d;
			best_idx = i;
			if (best_dist == 0) break;  // 完全一致
		}
	}

	if (out_dist) {
		*out_dist = best_dist;
	}
	return best_idx;
}
}  // namespace spu::libspu::video::ecc8to64
