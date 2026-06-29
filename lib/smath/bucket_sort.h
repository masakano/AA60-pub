//
// BucketSort :
//
#pragma once

#include "voxel.h"

namespace spu {

template<class T>
concept vec3f_castable_t = requires(T t) {
	{ static_cast<Vec3f>(t) } -> std::same_as<Vec3f>;
};

template<vec3f_castable_t particle_t> class BucketSort {
public:
	void init(float search_radius) { m_scale = 1.0 / search_radius; }

	void sort(particle_t *sp, particle_t *ep)
	{
		Range3f range;
		range.invalidate();
		for (auto *p = sp; p != ep; p++) {
			range.expand(Vec3f(*p));
		}

		if (((range.span() < Vec3f(1)).pack() & 0xfff) == 0xfff) {
			m_range = range;
		}

		auto size = m_range.span() * m_scale + 1;
		size.w = 1;
		m_buckets.resize(size);
		m_buckets.fill(std::vector<particle_t *>());

		for (auto p = sp; p != ep; ++p) {
			auto grid = positionToGrid(Vec3f(*p));
			m_buckets.at(grid).push_back(p);
		}
	}

	void getNeighbour(const particle_t *p0, std::vector<particle_t *> &neighbours) const
	{
		const auto search_radius = 1.0 / m_scale;
		const auto search_radius2 = search_radius * search_radius;

		Vec4i g0 = positionToGrid(Vec3f(*p0));
		Vec4i g = Vec4i(0, 0, 0, 0);
		auto size = m_buckets.size();
		for (g.z = std::max(g0.z - 1, 0); g.z < std::min(g0.z + 2, size.z); g.z++) {
			for (g.y = std::max(g0.y - 1, 0); g.y < std::min(g0.y + 2, size.y); g.y++) {
				for (g.x = std::max(g0.x - 1, 0); g.x < std::min(g0.x + 2, size.x); g.x++) {
					for (auto p1: m_buckets.at(g)) {
						if (p1 >= p0) break;
						auto delta = Vec3f(*p0) - Vec3f(*p1);
						if (dot(delta, delta) < search_radius2) {
							neighbours.push_back(p1);
						}
					}
				}
			}
		}
	}

private:
	Voxels<std::vector<particle_t *>> m_buckets;
	float m_scale;
	Range3f m_range;

	Vec4i positionToGrid(const Vec3f &position) const
	{
		auto clamped_position = clamp(position, m_range.p0, m_range.p1);
		return Vec4i((clamped_position - m_range.p0) * m_scale);
	}
};

}  // namespace spu
