//
// VoxelFluid :
//
/* Author: Johannes Schmid, 2006, johnny@grob.org */
#include <smath/voxel_fluid.h>
#include <ssys/random_generator.h>

namespace spu {

VoxelFluid::VoxelFluid(int32_t size) : m_size(size)
{
	auto grid = Vec4i(size, size, size, 1);
	m_divergence0.resize(grid);
	m_divergence1.resize(grid);
	m_rotation1.resize(grid);

	RandomGenerator<float> frand = {0, 1};
	for (auto &x: m_rands) {
		x = frand();
	}
}

float VoxelFluid::genfunc(int32_t x, int32_t y, int32_t sx, int32_t sy, float t)
{
	constexpr auto pi2 = pi() * 2;
	auto *rp = m_rands;
	auto f = 0.0f;
	for (auto i = 0; i < 12; i++) {
		auto xf = float(x);
		auto yf = float(y);

		auto cx = sin(xf / sx * pi2 * (rp[0] + 1.0) + rp[1] * pi2 + rp[2] * t);
		auto cy = sin(yf / sy * pi2 * (rp[3] + 1.0) + rp[4] * pi2 + rp[5] * t);
		auto cz = sin((rp[6] + 0.5) * t + rp[7] * pi2);

		f += (1.0 + cx * cy) * (1.0 + cz) * 0.25;
		rp += 8;
	}
	// f *= 1.0 / float(i);
	f /= 12.0;

	auto landing = [](float x, float s) {
		if (x < s * 0.1) return 0.5 + x / (s * 0.2);
		if (x < s * 0.9) return 1.0;
		return 1.0 - (x - s * 0.9) / (s * 0.2);
	};
	return f * landing(x, sx) * landing(y, sy);
}

template<class VT> void VoxelFluid::addSource(VoxelState<VT> &state, float dt)
{
	auto op = [&](const Vec4i & /*p*/, int32_t index) { state.curr[index] += state.source[index] * dt; };
	state.curr.apply(0, op);
}

template<class VT> void VoxelFluid::diffuse(VoxelState<VT> &state, float diff, float dt)
{
	auto size = float(this->size());
	auto r = dt * diff * (size - 2.0f) * (size - 2.0f) * (size - 2.0f);
	auto a = 1 / (1 + 6 * r);
	auto b = r / (1 + 6 * r);

	for (auto l = 0; l < 20; l++) {
		auto op = [&](const Vec4i &p, int32_t index) {
			state.next[index] = a * state.curr[index] + b * state.next.sum6(p);
		};
		state.next.apply(1, op);
	}
}

template<class VT> void VoxelFluid::advect(VoxelState<VT> &state, const Voxels<Vec3f> &v, float dt)
{
	auto size = float(this->size());
	auto dt0 = dt * (size - 2.0f);
	auto op = [&](const Vec4i &p, int32_t index) {
		Vec3f d = {
		        p.x - dt0 * v[index].x,
		        p.y - dt0 * v[index].y,
		        p.z - dt0 * v[index].z,
		};
		d = clamp(d, Vec3f(0.5), Vec3f((size - 2.0f) + 0.5f));
		state.curr[index] = state.next.get(d);
	};
	state.curr.apply(1, op);
}

void VoxelFluid::project(Voxels<Vec3f> &vel)
{
	auto size = float(this->size());
	auto h = 1.0f / (size - 2.0f);

	auto op0 = [&](const Vec4i &p, int32_t index) {
		m_divergence0[index] = -h * (vel.dx(p).x + vel.dy(p).y + vel.dz(p).z) / 3;  // v0: divergence
		m_divergence1[index] = 0;
	};
	m_divergence0.apply(1, op0);

	auto op1 = [&](const Vec4i &p, int32_t index) {
		m_divergence1[index] = (m_divergence0[index] + m_divergence1.sum6(p)) / 6;
	};

	for (auto l = 0; l < 20; l++) {
		m_divergence1.apply(1, op1);
	}

	auto op2 = [&](const Vec4i &p, int32_t index) {
		vel[index].x -= m_divergence1.dx(p) / 3 / h;
		vel[index].y -= m_divergence1.dy(p) / 3 / h;
		vel[index].z -= m_divergence1.dz(p) / 3 / h;
	};
	vel.apply(1, op2);
}

void VoxelFluid::vorticityConfinement(VoxelState<Vec3f> &vel, float dt)
{
	auto dt0 = dt * m_eps;

	auto op0 = [&](const Vec4i &p, int32_t index) {
		vel.next[index].x = (vel.curr.dy(p).z - vel.curr.dz(p).y) * 0.5;  // curlx = dw/dy - dv/dz
		vel.next[index].y = (vel.curr.dz(p).x - vel.curr.dx(p).z) * 0.5;  // curly = du/dz - dw/dx
		vel.next[index].z = (vel.curr.dx(p).y - vel.curr.dy(p).x) * 0.5;  // curlz = dv/dx - du/dy
		m_rotation1[index] = length(Vec3f(vel.next[index].x, vel.next[index].y, vel.next[index].z));
	};
	m_rotation1.apply(1, op0);  // d0: curl

	auto op1 = [&](const Vec4i &p, int32_t index) {
		Vec3f M = {
		        m_rotation1.dx(p) * 0.5f,
		        m_rotation1.dy(p) * 0.5f,
		        m_rotation1.dz(p) * 0.5f,
		};
		M = normalize(M + Vec3f(epsilon()));  // avoid FPE exception
		vel.curr[index].x += (M.y * vel.next[index].z - M.z * vel.next[index].y) * dt0;
		vel.curr[index].y += (M.z * vel.next[index].x - M.x * vel.next[index].z) * dt0;
		vel.curr[index].z += (M.x * vel.next[index].y - M.y * vel.next[index].x) * dt0;
	};
	m_rotation1.apply(1, op1);  // d0: curl
}

template void VoxelFluid::addSource(VoxelState<float> &state, float dt);
template void VoxelFluid::addSource(VoxelState<Vec3f> &state, float dt);
template void VoxelFluid::diffuse(VoxelState<float> &state, float diff, float dt);
template void VoxelFluid::diffuse(VoxelState<Vec3f> &state, float diff, float dt);
template void VoxelFluid::advect(VoxelState<float> &state, const Voxels<Vec3f> &v, float dt);
template void VoxelFluid::advect(VoxelState<Vec3f> &state, const Voxels<Vec3f> &v, float dt);

}  // namespace spu
