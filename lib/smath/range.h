//
// Range :
//
#pragma once
#include "vec.h"

namespace spu {

template<class T, uint32_t MASK> class Range {
public:
	T p0;  // min
	T p1;  // max

	Range() = default;
	Range(const T &p0, const T &p1) noexcept : p0(p0), p1(p1) {}

	template<class T0, uint32_t MASK0>
	Range(const Range<T0, MASK0> &range) noexcept : p0(range.p0), p1(range.p1)
	{
	}
	template<class T0> explicit Range(const std::vector<T0> &points)
	{
		invalidate();
		for (auto &point: points) {
			p0 = min(p0, T(point));
			p1 = max(p1, T(point));
		}
	}

	T span() const { return p1 - p0; }
	T center() const { return (p0 + p1) * T(0.5); }
	void invalidate()
	{
		p0 = T(+huge());
		p1 = T(-huge());
	}

	void expand(const T &point)
	{
		p0 = min(p0, point);
		p1 = max(p1, point);
	}

	template<class T0> void expand(const std::vector<T0> &points)
	{
		for (auto &point: points) {
			p0 = min(p0, T(point));
			p1 = max(p1, T(point));
		}
	}
	void expand(const Range &range)
	{
		p0 = min(p0, range.p0);
		p1 = max(p1, range.p1);
	}

	void shrink(const Range &range)
	{
		p0 = max(p0, range.p0);
		p1 = min(p1, range.p1);
		p1 = max(p0, p1);
	}
#if 1
	void grow(const T &r)
	{
		auto c = center();
		auto s = span();
		p0 = c - s * r * 0.5f;
		p1 = c + s * r * 0.5f;
	}

#else
	Range grow(const T &s) const
	{
		auto c = center();
		auto d0 = max(T((c - p0) * s), T(epsilon()));
		auto d1 = max(T((p1 - c) * s), T(epsilon()));
		return {c - d0, c + d1};
	}
#endif
	void report(const char *str) const
	{
		if (str && *str) aux_printf("%s:\n", str);
		p0.report("p0");
		p1.report("p1");
	}
	bool valid() const { return ((p1 >= p0).pack() & MASK) == MASK; }

	bool inside(const T &point) const { return (pack((p0 <= point) & (point <= p1)) & MASK) == MASK; }

	template<class T0> std::vector<T> inside(const std::vector<T0> &points) const
	{
		std::vector<T0> inside_points;
		for (auto &p: points) {
			if (inside(p)) {
				inside_points.push_back(p);
			}
		}
		return inside_points;
	}

	float distance(const T &point, [[maybe_unused]] T *nearest = nullptr) const
	{
		assert(nearest == nullptr);
		float d2 = 0;
		for (auto &p: points()) {
			d2 = std::max(d2, std::abs(dot(point, p)));
		}
		return sqrtf(d2);
	}
	bool intersect(const Range &range) const
	{
		auto flag = (p1 < range.p0) | (range.p1 < p0);
		return pack(flag) == 0;
	}

	float area(void) const
	{
		if (valid()) {
			auto d = p1 - p0;
			return (d.x * d.y + d.y * d.z + d.z * d.x) * 2.0f;
		}
		return 0.0;
	}

	std::vector<T> points() const;

	friend Range operator+(const Range &src, const T &t) { return {src.p0 + t, src.p1 + t}; }
	friend Range operator-(const Range &src, const T &t) { return {src.p0 - t, src.p1 - t}; }
	friend Range operator*(const Range &src, const T &s) { return {src.p0 * s, src.p1 * s}; }

private:
	int32_t pack(const Vec4i &det) const { return det.pack(); }
	int32_t pack(bool det) const { return det ? MASK : 0; }
};

template<> inline std::vector<Vec2f> Range<Vec2f, 0x0ff>::points() const
{
	// quad order
	return {
	        {p0.x, p0.y},
	        {p1.x, p0.y},
	        {p1.x, p1.y},
	        {p0.x, p1.y},
	};
}

template<> inline std::vector<Vec3f> Range<Vec3f, 0xfff>::points() const
{
	// 2 quad order
	return {
	        {p0.x, p0.y, p0.z},
                {p1.x, p0.y, p0.z},
                {p1.x, p1.y, p0.z},
                {p0.x, p1.y, p0.z},

	        {p0.x, p0.y, p1.z},
                {p1.x, p0.y, p1.z},
                {p1.x, p1.y, p1.z},
                {p0.x, p1.y, p1.z},
	};
}

using Range1f = Range<float, 0x000f>;
using Range2f = Range<Vec2f, 0x00ff>;
using Range3f = Range<Vec3f, 0x0fff>;

/// 2D rectangle
template<class vec_t> struct Rect : public vec_t {
	using scalar_t = decltype(vec_t::x);

	Rect() noexcept : vec_t(0) {}
	Rect(scalar_t ox, scalar_t oy, scalar_t sx, scalar_t sy) noexcept : vec_t(ox, oy, sx, sy) {}
	Rect(const vec4f_t &v4) noexcept : vec_t(v4) {}
	Rect(const Range2f &range) : vec_t(range.p0.x, range.p0.y, range.span().x, range.span().y) {}

	template<class T> explicit Rect(const Rect<T> &v4) noexcept : vec_t(v4) {}

	operator Range2f() const
	{
		const float x0 = vec_t::ox, y0 = vec_t::oy;
		const float x1 = vec_t::ox + vec_t::sx, y1 = vec_t::oy + vec_t::sy;

		return {
		        {x0, y0},
		        {x1, y1},
		};
	}

	bool offset() const noexcept { return vec_t::ox != 0 || vec_t::oy != 0; }

	std::vector<Vec2f> points() const
	{
		const float x0 = vec_t::ox, y0 = vec_t::oy;
		const float x1 = vec_t::ox + vec_t::sx, y1 = vec_t::oy + vec_t::sy;

		// CCW order
		return {
		        {x0, y0},
                        {x1, y0},
                        {x1, y1},
                        {x0, y1}
                };
	}
};
using Recti = Rect<Vec4i>;
using Rectf = Rect<Vec4f>;
}  // namespace spu
