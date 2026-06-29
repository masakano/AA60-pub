//
// Embed :
//
#pragma once
#include <ssys/ssys.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#endif

namespace spu::libspu::video {

class Embed {
public:
	union RGBA {
		uint32_t ui32;
		struct {
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};
	};

	struct Macroblock {
		uint8_t Y[16][16];
		uint8_t Cb[8][8];
		uint8_t Cr[8][8];
	};

	struct Param {
		// 0/1 bit 用輝度
		uint8_t y_dark = 40;
		uint8_t y_bright = 215;

		// しきい値（フォールバック用。現状は adaptive が主なので未使用でもOK）
		uint8_t y_thresh = 128;

		// mix 探索パラメータ
		float mix_start = 0.25f;
		float mix_end = 1.0f;
		float mix_step = 0.05f;

		// 自己チェック時に安全とみなす距離
		int32_t max_acceptable_dist = 8;
	};

	Param m_param{};

	bool embed_byte_at_mb(Macroblock& mb, uint8_t value, float& out_mix, int32_t& out_dist);
	uint8_t extract_byte_at_mb(const Macroblock& mb, int32_t& dist) const;
	void rgba_to_mb(const RGBA rgba[16 * 16], Macroblock& mb);
	void mb_to_rgba(const Macroblock& mb, RGBA rgba[16 * 16]);

	void embed_byte_at_rgba(RGBA rgba[16 * 16], uint8_t value)
	{
		Macroblock mb;
		rgba_to_mb(rgba, mb);

		float mix = 0.0f;
		int dist = 0;
		[[maybe_unused]] auto is_embedded = embed_byte_at_mb(mb, value, mix, dist);
		assert(is_embedded);
		mb_to_rgba(mb, rgba);
	}

	uint8_t extract_byte_at_rgba(RGBA rgba[16 * 16])
	{
		Macroblock mb;
		rgba_to_mb(rgba, mb);
		int32_t dist = 0;
		return extract_byte_at_mb(mb, dist);
	}

private:
	uint8_t rgba_to_y(RGBA rgba) const;
	void rgb_to_cbcr(uint8_t r, uint8_t g, uint8_t b, uint8_t& Cb, uint8_t& Cr) const;
	void ycbcr_to_rgba(uint8_t Y, uint8_t Cb, uint8_t Cr, RGBA& rgba) const;
	void embed_codeword_into_luma_block_shifted(uint8_t Y[16][16], uint64_t code, float mix) const;
	uint8_t compute_adaptive_threshold(const uint8_t Y[16][16]) const;
	uint64_t extract_codeword_from_luma_block_adaptive(const uint8_t Y[16][16]) const;
	bool embed_with_self_check(
	        const uint8_t Y_in[16][16], uint8_t Y_out[16][16], uint8_t value, float& out_mix,
	        int32_t& out_dist) const;
};
}  // namespace spu::libspu::video

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
