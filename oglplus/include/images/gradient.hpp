//
//$<<Header>>$
//

#pragma once

#include <images/image.hpp>

#include <map>

namespace spu::oglplus::images {

class LinearGradient : public Image {
private:
	// template <typename P, std::size_t N>
	template<typename P, typename Vec>
	static void make_gradient(
	        const Vec &background, int32_t dimension, const std::map<P, Vec> &points,
	        std::vector<Vec> &gradient)
	{
		auto point_cur = points.begin();
		auto point_end = points.end();
		auto grad_cur = gradient.begin();
		auto grad_end = gradient.end();

		const P step = P(1) / P(dimension);

		P prev_point = P(0);
		P curr_point = P(0);
		P next_point = P(1);

		Vec prev_color = background;
		Vec next_color = background;

		for (auto p = 0; p != dimension; ++p) {
			if (point_cur != point_end) {
				if (point_cur->first <= curr_point) {
					prev_point = point_cur->first;
					auto npp = point_cur;
					++npp;
					if (npp != point_end) {
						prev_color = point_cur->second;
						next_point = npp->first;
						next_color = npp->second;
						point_cur = npp;
					}
					else {
						prev_color = background;
						next_point = P(1);
						next_color = background;
					}
				}
				else {
					next_point = point_cur->first;
				}
			}
			P factor = P(0);
			if (prev_point != next_point) {
				factor = (curr_point - prev_point) / (next_point - prev_point);
			}

			assert(grad_cur != grad_end);
			*grad_cur = prev_color * (P(1) - factor) + next_color * factor;
			++grad_cur;

			curr_point += step;
		}
		assert(grad_cur == grad_end);
	}

	static uint8_t cc_max() { return ~u_char(0); }

	template<typename T> static T clamp(T v)
	{
		if (v < T(0)) {
			return T(0);
		}
		if (v > T(1)) {
			return T(1);
		}
		return v;
	}

	template<typename Vec>
	static void apply_gradient(const std::vector<Vec> &grad0, uint8_t *dp, const uint8_t *de)
	{
		auto gb0 = grad0.begin();
		auto ge0 = grad0.end();

		for (auto gp0 = gb0; gp0 != ge0; ++gp0) {
			Vec color = *gp0;
			for (auto c = 0; c != color.size(); ++c) {
				assert(dp != de);
				*dp++ = u_char(clamp(color.f[c]) * cc_max());
			}
		}
		assert(dp == de);
	}

	template<typename Combine, typename Vec>
	static void apply_gradient(
	        Combine combine, const std::vector<Vec> &grad0, const std::vector<Vec> &grad1, uint8_t *dp,
	        const uint8_t *de)
	{
		auto gb0 = grad0.begin();
		auto ge0 = grad0.end();
		auto gb1 = grad1.begin();
		auto ge1 = grad1.end();

		for (auto gp0 = gb0; gp0 != ge0; ++gp0) {
			for (auto gp1 = gb1; gp1 != ge1; ++gp1) {
				Vec color = combine(*gp0, *gp1);
				for (std::size_t c = 0; c != color.size(); ++c) {
					assert(dp != de);
					*dp++ = u_char(m_clamp(color.f[c]) * cc_max());
				}
			}
		}
		assert(dp == de);
	}

	template<typename Combine, typename Vec>
	static void apply_gradient(
	        Combine combine, const std::vector<Vec> &grad0, const std::vector<Vec> &grad1,
	        const std::vector<Vec> &grad2, uint8_t *dp, const uint8_t *de)
	{
		auto gb0 = grad0.begin();
		auto ge0 = grad0.end();
		auto gb1 = grad1.begin();
		auto ge1 = grad1.end();
		auto gb2 = grad2.begin();
		auto ge2 = grad2.end();

		for (auto gp0 = gb0; gp0 != ge0; ++gp0) {
			for (auto gp1 = gb1; gp1 != ge1; ++gp1) {
				for (auto gp2 = gb2; gp2 != ge2; ++gp2) {
					Vec color = combine(*gp0, *gp1, *gp2);
					for (std::size_t c = 0; c != color.size(); ++c) {
						assert(dp != de);
						*dp++ = u_char(clamp(color.f[c]) * cc_max());
					}
				}
			}
		}
		assert(dp == de);
	}

public:
	struct AddComponents {
		template<typename Vec> Vec operator()(const Vec &a, const Vec &b) const { return a + b; }

		template<typename Vec> Vec operator()(const Vec &a, const Vec &b, const Vec &c) const
		{
			return a + b + c;
		}
	};

	template<typename P, typename Vec>
	LinearGradient(int32_t width, const Vec &background, const std::map<P, Vec> &x_points)
	        : Image(width, 1, 1, Vec::size(), static_cast<uint8_t *>(nullptr))
	{
		std::vector<Vec> x_gradient(width);
		make_gradient(background, width, x_points, x_gradient);
		apply_gradient(x_gradient, begin(), end());
	}

	template<typename P, typename Vec, typename Combine>
	LinearGradient(
	        int32_t width, int32_t height, const Vec &background, const std::map<P, Vec> &x_points,
	        const std::map<P, Vec> &y_points, Combine combine)
	        : Image(width, height, 1, Vec::size(), static_cast<uint8_t *>(nullptr))
	{
		std::vector<Vec> y_gradient(height);
		make_gradient(background, height, y_points, y_gradient);

		std::vector<Vec> x_gradient(width);
		make_gradient(background, width, x_points, x_gradient);

		apply_gradient(combine, y_gradient, x_gradient, begin(), end());
	}

	template<typename P, typename Vec, typename Combine>
	LinearGradient(
	        int32_t width, int32_t height, int32_t depth, const Vec &background,
	        const std::map<P, Vec> &x_points, const std::map<P, Vec> &y_points,
	        const std::map<P, Vec> &z_points, Combine combine)
	        : Image(width, height, depth, Vec::size(), static_cast<uint8_t *>(nullptr))
	{
		std::vector<Vec> z_gradient(depth);
		make_gradient(background, depth, z_points, z_gradient);

		std::vector<Vec> y_gradient(height);
		make_gradient(background, height, y_points, y_gradient);

		std::vector<Vec> x_gradient(width);
		make_gradient(background, width, x_points, x_gradient);

		apply_gradient(combine, z_gradient, y_gradient, x_gradient, begin(), end());
	}
};

}  // namespace spu::oglplus::images
