//
// Particle :
//
#pragma once
#include "vec.h"

namespace spu {

template<class vec_t = Vec3f> struct Particle {
	vec_t p = vec_t(0);
	vec_t v = vec_t(0);
};

template<class physics_t, class vec_t = Vec3f> class Particles : public std::vector<Particle<vec_t>> {
public:
	Particles eulerF(physics_t &physics, float h = 1.0)
	{
		return madd(1.0, physics.differential(*this, h));
	}

	Particles eulerB(physics_t &physics, float h = 1.0)
	{
		auto s0 = madd(1.0, physics.differential(*this, h));
		for (auto &s: *this) {
			auto i = &s - &(*this)[0];
			s0[i].p = s.p;  // backward
		}
		auto d1 = physics.differential(s0, h);
		return madd(1.0, d1);
	}

	Particles rungekutta(physics_t &physics, float h = 1.0)
	{
		auto k1 = physics.differential(*this, 0);
		auto k2 = physics.differential(madd(h / 2, k1), h / 2);
		auto k3 = physics.differential(madd(h / 2, k2), h / 2);
		auto k4 = physics.differential(madd(h, k3), h);

		Particles dst;
		for (auto &s: *this) {
			auto i = &s - &(*this)[0];
			dst.emplace_back(
			        s.p + 1 * h / 6 * k1[i].p + 2 * h / 6 * k2[i].p + 2 * h / 6 * k3[i].p
			                + 1 * h / 6 * k4[i].p,

			        s.v + 1 * h / 6 * k1[i].v + 2 * h / 6 * k2[i].v + 2 * h / 6 * k3[i].v
			                + 1 * h / 6 * k4[i].v);
		}
		return dst;
	}

	Particles madd(float a, const Particles &v) const
	{
		Particles dst;
		for (auto &s: *this) {
			int32_t i = &s - &(*this)[0];
			dst.emplace_back(s.p + a * v[i].p, s.v + a * v[i].v);
		}
		return dst;
	}
};
}  // namespace spu
