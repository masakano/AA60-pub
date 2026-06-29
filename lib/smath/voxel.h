//
// Voxels :
//
#pragma once

#include "mat4f.h"
#include <bit>  // popcount

namespace spu {

/// generic 3D grid
template<class T> class Voxels {
public:
	void resize(const Vec4i &size)
	{
		auto n = size.x * size.y * size.z * size.w;
		assert(n);

		m_size = size;
		m_stride = {1, m_size.x, m_size.x * m_size.y, m_size.x * m_size.y * m_size.w};
		m_data.resize(n);
	}

	bool inside(const Vec4i &p, const Vec4i &m = Vec4i(0)) const
	{
		auto det = (p >= m & p < m_size - m);
		return det.pack() == 0xffff;
	}

	Vec4i gridof(int32_t index) const
	{
		Vec4i p = {
		        (index / m_stride.x) % m_size.x,
		        (index / m_stride.y) % m_size.y,
		        (index / m_stride.z) % m_size.z,
		        (index / m_stride.w),
		};
		return p;
	}
	int32_t indexof(const Vec4i &p) const { return dot(p, m_stride); }
	const Vec4i &size() const { return m_size; }

	T *data() { return m_data.empty() ? nullptr : m_data.data(); }
	const T *data() const { return m_data.empty() ? nullptr : m_data.data(); }

	T &operator[](int32_t index) { return m_data[index]; }
	const T &operator[](int32_t index) const { return m_data[index]; }

	T &at(const Vec4i &p) { return m_data.at(indexof(p)); }
	const T &at(const Vec4i &p) const { return m_data.at(indexof(p)); }

	T dx(const Vec4i &p) const { return geti({p.x + 1, p.y, p.z, p.w}) - geti({p.x - 1, p.y, p.z, p.w}); }
	T dy(const Vec4i &p) const { return geti({p.x, p.y + 1, p.z, p.w}) - geti({p.x, p.y - 1, p.z, p.w}); }
	T dz(const Vec4i &p) const { return geti({p.x, p.y, p.z + 1, p.w}) - geti({p.x, p.y, p.z - 1, p.w}); }
	T dw(const Vec4i &p) const { return geti({p.x, p.y, p.z, p.w + 1}) - geti({p.x, p.y, p.z, p.w - 1}); }

	T sum6(const Vec4i &p) const  // 3D neighbour 6 points
	{
		return at({p.x - 1, p.y, p.z, p.w}) + at({p.x + 1, p.y, p.z, p.w})
		     + at({p.x, p.y - 1, p.z, p.w}) + at({p.x, p.y + 1, p.z, p.w})
		     + at({p.x, p.y, p.z - 1, p.w}) + at({p.x, p.y, p.z + 1, p.w});
	}

	T geti(const Vec4i &p) const { return at(clamp(p, Vec4i(0), m_size - 1)); }

	T get(Vec4f p) const
	{
		auto lerp_x = [&](float x, int32_t y0, int32_t z0, int32_t w0) {
			auto x0 = int32_t(x);
			auto x1 = int32_t(x0) + 1;
			return lerp(geti({x0, y0, z0, w0}), geti({x1, y0, z0, w0}), x - x0);
		};

		auto lerp_xy = [&](float x, float y, int32_t z0, int32_t w0) {
			auto y0 = int32_t(y);
			auto y1 = int32_t(y0) + 1;
			return lerp(lerp_x(x, y0, z0, w0), lerp_x(x, y1, z0, w0), y - y0);
		};

		auto lerp_xyz = [&](float x, float y, float z, int32_t w0) {
			auto z0 = int32_t(z);
			auto z1 = int32_t(z0) + 1;
			return lerp(lerp_xy(x, y, z0, w0), lerp_xy(x, y, z1, w0), z - z0);
		};

		auto lerp_xyzw = [&](float x, float y, float z, float w) {
			auto w0 = int32_t(w);
			auto w1 = int32_t(w0) + 1;
			return lerp(lerp_xyz(x, y, z, w0), lerp_xyz(x, y, z, w1), w - w0);
		};

		if (m_size.w > 1) return lerp_xyzw(p.x, p.y, p.z, p.w);  // 4D
		if (m_size.z > 1) return lerp_xyz(p.x, p.y, p.z, 0);     // 3D
		if (m_size.y > 1) return lerp_xy(p.x, p.y, 0, 0);        // 2D
		return lerp_x(p.x, 0, 0, 0);                             // 1D
	}

	void apply(int32_t margin, std::function<void(const Vec4i &p, int32_t index)> func)
	{
		auto m = min(m_size - 1, Vec4i(margin));
		Vec4i p;
		for (p.w = m.w; p.w < m_size.w - m.w; p.w++) {
			for (p.z = m.z; p.z < m_size.z - m.z; p.z++) {
				for (p.y = m.y; p.y < m_size.y - m.y; p.y++) {
					auto index = indexof({m.x, p.y, p.z, p.w});
					for (p.x = m.x; p.x < m_size.x - m.x; p.x++, index++) {
						func(p, index);
					}
				}
			}
		}
	}

	void swap(Voxels<T> &v)
	{
		std::swap(m_size, v.m_size);
		std::swap(m_stride, v.m_stride);
		m_data.swap(v.m_data);
	}

	template<class T0> void copy(const T0 &v)
	{
		for (auto i = 0u; i < m_data.size(); i++) {
			m_data[i] = T(v[i]);
		}
	}

	template<class T0> void fill(const T0 &v)
	{
		for (auto i = 0u; i < m_data.size(); i++) {
			m_data[i] = T(v);
		}
	}

protected:
	Vec4i m_size = Vec4i(1);
	Vec4i m_stride = Vec4i(1);
	std::vector<T> m_data;
};
}  // namespace spu
