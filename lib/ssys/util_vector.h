//
//
//
#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>  // memcpy
#include <numeric>
#include <vector>

// namespace spu::util {
namespace spu {

template<class T> T vector_at(const std::vector<T> &v, int32_t index)
{
	assert(!v.empty());
	return v[std::clamp(index, 0, int32_t(v.size()) - 1)];
}

template<class T0, class T1> std::vector<T0> &vector_cat(std::vector<T0> &v0, const std::vector<T1> &v1)
{
	v0.insert(end(v0), begin(v1), end(v1));
	return v0;
}
template<class T> T vector_accumulate(const std::vector<T> &vec)
{
	return std::accumulate(begin(vec), end(vec), T(0));
}
template<class T> T vector_average(const std::vector<T> &vec) { return vector_accumulate(vec) / vec.size(); }

template<class T> void vector_sort(std::vector<T> &vec) { std::sort(begin(vec), end(vec)); }

template<class T, class DT> void vector_sort(std::vector<T> &vec, DT gt)
{
	std::sort(begin(vec), end(vec), gt);
}

template<class T, class DT0, class DT1> void vector_sort_and_uniq(std::vector<T> &vec, DT0 gt, DT1 eq)
{
	std::sort(begin(vec), end(vec), gt);
	vec.erase(std::unique(begin(vec), end(vec), eq), end(vec));
}

template<class T> void vector_sort_and_uniq(std::vector<T> &vec)
{
	vector_sort_and_uniq(
	        vec, [](const T &b0, const T &b1) { return b0 > b1; },
	        [](const T &b0, const T &b1) { return b0 == b1; });
}

template<class T0, class T1> bool vector_is_find(const std::vector<T0> &vec, const T1 &v1)
{
	return std::find(begin(vec), end(vec), T0(v1)) != end(vec);
}

template<class T, class DT> bool vector_is_find_if(const std::vector<T> &vec, DT op)
{
	return std::find_if(begin(vec), end(vec), op) != end(vec);
}

template<class T0, class T1> auto vector_find(const std::vector<T0> &vec, const T1 &v)
{
	return std::find(begin(vec), end(vec), T0(v));
}

template<class T, class DT> auto vector_find_if(const std::vector<T> &vec, DT op)
{
	return std::find_if(begin(vec), end(vec), op);
}

template<class T0, class T1> auto vector_find(std::vector<T0> &vec, const T1 &v)
{
	return std::find(begin(vec), end(vec), T0(v));
}

template<class T, class DT> auto vector_find_if(std::vector<T> &vec, DT op)
{
	return std::find_if(begin(vec), end(vec), op);
}

template<class T0, class T1> void vector_remove(std::vector<T0> &vec, const T1 &v)
{
	vec.erase(std::remove(begin(vec), end(vec), T0(v)), end(vec));
}

template<class T, class DT> void vector_remove_if(std::vector<T> &vec, DT op)
{
	vec.erase(std::remove_if(begin(vec), end(vec), op), end(vec));
}

template<class T, class DT> auto vector_minmax(const std::vector<T> &vec, DT op)
{
	auto max_d = op(vec[0]);
	auto min_d = op(vec[0]);
	auto max_it = begin(vec);
	auto min_it = begin(vec);

	for (auto it = begin(vec) + 1; it != end(vec); it++) {
		auto d = op(*it);
		if (d > max_d) max_d = d, max_it = it;
		if (d < min_d) min_d = d, min_it = it;
	}
	return std::pair<decltype(min_it), decltype(max_it)>(min_it, max_it);
}
template<class T, class DT> auto vector_min(const std::vector<T> &vec, DT op)
{
	auto min_d = op(vec[0]);
	auto min_it = begin(vec);
	for (auto it = begin(vec) + 1; it != end(vec); it++) {
		auto d = op(*it);
		if (d < min_d) min_d = d, min_it = it;
	}
	return min_it;
}
template<class T, class DT> auto vector_max(const std::vector<T> &vec, DT op)
{
	auto max_d = op(vec[0]);
	auto max_it = begin(vec);

	for (auto it = begin(vec) + 1; it != end(vec); it++) {
		auto d = op(*it);
		if (d > max_d) max_d = d, max_it = it;
	}
	return max_it;
}

template<class F, class S> auto vector_sort_and_separate(std::vector<std::pair<F, S>> &vec)
{
	std::pair<std::vector<F>, std::vector<S>> vec_soa;
	vector_sort(vec, [](std::pair<F, S> &p0, std::pair<F, S> &p1) { return p0.first < p1.first; });
	for (auto &elem: vec) {
		vec_soa.first.push_back(elem.first);
		vec_soa.second.push_back(elem.second);
	}
	return vec_soa;
}
}  // namespace spu
