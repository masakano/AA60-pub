//
// ShadowFrustumf :
//
#pragma once
#include "composition.h"
namespace spu {

/// well-culled shadow frustum
class ShadowFrustumf {
public:
	enum {
		e_parallel = 0,
		e_point = 1,
	};

	ShadowFrustumf(const Vec3f &light_direction, const Vec3f &light_position, int32_t light_type = e_point)
	        : m_lightDirection(light_direction), m_lightPosition(light_position), m_lightType(light_type)
	{
	}

	Mat4f capture(const std::vector<Vec3f> &blocker_points) const;
	Mat4f prune(const Mat4f &worldscreen, const Mat4f shadow_frustum) const;
	// std::vector<Vec3f> prune(const Mat4f &worldscreen, const std::vector<Vec3f> &blocker_points) const;

private:
	const Vec3f m_lightDirection;
	const Vec3f m_lightPosition;
	const int32_t m_lightType;
};
}  // namespace spu
