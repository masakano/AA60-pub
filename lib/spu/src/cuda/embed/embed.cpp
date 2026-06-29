//
// Embed :
//
#include "embed.h"
#include "ecc8to64.h"

namespace spu::libspu::video {
namespace {

uint8_t clamp_u8(int32_t v) { return uint8_t(std::clamp(v, 0, 255)); }

float tile_mean_2x2(const uint8_t Y[16][16], int32_t y0, int32_t x0)
{
	auto sum = int32_t(Y[y0][x0]);
	sum += Y[y0][x0 + 1];
	sum += Y[y0 + 1][x0];
	sum += Y[y0 + 1][x0 + 1];
	return float(sum) * 0.25f;
}

}  // namespace

uint8_t Embed::rgba_to_y(RGBA rgba) const
{
	const auto r = float(rgba.r);
	const auto g = float(rgba.g);
	const auto b = float(rgba.b);

	const auto yf = 0.299f * r + 0.587f * g + 0.114f * b;
	return clamp_u8(int32_t(std::lround(yf)));
}

void Embed::rgb_to_cbcr(uint8_t r, uint8_t g, uint8_t b, uint8_t& Cb, uint8_t& Cr) const
{
	const auto rf = float(r);
	const auto gf = float(g);
	const auto bf = float(b);

	const auto cbf = -0.168736f * rf - 0.331264f * gf + 0.5f * bf + 128.0f;
	const auto crf = 0.5f * rf - 0.418688f * gf - 0.081312f * bf + 128.0f;

	Cb = clamp_u8(int32_t(std::lround(cbf)));
	Cr = clamp_u8(int32_t(std::lround(crf)));
}

void Embed::ycbcr_to_rgba(uint8_t Y, uint8_t Cb, uint8_t Cr, RGBA& rgba) const
{
	const auto alpha = rgba.a;

	const auto y = float(Y);
	const auto cb = float(Cb) - 128.0f;
	const auto cr = float(Cr) - 128.0f;

	const auto rf = y + 1.402f * cr;
	const auto gf = y - 0.344136f * cb - 0.714136f * cr;
	const auto bf = y + 1.772f * cb;

	const auto ri = int32_t(std::lround(rf));
	const auto gi = int32_t(std::lround(gf));
	const auto bi = int32_t(std::lround(bf));

	rgba.r = clamp_u8(ri);
	rgba.g = clamp_u8(gi);
	rgba.b = clamp_u8(bi);
	rgba.a = alpha;
}

void Embed::rgba_to_mb(const RGBA rgba[16 * 16], Macroblock& mb)
{
	for (auto y = 0; y < 16; ++y) {
		for (auto x = 0; x < 16; ++x) {
			mb.Y[y][x] = rgba_to_y(rgba[y * 16 + x]);
		}
	}

	for (auto by = 0; by < 8; ++by) {
		for (auto bx = 0; bx < 8; ++bx) {
			auto sum_r = 0;
			auto sum_g = 0;
			auto sum_b = 0;

			for (auto dy = 0; dy < 2; ++dy) {
				for (auto dx = 0; dx < 2; ++dx) {
					const auto src = rgba[(by * 2 + dy) * 16 + (bx * 2 + dx)];
					sum_r += src.r;
					sum_g += src.g;
					sum_b += src.b;
				}
			}

			const auto avg_r = uint8_t((sum_r + 2) / 4);
			const auto avg_g = uint8_t((sum_g + 2) / 4);
			const auto avg_b = uint8_t((sum_b + 2) / 4);
			rgb_to_cbcr(avg_r, avg_g, avg_b, mb.Cb[by][bx], mb.Cr[by][bx]);
		}
	}
}

void Embed::mb_to_rgba(const Macroblock& mb, RGBA rgba[16 * 16])
{
	for (auto by = 0; by < 8; ++by) {
		for (auto bx = 0; bx < 8; ++bx) {
			const auto cb = mb.Cb[by][bx];
			const auto cr = mb.Cr[by][bx];

			for (auto dy = 0; dy < 2; ++dy) {
				for (auto dx = 0; dx < 2; ++dx) {
					auto& dst = rgba[(by * 2 + dy) * 16 + (bx * 2 + dx)];
					ycbcr_to_rgba(mb.Y[by * 2 + dy][bx * 2 + dx], cb, cr, dst);
				}
			}
		}
	}
}

void Embed::embed_codeword_into_luma_block_shifted(uint8_t Y[16][16], uint64_t code, float mix) const
{
	mix = std::clamp(mix, 0.0f, 1.0f);

	for (auto tr = 0; tr < 8; ++tr) {
		for (auto tc = 0; tc < 8; ++tc) {
			const auto bit_idx = tr * 8 + tc;
			const auto y0 = tr * 2;
			const auto x0 = tc * 2;

			const auto bit = int32_t((code >> bit_idx) & 1ULL);
			const auto level = bit ? m_param.y_bright : m_param.y_dark;
			const auto mean = tile_mean_2x2(Y, y0, x0);
			const auto delta = mix * (float(level) - mean);

			for (auto dy = 0; dy < 2; ++dy) {
				for (auto dx = 0; dx < 2; ++dx) {
					const auto orig = float(Y[y0 + dy][x0 + dx]);
					const auto newYf = orig + delta;
					const auto newYi = int32_t(std::lround(newYf));
					Y[y0 + dy][x0 + dx] = clamp_u8(newYi);
				}
			}
		}
	}
}

uint8_t Embed::compute_adaptive_threshold(const uint8_t Y[16][16]) const
{
	float tile_means[64];
	auto minv = 255.0f;
	auto maxv = 0.0f;

	for (auto tr = 0; tr < 8; ++tr) {
		for (auto tc = 0; tc < 8; ++tc) {
			const auto idx = tr * 8 + tc;
			const auto y0 = tr * 2;
			const auto x0 = tc * 2;
			const auto mean = tile_mean_2x2(Y, y0, x0);
			tile_means[idx] = mean;
			minv = std::min(minv, mean);
			maxv = std::max(maxv, mean);
		}
	}

	auto m0 = minv;
	auto m1 = maxv;

	for (auto iter = 0; iter < 5; ++iter) {
		auto sum0 = 0.0f;
		auto sum1 = 0.0f;
		auto cnt0 = 0;
		auto cnt1 = 0;

		for (auto i = 0; i < 64; ++i) {
			const auto v = tile_means[i];
			if (std::fabs(v - m0) <= std::fabs(v - m1)) {
				sum0 += v;
				++cnt0;
			}
			else {
				sum1 += v;
				++cnt1;
			}
		}

		if (cnt0 > 0) m0 = sum0 / float(cnt0);
		if (cnt1 > 0) m1 = sum1 / float(cnt1);
	}

	if (m0 > m1) std::swap(m0, m1);

	auto th = 0.5f * (m0 + m1);
	th = std::clamp(th, 0.0f, 255.0f);

	return uint8_t(std::lround(th));
}

uint64_t Embed::extract_codeword_from_luma_block_adaptive(const uint8_t Y[16][16]) const
{
	auto code = uint64_t{0};
	const auto thr = compute_adaptive_threshold(Y);

	for (auto tr = 0; tr < 8; ++tr) {
		for (auto tc = 0; tc < 8; ++tc) {
			const auto bit_idx = tr * 8 + tc;
			const auto y0 = tr * 2;
			const auto x0 = tc * 2;
			const auto mean = tile_mean_2x2(Y, y0, x0);

			if (mean >= float(thr)) {
				code |= (uint64_t(1) << bit_idx);
			}
		}
	}

	return code;
}

bool Embed::embed_with_self_check(
        const uint8_t Y_in[16][16], uint8_t Y_out[16][16], uint8_t value, float& out_mix,
        int32_t& out_dist) const
{
	using namespace ecc8to64;

	const auto code = encode(value);
	auto best_mix = m_param.mix_end;
	auto best_dist = 64;
	auto has_best = false;
	uint8_t best_y[16][16];

	for (auto mix = m_param.mix_start; mix <= m_param.mix_end + 1e-6f; mix += m_param.mix_step) {
		uint8_t tmp_y[16][16];
		std::memcpy(tmp_y, Y_in, sizeof(tmp_y));

		embed_codeword_into_luma_block_shifted(tmp_y, code, mix);

		const auto extracted = extract_codeword_from_luma_block_adaptive(tmp_y);

		auto dist = 0;
		const auto decoded = decode(extracted, &dist);

		if (dist < best_dist) {
			best_dist = dist;
			best_mix = mix;
			std::memcpy(best_y, tmp_y, sizeof(tmp_y));
			has_best = true;
		}

		if (decoded == value && dist <= m_param.max_acceptable_dist) {
			std::memcpy(Y_out, tmp_y, sizeof(tmp_y));
			out_mix = mix;
			out_dist = dist;
			return true;
		}
	}

	if (!has_best) {
		std::memcpy(Y_out, Y_in, sizeof(best_y));
		out_mix = m_param.mix_start;
		out_dist = best_dist;
		return false;
	}

	std::memcpy(Y_out, best_y, sizeof(best_y));
	out_mix = best_mix;
	out_dist = best_dist;
	return false;
}

bool Embed::embed_byte_at_mb(Macroblock& mb, uint8_t value, float& out_mix, int32_t& out_dist)
{
	return embed_with_self_check(mb.Y, mb.Y, value, out_mix, out_dist);
}

uint8_t Embed::extract_byte_at_mb(const Macroblock& mb, int32_t& dist) const
{
	const auto code = extract_codeword_from_luma_block_adaptive(mb.Y);
	return ecc8to64::decode(code, &dist);
}
}  // namespace spu::libspu::video
