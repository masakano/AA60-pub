//
// LinearCurve :
//
#pragma once
// #include "range.h"
#include "mat4f.h"
#include "quatf.h"

namespace spu {

/// 1D B-spline interpolation
template<class key_t, class value_t> class LinearCurve {
public:
	using key_value_t = std::pair<key_t, value_t>;

	LinearCurve() = default;
	LinearCurve(std::vector<key_value_t> &key_values) { init(key_values); }

	void init(std::vector<key_value_t> &key_values)
	{
		auto key_values_soa = vector_sort_and_separate(key_values);
		init(key_values_soa.first, key_values_soa.second);
	}
	void init(const std::vector<key_t> &keys, const std::vector<value_t> &values)
	{
		m_x = keys;
		m_y = values;
	}
	value_t operator()(float key) const
	{
		switch (m_x.size()) {
		case 0: return value_t(0);
		case 1: return m_y[0];
		default: {
			if (key <= m_x.front()) {
				return m_y.front();
			}
			if (key >= m_x.back()) {
				return m_y.back();
			}

			auto i1 = std::lower_bound(begin(m_x), end(m_x), key) - begin(m_x);
			auto i0 = i1 - 1;
			auto rate = (key - m_x[i0]) / (m_x[i1] - m_x[i0]);
			return lerp(m_y[i0], m_y[i1], rate);
		}
		}
	}

	int32_t index(key_t key) const { return std::lower_bound(begin(m_x), end(m_x), key) - begin(m_x) - 1; }
	Range<key_t, 0x000f> range() const { return {m_x.front(), m_x.back()}; }

	std::vector<key_t> keys() const { return m_x; }
	std::vector<value_t> values() const { return m_y; }

private:
	std::vector<key_t> m_x;
	std::vector<value_t> m_y;
};

/// 3D B-spline interpolation
template<class key_t, class value_t> class SplineCurve {
public:
	SplineCurve() = default;
	SplineCurve(std::vector<std::pair<key_t, value_t>> &key_values) { init(key_values); }

	void init(std::vector<std::pair<key_t, value_t>> &key_values)
	{
		auto key_values_soa = vector_sort_and_separate(key_values);
		init(key_values_soa.first, key_values_soa.second);
	}
	void init(const std::vector<key_t> &keys, const std::vector<value_t> &values)
	{
		assert(keys.size() == values.size());
		auto n = int32_t(keys.size()) - 1;
		if (n <= 0) {
			return;
		}

		std::vector<key_t> h(n + 1);
		std::vector<key_t> b(n + 1);
		std::vector<key_t> g(n + 1);
		std::vector<value_t> d(n + 1);
		std::vector<value_t> u(n + 1);

		m_q.resize(n + 1);
		m_s.resize(n + 1);
		m_r.resize(n + 1);

		m_x = keys;
		m_y = values;

		// step 1
		for (auto i = 0; i < n; i++) {
			h[i] = m_x[i + 1] - m_x[i];
		}

		for (auto i = 1; i < n; i++) {
			b[i] = 2 * (h[i] + h[i - 1]);
			d[i] = 3 * (div2(m_y[i + 1] - m_y[i], h[i]) - div2(m_y[i] - m_y[i - 1], h[i - 1]));
		}
		// step 2
		g[1] = h[1] / b[1];

		for (auto i = 2; i < n - 1; i++) {
			g[i] = h[i] / (b[i] - h[i - 1] * g[i - 1]);
		}

		u[1] = d[1] / b[1];
		for (auto i = 2; i < n; i++) {
			u[i] = (d[i] - h[i - 1] * u[i - 1]) / (b[i] - h[i - 1] * g[i - 1]);
		}

		// step 3
		m_r[0] = value_t(0);
		m_r[n] = value_t(0);
		m_r[n - 1] = u[n - 1];
		for (auto i = n - 2; i >= 1; i--) {
			m_r[i] = u[i] - g[i] * m_r[i + 1];
		}

		// step 4
		for (auto i = 0; i < n; i++) {
			m_q[i] = div2(m_y[i + 1] - m_y[i], h[i]) - h[i] * div2(m_r[i + 1] + 2 * m_r[i], 3);
			m_s[i] = div2(m_r[i + 1] - m_r[i], 3 * h[i]);
		}
	}

	value_t operator()(key_t key) const
	{
		switch (m_x.size()) {
		case 0: return value_t(0);
		case 1: return m_y[0];
		case 2: {
			auto rate = std::clamp((key - m_x[0]) / (m_x[1] - m_x[0]), key_t(0), key_t(1));
			return lerp(m_y[0], m_y[1], rate);
		}
		default: {
			if (key <= m_x.front()) {
				return m_y.front();
			}
			if (key >= m_x.back()) {
				return m_y.back();
			}
			auto i0 = std::lower_bound(begin(m_x), end(m_x), key) - begin(m_x) - 1;
			auto dx = key - m_x[i0];
			return m_y[i0] + dx * (m_q[i0] + dx * (m_r[i0] + m_s[i0] * dx));
		}
		}
	}

	int32_t index(key_t key) const { return std::lower_bound(begin(m_x), end(m_x), key) - begin(m_x) - 1; }
	Range<key_t, 0x000f> range() const { return {m_x.front(), m_x.back()}; }

	std::vector<key_t> keys() const { return m_x; }
	std::vector<value_t> values() const { return m_y; }

private:
	std::vector<key_t> m_x;
	std::vector<value_t> m_y, m_r, m_q, m_s;

	// safe divide
	value_t div2(const value_t &x, const key_t &y) { return y == key_t(0) ? value_t(0) : x / y; }
};
}  // namespace spu
