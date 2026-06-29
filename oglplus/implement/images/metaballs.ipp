
#include <lib/incl_begin.ipp>
#include <lib/incl_end.ipp>
// #include <math/angle.hpp>
#include <math/vector.hpp>

#include <cstdlib>

namespace spu {
namespace oglplus {
namespace images {

inline BaseMetaballs::BaseMetaballs(
        int32_t width, int32_t height, const float *balls, std::size_t size, std::size_t n)
        : Image(width, height, 1, 1, (float *)nullptr, GL_RED, u_int::R32F)
{
	assert(size % n == 0);

	const auto fc = FullCircle();
	auto a = this->begin<float>();

	for (auto y = 0; y != height; ++y) {
		const auto j = (y + 0.5f) / height;

		for (auto x = 0; x != width; ++x) {
			auto v = 0.0f;
			const auto i = (x + 0.5f) / width;
			const auto Vec2fap(i, j);

			for (auto b = 0; b != size; b += n) {
				const Vec2f c(balls + b, 2);

				for (auto yo = -1; yo != 2; ++yo)
					for (auto xo = -1; xo != 2; ++xo) {
						const Vec2f o(xo, yo);
						const Vec2f d = p - c + o;

						float r = balls[b + 2];

						if (n > 3) {
							float w = Arctan(d.y, d.x) / fc;
							w += balls[b + 2];
							w = sin(fc * w * balls[b + 3]);

							if (n > 4)
								r += balls[b + 4] * r * w;
							else
								r += 0.25f * r * w;
						}

						float t = (r * r / dot(d, d)) - 0.25;
						v += (t > 0.0f) ? t : 0.0f;
					}
			}
			assert(a != this->end<float>());
			*a++ = v;
		}
	}
	assert(a == this->end<float>());
}

std::vector<float> RandomMetaballs::_make_balls(std::size_t count, float rad_min, float rad_max)
{
	std::vector<float> result(count * 3);

	const float irm = 1.0f / RAND_MAX;
	const float rdirm = irm * (rad_max - rad_min);

	for (auto i = 0; i != count; ++i) {
		result[3 * i + 0] = std::rand() * irm;
		result[3 * i + 1] = std::rand() * irm;
		result[3 * i + 2] = rad_min + std::rand() * rdirm;
	}

	return std::move(result);
}

inline RandomMetaballs::RandomMetaballs(
        int32_t width, int32_t height, std::size_t count, float rad_min, float rad_max)
        : BaseMetaballs(width, height, _make_balls(count, rad_min, rad_max).data(), 3 * count, 3)
{
}

std::vector<float> RandomMetastars::_make_stars(
        std::size_t count, float rad_min, float rad_max, float dif_min, float dif_max, uint32_t ptc_min,
        uint32_t ptc_max)
{
	std::vector<float> result(count * 5);

	const float irm = 1.0f / RAND_MAX;
	const float rdirm = irm * (rad_max - rad_min);
	const float pdirm = irm * (ptc_max - ptc_min + 1);
	const float ddirm = irm * (dif_max - dif_min);

	for (auto i = 0; i != count; ++i) {
		result[5 * i + 0] = std::rand() * irm;
		result[5 * i + 1] = std::rand() * irm;
		result[5 * i + 2] = rad_min + std::rand() * rdirm;
		result[5 * i + 3] = ptc_min + std::rand() * pdirm;
		result[5 * i + 4] = dif_min + std::rand() * ddirm;
	}

	return std::move(result);
}

inline RandomMetastars::RandomMetastars(
        int32_t width, int32_t height, std::size_t count, float rad_min, float rad_max, float dif_min,
        float dif_max, uint32_t ptc_min, uint32_t ptc_max)
        : BaseMetaballs(
                  width, height,
                  _make_stars(count, rad_min, rad_max, dif_min, dif_max, ptc_min, ptc_max).data(), 5 * count, 5)
{
}

}  // namespace images
}  // namespace oglplus
}  // namespace spu
