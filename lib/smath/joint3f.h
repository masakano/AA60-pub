//
// Joint3f :
//
#pragma once
#include "geometry.h"
#include "bone3f.h"

namespace spu {

class Joint3f : public Bone3f {
public:
	Joint3f() = default;
	~Joint3f() = default;

	Joint3f(Bone3f *parent, const Transformf &transform, bool is_local = true)
	{
		init(parent, transform, is_local);
	}

	void update(const Vec3f &target, const Vec3f &effect);
	void setFixedAxis(const Vec3f *axis);
	void setMaxRadian(float max_radian) { m_maxRadian = max_radian; }

protected:
	const float c_maxError = 0.0001;  // ad-hoc
	Vec3f m_fixedAxis = {0, 0, 1};
	float m_maxRadian = radians(180.0f);
	bool m_isFixedAxis = false;
};
}  // namespace spu
