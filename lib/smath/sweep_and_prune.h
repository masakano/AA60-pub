//
// SweepAndPrune :
//
#pragma once
#include "vec.h"

namespace spu {

template<class vec_t = Vec3f> class SweepAndPrune {
public:
	void init(uint32_t size)
	{
		m_size = size;
		m_pairPoints.resize(m_size * 2);
		m_bitvec0.init(m_size * size);
		m_bitvec1.init(m_size * size);

		for (auto dim = 0; dim < 3; dim++) {
			m_indices[dim].resize(m_size * 2);
			std::iota(begin(m_indices[dim]), end(m_indices[dim]), 0);
		}
	}

	std::vector<vec_t> &getPairPoints() { return m_pairPoints; }
	const std::vector<vec_t> &getPairPoints() const { return m_pairPoints; }

	std::vector<vec_t> centerPoints() const
	{
		std::vector<vec_t> center_points;
		center_points.reserve(m_pairPoints.size() / 2);
		for (auto n = 0u; n < m_pairPoints.size(); n += 2) {
			center_points.push_back((m_pairPoints[n + 0] + m_pairPoints[n + 1]) * 0.5);
		}
		return center_points;
	}

	void sort()
	{
		m_dusecs[0] = m_dusecs[1] = 0;
		m_bitvec0.reset(true);

		for (auto dim = 0; dim < 3; dim++) {
			// sort
			auto usec0 = get_microsec();
			{
				auto &indices = m_indices[dim];
				auto gt = [&](const uint32_t i0, uint32_t i1) {
					auto &p0 = m_pairPoints[i0].f[dim];
					auto &p1 = m_pairPoints[i1].f[dim];
					if (p0 == p1) {
						return (i0 % 2) < (i1 % 2);  // begin < end
					}
					return p0 < p1;
				};
				vector_sort(indices, gt);
			}
			auto usec1 = get_microsec();
			m_dusecs[0] += usec1 - usec0;

			// degrate
			{
				auto &indices = m_indices[dim];
				auto obj0 = indices.front() / 2;
				auto obj1 = indices.back() / 2;
				auto obj0_p1 = m_pairPoints[obj0 * 2 + 1];
				auto obj1_p0 = m_pairPoints[obj1 * 2 + 0];
				if (obj0_p1.f[dim] >= obj1_p0.f[dim]) {
					continue;
				}
			}

			// check
			{
				auto &indices = m_indices[dim];
				m_bitvec1.reset(false);
				for (auto it0 = begin(indices); it0 != end(indices); ++it0) {
					if (*it0 % 2) continue;
					for (auto it1 = it0 + 1; *it1 != *it0 + 1 && it1 != end(indices);
					     ++it1) {
						auto index0 = *it0 / 2;
						auto index1 = *it1 / 2;
						if (index0 > index1) std::swap(index0, index1);
						m_bitvec1.set(index0 * m_size + index1);
					}
				}
				m_bitvec0 &= m_bitvec1;
			}
			auto usec2 = get_microsec();
			m_dusecs[1] += usec2 - usec1;
		}
	}

	bool hit(uint32_t index0, uint32_t index1) const
	{
		if (index0 > index1) std::swap(index0, index1);
		return m_bitvec0.get(index0 * m_size + index1);
	}

	uint64_t dusecs(uint64_t index) { return m_dusecs[index]; }
	uint32_t size() const { return m_size; }

private:
	class BitVec {
	public:
		using array_t = uint64_t;
		static constexpr const uint32_t N = sizeof(array_t) * 8;

		void init(uint32_t size) { m_array.resize((size + N - 1) / N); }
		void reset(bool bit) { std::fill(begin(m_array), end(m_array), bit ? ~0lu : 0lu); }
		void set(uint32_t index) { m_array[index / N] |= (1lu << (index % N)); }
		bool get(uint32_t index) const
		{
			return (m_array[index / N] & (1lu << (index % N))) == 0 ? 0 : 1;
		}
		const BitVec &operator&=(const BitVec &bv)
		{
			for (auto i = 0u; i < m_array.size(); i++) {
				m_array[i] &= bv.m_array[i];
			}
			return *this;
		}

	private:
		std::vector<array_t> m_array;
	};

	uint32_t m_size = 0;
	std::vector<vec_t> m_pairPoints;
	std::vector<uint32_t> m_indices[3];
	BitVec m_bitvec0;
	BitVec m_bitvec1;
	uint64_t m_dusecs[2];
};
}  // namespace spu
