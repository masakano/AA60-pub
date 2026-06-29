//
//$<<Header>>$
//

#pragma once

#include <array>
#include <smath/quatf.h>

namespace spu::oglplus {

namespace math {
template<typename T, typename P, int32_t N> struct Bezier {
private:
	static constexpr int32_t factorial(int32_t n) { return n > 0 ? n * factorial(n - 1) : 1; }

	static constexpr int32_t binormal(int32_t n, int32_t k)
	{
		return factorial(n) / (factorial(n - k) * factorial(k));
	}

	template<typename VT> static constexpr VT pow(VT v, int32_t n)
	{
		return n > 0 ? v * pow(v, n - 1) : VT(1);
	}

	static constexpr P bi(int32_t M, int32_t I, P t)
	{
		return binormal(M, I) * pow<P>(t, I) * pow<P>(P(1) - t, M - I);
	}

	static constexpr T f(int32_t D, int32_t I, const T *v, P t)
	{
		switch (D) {
		case 0: return bi(N - 0, I, t) * v[I];
		case 1: return bi(N - 1, I, t) * N * (v[I + 1] - v[I]);
		case 2: return bi(N - 2, I, t) * N * (N - 1) * (v[I + 2] - 2 * v[I + 1] + v[I]);
		default: assert(0);
		}
	}

	static constexpr T sum(int32_t D, int32_t I, const T *v, P t)
	{
		return I > 0 ? sum(D, I - 1, v, t) + f(D, I, v, t) : f(D, 0, v, t);
	}

public:
	static T b(int32_t D, const T *v, int32_t s, P t)
	{
		assert(s >= N);
		return sum(D, N - D, v, t);
	}

	static T position(const T *v, int32_t s, P t) { return b(0, v, s, t); }

	static T derivative1(const T *v, int32_t s, P t) { return b(1, v, s, t); }

	static T derivative2(const T *v, int32_t s, P t) { return b(2, v, s, t); }
};
}  // namespace math

template<typename Type, typename Parameter, uint32_t Order> class BezierCurves {
private:
	std::vector<Type> m_points;
	bool m_connected;

public:
	static bool is_connected(const std::vector<Type> &points) { return ((points.size() - 1) % Order) == 0; }

	bool is_connected() const { return m_connected; }

	static bool is_separated(const std::vector<Type> &points) { return (points.size() % (Order + 1)) == 0; }

	bool is_separated() const { return !m_connected; }

	static bool pointsOk(const std::vector<Type> &points)
	{
		if (points.empty()) {
			return false;
		}
		return (is_connected(points) || is_separated(points));
	}

	void init(const std::vector<Type> &points)
	{
		m_points = points;
		m_connected = is_connected(m_points);
		assert(pointsOk(m_points));
	}

	void init(const std::vector<Type> &points, bool connected)
	{
		m_points = points;
		m_connected = connected;

		assert(pointsOk(m_points));
		assert(is_connected(m_points) == m_connected);
	}

	uint32_t segmentStep() const
	{
		assert(pointsOk(m_points));
		if (m_connected) {
			return Order;
		}
		return Order + 1;
	}

	uint32_t segmentCount() const
	{
		assert(pointsOk(m_points));
		if (m_connected) {
			return (m_points.size() - 1) / Order;
		}
		return m_points.size() / (Order + 1);
	}

	const std::vector<Type> &controlPoints() const { return m_points; }

	static Parameter wrap(Parameter t)
	{
		const Parameter zero(0);
		const Parameter one(1);
		if (t < zero) {
			t += std::floor(std::fabs(t)) + one;
		}
		else if (t > one) {
			t -= std::floor(t);
		}
		assert(t >= zero && t <= one);
		return t;
	}

	Type position01(Parameter t) const
	{
		const Parameter zero(0);
		const Parameter one(1);

		if (t == one) {
			t = zero;
		}
		assert(t >= zero && t < one);

		Parameter toffs = t * segmentCount();

		uint32_t poffs = uint32_t(toffs) * segmentStep();

		assert(poffs < m_points.size() - Order);
		Parameter t_sub = toffs - std::floor(toffs);
		return math::Bezier<Type, Parameter, Order>::position(
		        m_points.data() + poffs, m_points.size() - poffs, t_sub);
	}

	Type position(Parameter t) const { return position01(wrap(t)); }

	void approximate(std::vector<Type> &dest, uint32_t n) const
	{
		uint32_t sstep = segmentStep();
		uint32_t s = segmentCount();

		dest.resize(s * n + 1);

		auto p = dest.begin();
		const Parameter t_step = Parameter(1) / n;

		for (auto i = 0u; i != s; ++i) {
			uint32_t poffs = i * sstep;
			auto t_sub = Parameter(0);
			const Type *data = m_points.data() + poffs;
			uint32_t size = m_points.size() - poffs;
			for (auto j = 0u; j != n; ++j) {
				using b = math::Bezier<Type, Parameter, Order>;
				assert(p != dest.end());
				*p = Type(b::position(data, size, t_sub));
				++p;

				t_sub += t_step;
			}
		}
		assert(p != dest.end());
		*p = m_points.back();
		++p;
		assert(p == dest.end());
	}

	std::vector<Type> approximate(uint32_t n) const
	{
		std::vector<Type> result;
		approximate(result, n);
		return result;
	}
};

template<typename Type, typename Parameter> class CubicBezierLoop : public BezierCurves<Type, Parameter, 3> {
private:
	template<typename StdRange> static std::vector<Type> make_cpoints(const StdRange &points, Parameter r)
	{
		std::size_t i = 0;
		std::size_t n = points.size();
		assert(n != 0);
		std::vector<Type> result(n * 3 + 1);
		auto ir = result.begin();
		while (i != n) {
			uint32_t a = (n + i - 1) % n;
			uint32_t b = i;
			uint32_t c = (i + 1) % n;
			uint32_t d = (i + 2) % n;
			assert(ir != result.end());
			*ir = points[b];
			++ir;
			assert(ir != result.end());
			*ir = Type(points[b] + (points[c] - points[a]) * r);
			++ir;
			assert(ir != result.end());
			*ir = Type(points[c] + (points[b] - points[d]) * r);
			++ir;
			++i;
		}
		assert(ir != result.end());
		*ir = points[0];
		++ir;
		assert(ir == result.end());
		return result;
	}

public:
	void init(const std::vector<Type> &points, Parameter r = Parameter(1) / Parameter(3))
	{
		BezierCurves<Type, Parameter, 3>::init(make_cpoints(points, r));
	}
};

}  // namespace spu::oglplus
