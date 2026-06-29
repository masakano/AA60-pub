//
// RandomGenerator : quick random number generator
//
#pragma once

#include "ssys.h"
#include <random>

namespace spu {

template<class T, class distribution_t = std::uniform_real_distribution<T>> class RandomGenerator {
public:
	RandomGenerator(const distribution_t &dist = distribution_t(T(0.0), T(1.0))) : m_dist(dist)
	{
		if (ms_seeds().empty()) {
			std::random_device rd;
			m_mt.seed(rd());
		}
		else {
			m_mt.seed(ms_seeds()[ms_index]);
			ms_index = (ms_index + 1) % ms_seeds().size();
		}
	}
	RandomGenerator(T a0, T a1) : RandomGenerator(distribution_t(a0, a1)) {}
	void seed(uint32_t seed) { m_mt.seed(seed); }
	T operator()() { return m_dist(m_mt); }

	static void reset(const std::vector<uint32_t> &seeds)
	{
		ms_seeds() = seeds;
		ms_index = 0;
	}

private:
	std::mt19937 m_mt;
	distribution_t m_dist;
	static std::vector<uint32_t> &ms_seeds()
	{
		static std::vector<uint32_t> v;
		return v;
	}
	inline static uint32_t ms_index;
};
}  // namespace spu
