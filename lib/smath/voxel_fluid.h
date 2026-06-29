//
// VoxelFluid :
//
/* Author: Johannes Schmid, 2006, johnny@grob.org */
#pragma once
#include "voxel.h"

namespace spu {

class VoxelFluid {
public:
	template<class T> struct VoxelState {
		Voxels<T> source;
		Voxels<T> curr;
		Voxels<T> next;

		void resize(const Vec4i &grid)
		{
			curr.resize(grid);
			next.resize(grid);
			source.resize(grid);
		}

		void swap() { curr.swap(next); }
	};

	struct RGBA {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	explicit VoxelFluid(int32_t size = 32);
	virtual ~VoxelFluid() = default;
	virtual void update(float dt) = 0;

	void setEpsilon(const float eps) { m_eps = eps; }
	int32_t indexof(const Vec4i &p) const { return m_divergence0.indexof(p); }
	int32_t size() const { return m_size; }

	VoxelState<float> &getDensity() { return m_density; }
	const VoxelState<float> &getDensity() const { return m_density; }

	VoxelState<Vec3f> &getVelocity() { return m_velocity; }
	const VoxelState<Vec3f> &getVelocity() const { return m_velocity; }

	VoxelState<float> &getTemperature() { return m_tempreature; }
	const VoxelState<float> &getTemperature() const { return m_tempreature; }

	void setDiffusion(float diffusion) { m_diffusion = diffusion; }
	void setViscosity(float viscosity) { m_viscosity = viscosity; }
	void setBuoyancy(float buoyancy) { m_buoyancy = buoyancy; }
	void setCooling(float cooling) { m_cooling = cooling; }

	float genfunc(int32_t x, int32_t y, int32_t sx, int32_t sy, float t);
	static std::vector<RGBA> generateSpectrum(float t1, float t2, int32_t n);

protected:
	template<class T> void addSource(VoxelState<T> &state, float dt);
	template<class T> void diffuse(VoxelState<T> &state, float diff, float dt);
	template<class T> void advect(VoxelState<T> &state, const Voxels<Vec3f> &v, float dt);

	void project(Voxels<Vec3f> &vel);
	void vorticityConfinement(VoxelState<Vec3f> &vel, float dt);  // vel.curr -> vel.next -> vel.curr

	VoxelState<float> m_density;
	VoxelState<float> m_tempreature;
	VoxelState<Vec3f> m_velocity;

	float m_diffusion;
	float m_viscosity;
	float m_buoyancy;
	float m_cooling;

private:
	Voxels<float> m_divergence0;
	Voxels<float> m_divergence1;
	Voxels<float> m_rotation1;
	int32_t m_size;
	float m_rands[256];
	float m_eps;
};

class VoxelFluidFire : public VoxelFluid {
public:
	explicit VoxelFluidFire(int32_t size = 32);
	void update(float dt) override;

private:
	void advectCool(
	        VoxelState<float> &state_x, VoxelState<float> &state_y, const Voxels<Vec3f> &vec, float dt);

	void addBuoyancy(float dt);
	void velocityStep(float dt);
	void densityAndTempreatureStep(float dt);
};

class VoxelFluidSmoke : public VoxelFluid {
public:
	explicit VoxelFluidSmoke(int32_t size = 32);
	void update(float dt) override;

private:
	void addBuoyancy(float dt);
	void velocityStep(float dt);
	void densityStep(float dt);
};

}  // namespace spu
