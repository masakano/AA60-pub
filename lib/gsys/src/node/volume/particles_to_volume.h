//
// ParticlesToVolume :
//
#pragma once
#include <gsys/canvas.h>

namespace spu::gs_node::volume {

class ParticlesToVolume {
public:
	float u_radius = 1.0;

	ParticlesToVolume(uint32_t density_texture);
	~ParticlesToVolume();
	void draw(const std::vector<Vec3f> &particles);

private:
	const int32_t def_local_size = 128;
	const int32_t def_copy_local_size = 8;

	uint32_t u_uint_density_texture = 0;
	uint32_t u_density_texture = 0;
	int32_t u_particle_count = 0;

	SpuComputeArray m_fillArray;
	SpuComputeArray m_copyArray;
};
}  // namespace spu::gs_node::volume
