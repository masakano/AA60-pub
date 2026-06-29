//
// DualQuatf :
//
#pragma once
#include "quatf.h"

namespace spu {

/// dual quaternion
struct DualQuatf {
public:
	Quatf real;
	Quatf dual;

	DualQuatf() : real(0, 0, 0, 1), dual(0, 0, 0, 0) {}

	DualQuatf(const Quatf &real, const Quatf &dual) : real(real), dual(dual) {}

	DualQuatf(const Quatf &q, const Vec3f &t)
	{
		real = spu::normalize(q);
		dual = (Quatf(t.x, t.y, t.z, 0) * real) * 0.5;
	}

	DualQuatf(const Transformf &tr) : DualQuatf(tr.q, tr.t) {}

	operator Transformf() const
	{
		auto q = spu::normalize(real);
		auto t = 2.0 * dual * q.conj();
		return Transformf(t, q);
	}

	friend DualQuatf operator+(const DualQuatf &dq0, const DualQuatf &dq1)
	{
		return DualQuatf(dq0.real + dq1.real, dq0.dual + dq1.dual);
	}

	friend DualQuatf operator-(const DualQuatf &dq0, const DualQuatf &dq1)
	{
		return DualQuatf(dq0.real - dq1.real, dq0.dual - dq1.dual);
	}

	friend DualQuatf operator*(const DualQuatf &dq0, const DualQuatf &dq1)
	{
		return DualQuatf(dq1.real * dq0.real, dq1.dual * dq0.real + dq1.real * dq0.dual);
	}

	friend DualQuatf operator*(const DualQuatf &dq, float scale)
	{
		return DualQuatf(dq.real * scale, dq.dual * scale);
	}

	friend DualQuatf operator*(float scale, const DualQuatf &dq)
	{
		return DualQuatf(dq.real * scale, dq.dual * scale);
	}

	DualQuatf normalize() const
	{
		auto d = real.dot(real);
		assert(d > 0);
		return *this * (1.0 / d);
	}

	DualQuatf conj() const { return DualQuatf(real.conj(), dual.conj()); }
	Quatf rot() const { return real; }
	Vec3f trans() const { return 2.0 * dual * real.conj(); }

	template<class T> static T blend(const std::vector<std::pair<float, T>> &pairs)
	{
		auto real = Quatf(0, 0, 0, 0);
		auto dual = Quatf(0, 0, 0, 0);

		auto scale = 0.0f;
		for (auto &pair: pairs) {
			scale += pair.first;
		}
		for (auto &pair: pairs) {
			auto weight = pair.first / scale;
			auto dual_quat = DualQuatf(pair.second);

			if (dot(Vec4f(real), Vec4f(dual_quat.real)) < 0) {  // long-path problem
				dual_quat.real = -dual_quat.real;
				dual_quat.dual = -dual_quat.dual;
			}
			real = real + dual_quat.real * weight;
			dual = dual + dual_quat.dual * weight;
		}
		return DualQuatf(real, dual);
	}
};
template<> inline DualQuatf normalize(const DualQuatf &dq) { return dq.normalize(); }

template<> inline DualQuatf lerp(const DualQuatf &a0, const DualQuatf &a1, const float &r)
{
	std::vector<std::pair<float, DualQuatf>> pairs = {
	        {1.0f - r, a0},
	        {r,        a1},
	};
	return DualQuatf::blend(pairs);
}
}  // namespace spu
