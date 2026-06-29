//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class Plane : public Shape {
public:
	Plane() : m_point(0.0F, 0.0F, 0.0F), m_u(1.0F, 0.0F, 0.0F), m_v(0.0F, 0.0F, -1.0F) {}

	Plane(uint32_t udiv, uint32_t vdiv) noexcept
	        : m_point(0.0F, 0.0F, 0.0F), m_u(1.0F, 0.0F, 0.0F), m_v(0.0F, 0.0F, -1.0F), m_udiv(udiv),
	          m_vdiv(vdiv)
	{
		assert(udiv > 1);
		assert(vdiv > 1);
	}

	Plane(const Vec3f &u, const Vec3f &v) noexcept
	        : m_point(0.0F, 0.0F, 0.0F), m_u(u), m_v(v), m_udiv(2), m_vdiv(2)
	{
		assert(length(m_u) > 0.0F);
		assert(length(m_v) > 0.0F);
	}

	Plane(const Vec3f &p, const Vec3f &u, const Vec3f &v, uint32_t udiv, uint32_t vdiv)
	        : m_point(p), m_u(u), m_v(v), m_udiv(udiv), m_vdiv(vdiv)
	{
		assert(length(m_u) > 0.0F);
		assert(length(m_v) > 0.0F);
		assert(m_udiv != 0);
		assert(m_vdiv != 0);
	}

	const Vec3f &point() const { return m_point; }
	const Vec3f &U() const { return m_u; }
	const Vec3f &V() const { return m_v; }

	Vec3f normal() const { return normalize(cross(m_u, m_v)); }
	Vec3f tangential() const { return normalize(m_u); }
	Vec3f bitangential() const { return normalize(m_v); }
	Vec4f equation() const { return Vec4f(normal(), -dot(normal(), m_point)); }
	uint32_t faceWinding() const { return GL_CW; }

	uint32_t normals(std::vector<float> &dest) const
	{
		auto k = 0u;
		auto n = vertex_count();
		auto normal = this->normal();

		dest.resize(n * 3);
		for (auto i = 0u; i != n; ++i) {
			dest[k++] = float(normal.x);
			dest[k++] = float(normal.y);
			dest[k++] = float(normal.z);
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t tangents(std::vector<float> &dest) const
	{
		auto k = 0u;
		auto n = vertex_count();
		auto tangent = normalize(m_u);

		dest.resize(n * 3);
		for (auto i = 0u; i != n; ++i) {
			dest[k++] = float(tangent.x);
			dest[k++] = float(tangent.y);
			dest[k++] = float(tangent.z);
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t bitangents(std::vector<float> &dest) const
	{
		auto k = 0u;
		auto n = vertex_count();
		auto bitangent = normalize(m_v);

		dest.resize(n * 3);
		for (auto i = 0u; i != n; ++i) {
			dest[k++] = bitangent.x;
			dest[k++] = bitangent.y;
			dest[k++] = bitangent.z;
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t positions(std::vector<float> &dest) const
	{
		auto k = 0u;
		auto n = vertex_count();
		auto pos = Vec3f(m_point - m_u - m_v);
		auto ustep = Vec3f(m_u * (2.0 / m_udiv));
		auto vstep = Vec3f(m_v * (2.0 / m_vdiv));

		auto leap = m_udiv + 1;

		dest.resize(n * 3);
		for (auto j = 0u; j != (m_vdiv + 1); ++j) {
			Vec3f tmp = pos;
			for (auto i = 0u; i != leap; ++i) {
				dest[k++] = float(tmp.x);
				dest[k++] = float(tmp.y);
				dest[k++] = float(tmp.z);
				tmp += ustep;
			}
			pos += vstep;
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t texCoordinates(std::vector<float> &dest) const
	{
		auto k = 0u;
		auto n = vertex_count();
		auto uc = 0.0f;
		auto vc = 0.0f;
		auto ustep = 1.0f / m_udiv;
		auto vstep = 1.0f / m_vdiv;
		auto leap = m_udiv + 1;

		dest.resize(n * 2);
		for (auto j = 0u; j != (m_vdiv + 1); ++j) {
			uc = float(0);
			for (auto i = 0u; i != leap; ++i) {
				dest[k++] = float(uc);
				dest[k++] = float(vc);
				uc += ustep;
			}
			vc += vstep;
		}
		assert(k == dest.size());
		return 2;
	}

	void boundingSphere(Sphere3f &bounding_sphere) const
	{
		bounding_sphere = Sphere3f(Vec3f(m_point.x, m_point.y, m_point.z), length(m_u + m_v));
	}

	using IndexArray = std::vector<uint32_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const;

	uint32_t restartIndex() const { return 0xfffffffe; }  // not -1 (represents no restart)

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const;

	IndexArray indices(PatchesTag) const;
	std::vector<spu::SpuCommand> instructions(PatchesTag) const;

	IndexArray indices(EdgesTag) const;
	std::vector<spu::SpuCommand> instructions(EdgesTag) const;

private:
	Vec3f m_point;
	Vec3f m_u, m_v;
	uint32_t m_udiv{2}, m_vdiv{2};
	uint32_t vertex_count() const { return (m_udiv + 1) * (m_vdiv + 1); }
};

}  // namespace spu::oglplus::shapes
#include <shapes/plane.ipp>
#include <utility>
