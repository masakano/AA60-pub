//
// MMDAnimation :
//
#include <smath/mmd.h>

namespace spu {

void MMDAnimation::init(
        const std::string &name, const std::vector<MMDMotion<Transformf>> bone_motion_pool,
        const std::vector<MMDMotion<float>> morph_motion_pool)
{
	m_name = name;
	m_boneMotionPool = bone_motion_pool;
	m_morphMotionPool = morph_motion_pool;

	m_frameRange.invalidate();
	for (auto &bone_motion: m_boneMotionPool) {
		m_frameRange.expand(bone_motion.range());
	}
	for (auto &morph_motion: m_morphMotionPool) {
		m_frameRange.expand(morph_motion.range());
	}
}

Transformf MMDAnimation::rehearsal(float frame, const std::string &name) const
{
	frame = std::clamp(frame, m_frameRange.p0, m_frameRange.p1);

	if (name == m_cachedBoneMotion.name()) {
		return m_cachedBoneMotion.animate(frame);
	}

	for (const auto &motion: m_boneMotions) {
		if (motion.target()->name() == name) {
			m_cachedBoneMotion = motion;
			return m_cachedBoneMotion.animate(frame);
		}
	}
	aux_error(true, "rehearsal: bone '%s' not found (%ld motions)\n", name.c_str(), m_boneMotions.size());
}

void MMDAnimation::animate(float frame)
{
	frame = std::clamp(frame, m_frameRange.p0, m_frameRange.p1);
	if (m_isBone) {
		for (auto &motion: m_boneMotions) {
			motion.animate(frame);
		}
	}

	if (m_isMorph) {
		for (auto &motion: m_morphMotions) {
			motion.animate(frame);
		}
	}
}

}  // namespace spu
