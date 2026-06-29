//
// VoxelFluidFire :
//
/* Author: Johannes Schmid, 2006, johnny@grob.org */
#include <smath/voxel_fluid.h>

namespace spu {

VoxelFluidFire::VoxelFluidFire(int32_t size) : VoxelFluid(size)
{
	auto grid_dim = this->size();
	auto grid = Vec4i(grid_dim, grid_dim, grid_dim, 1);

	m_density.resize(grid);
	m_tempreature.resize(grid);
	m_velocity.resize(grid);
}

void VoxelFluidFire::advectCool(
        VoxelState<float> &state_x, VoxelState<float> &state_y, const Voxels<Vec3f> &vec, float dt)
{
	auto size = float(this->size());
	auto dt0 = dt * (size - 2.0f);
	auto c0 = 1.0f - m_cooling * dt;

	auto op = [&](const Vec4i &p, int32_t index) {
		Vec3f d = {
		        p.x - dt0 * vec[index].x,
		        p.y - dt0 * vec[index].y,
		        p.z - dt0 * vec[index].z,
		};
		d = clamp(d, Vec3f(0.5), Vec3f((size - 2.0f) + 0.5f));
		state_x.next[index] = state_x.curr.get(d);
		state_y.next[index] = state_y.curr.get(d) * c0;
	};
	state_x.next.apply(1, op);
}

void VoxelFluidFire::addBuoyancy(float dt)
{
	auto op = [&](const Vec4i & /*p*/, int32_t index) {
		m_velocity.curr[index].y += -m_tempreature.curr[index] * m_buoyancy * dt;
	};
	m_velocity.curr.apply(0, op);
}

void VoxelFluidFire::velocityStep(float dt)
{
	addSource(m_velocity, dt);
	addBuoyancy(dt);
	vorticityConfinement(m_velocity, dt);

	diffuse(m_velocity, m_viscosity, dt);

	project(m_velocity.next);
	advect(m_velocity, m_velocity.next, dt);
	project(m_velocity.curr);
}

void VoxelFluidFire::densityAndTempreatureStep(float dt)
{
	addSource(m_density, dt);
	addSource(m_tempreature, dt);

	diffuse(m_density, m_diffusion, dt);
	advectCool(m_density, m_tempreature, m_velocity.curr, dt);

	m_density.swap();
	m_tempreature.swap();
}

void VoxelFluidFire::update(float dt)
{
	velocityStep(dt);
	densityAndTempreatureStep(dt);
}
}  // namespace spu
