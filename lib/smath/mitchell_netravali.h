//
// MitchellNetravali : Mitchell-Netravali grid interpolation
//
#pragma once
#include "voxel.h"

namespace spu {
template<class T> class MitchellNetravali : public Voxels<T> {
public:
	using base_t = Voxels<T>;

	MitchellNetravali() : base_t() {}
	MitchellNetravali(int32_t x, int32_t y) { base_t::resize({x, y, 1, 1}); }

	void rescale(int32_t x, int32_t y, bool is_linear = false)
	{
		assert(base_t::m_size.z == 1 && base_t::m_size.w == 1);

		if (x == base_t::m_size.x && y == base_t::m_size.y) {
			return;
		}

		auto result = MitchellNetravali<T>(x, y);
		auto scale = Vec2f(base_t::m_size) / Vec2f(result.size());

		auto lerp4 = [&](float x, const T &y0, const T &y1, const T &y2, const T &y3) {
			auto c0 = mitchell_netravali_coef(-x - 1);
			auto c1 = mitchell_netravali_coef(-x + 0);
			auto c2 = mitchell_netravali_coef(-x + 1);
			auto c3 = mitchell_netravali_coef(-x + 2);
			return c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3;
		};

		for (auto y = 0; y < result.size().y; y++) {
			for (auto x = 0; x < result.size().x; x++) {
				auto p = Vec2f(x, y) * scale;

				auto ix = int32_t(p.x);
				auto iy = int32_t(p.y);

				auto dx = p.x - float(ix);
				auto dy = p.y - float(iy);

				auto p00 = base_t::geti({ix - 1, iy - 1, 0, 0});
				auto p01 = base_t::geti({ix + 0, iy - 1, 0, 0});
				auto p02 = base_t::geti({ix + 1, iy - 1, 0, 0});
				auto p03 = base_t::geti({ix + 2, iy - 1, 0, 0});

				auto p10 = base_t::geti({ix - 1, iy + 0, 0, 0});
				auto p11 = base_t::geti({ix + 0, iy + 0, 0, 0});
				auto p12 = base_t::geti({ix + 1, iy + 0, 0, 0});
				auto p13 = base_t::geti({ix + 2, iy + 0, 0, 0});

				auto p20 = base_t::geti({ix - 1, iy + 1, 0, 0});
				auto p21 = base_t::geti({ix + 0, iy + 1, 0, 0});
				auto p22 = base_t::geti({ix + 1, iy + 1, 0, 0});
				auto p23 = base_t::geti({ix + 2, iy + 1, 0, 0});

				auto p30 = base_t::geti({ix - 1, iy + 2, 0, 0});
				auto p31 = base_t::geti({ix + 0, iy + 2, 0, 0});
				auto p32 = base_t::geti({ix + 1, iy + 2, 0, 0});
				auto p33 = base_t::geti({ix + 2, iy + 2, 0, 0});

				if (is_linear) {
					auto p1 = lerp(p11, p12, dx);
					auto p2 = lerp(p21, p22, dx);
					result.at({x, y, 0, 0}) = lerp(p1, p2, dy);
					// result.at({x, y, 0, 0}) = p11;
				}
				else {
					auto p0 = lerp4(dx, p00, p01, p02, p03);
					auto p1 = lerp4(dx, p10, p11, p12, p13);
					auto p2 = lerp4(dx, p20, p21, p22, p23);
					auto p3 = lerp4(dx, p30, p31, p32, p33);

					result.at({x, y, 0, 0}) = lerp4(dy, p0, p1, p2, p3);
				}
			}
		}
		*this = result;
	}

private:
	static double mitchell_netravali_coef(double x)
	{
		const auto B = 1.0 / 3.0, C = 1.0 / 3.0;  // Recommended
		// const auto B = 1.0, C = 0.0;   // Cubic B-spline (smoother results)
		// const auto B = 0.0, C = 1/2.0; // Catmull-Rom spline (sharper results)
		x = std::abs(x);

		if (x < 1.0) {
			const auto a3 = (12.0 - 9.0 * B - 6.0 * C) / 6.0;
			const auto a2 = (-18.0 + 12.0 * B + 6.0 * C) / 6.0;
			const auto a1 = 0.0;
			const auto a0 = (6.0 - 2.0 * B) / 6.0;
			return a3 * x * x * x + a2 * x * x + a1 * x + a0;
		}
		if (x < 2.0) {
			const auto a3 = (-B - 6.0 * C) / 6.0;
			const auto a2 = (6.0 * B + 30.0 * C) / 6.0;
			const auto a1 = (-12.0 * B - 48.0 * C) / 6.0;
			const auto a0 = (8.0 * B + 24.0 * C) / 6.0;
			return a3 * x * x * x + a2 * x * x + a1 * x + a0;
		}
		return 0.0;
	}
};
}  // namespace spu
