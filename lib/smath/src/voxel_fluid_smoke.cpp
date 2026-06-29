//
// VoxelFluidSmoke :
//
#include <smath/voxel_fluid.h>
#include <assert.h>
#include <math.h>

namespace spu {

VoxelFluidSmoke::VoxelFluidSmoke(int32_t size) : VoxelFluid(size)
{
	auto grid_dim = this->size();
	auto grid = Vec4i(grid_dim, grid_dim, grid_dim, 1);

	m_density.resize(grid);
	m_velocity.resize(grid);
}

void VoxelFluidSmoke::addBuoyancy(float dt)
{
	auto op = [&](const Vec4i & /*p*/, int32_t index) {
		m_velocity.curr[index].y += -m_density.curr[index] * m_buoyancy * dt;
	};
	m_velocity.curr.apply(0, op);
}

void VoxelFluidSmoke::velocityStep(float dt)
{
	addSource(m_velocity, dt);
	addBuoyancy(dt);
	vorticityConfinement(m_velocity, dt);

	diffuse(m_velocity, m_viscosity, dt);

	project(m_velocity.next);
	advect(m_velocity, m_velocity.next, dt);
	project(m_velocity.curr);
}

void VoxelFluidSmoke::densityStep(float dt)
{
	addSource(m_density, dt);
	diffuse(m_density, m_diffusion, dt);
	advect(m_density, m_velocity.curr, dt);
}

void VoxelFluidSmoke::update(float dt)
{
	velocityStep(dt);
	densityStep(dt);
}
}  // namespace spu
