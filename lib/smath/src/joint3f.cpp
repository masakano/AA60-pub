//
// Joint3f :
//
#include <smath/joint3f.h>

namespace spu {

void Joint3f::setFixedAxis(const Vec3f *axis)
{
	if (axis) {
		assert(length(*axis) > 0);
		m_fixedAxis = normalize(*axis);
		m_isFixedAxis = true;
	}
	else {
		m_isFixedAxis = false;
	}
}

void Joint3f::update(const Vec3f &target, const Vec3f &effect)
{
	if (length(effect - target) < c_maxError) {
		return;
	}

	auto prev_boneparent = boneparent();
	auto normalized_effect = normalize(effect);
	auto normalized_target = normalize(target);
	Quatf rotation;

	if (m_isFixedAxis) {
		rotation = Quatf::from_target_and_axis(
		        normalized_target, normalized_effect, m_fixedAxis, m_maxRadian);
	}
	else {
		rotation = Quatf::from_target(normalized_target, normalized_effect, m_maxRadian);
	}
	setBoneparent(boneparent() * Transformf(ezero(), rotation));

	if (parent()) {
		dynamic_cast<Joint3f *>(parent())->update(prev_boneparent * target, boneparent() * effect);
	}
	else {
		sync();
	}
}
}  // namespace spu
