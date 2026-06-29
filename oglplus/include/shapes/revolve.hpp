//
//$<<Header>>$
//

#pragma once

// #include <math/matrix.hpp>
#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class RevolveY : public Shape {
private:
	const double c_2pi = pi<double>() * 2.0;
	const std::vector<float> m_sections, m_section_factors;
	const uint32_t m_rings;

	const std::vector<Vec3f> m_positions_0, m_positions_1;
	const std::vector<Vec3f> m_normals_0, m_normals_1;
	const std::vector<Vec3f> m_tex_coords_0, m_tex_coords_1;

	float m_radius;

	static Vec3f mix(const Vec3f &a, const Vec3f &b, float factor)
	{
		if (factor < 0.0f) {
			factor = 0.0f;
		}
		if (factor > 1.0f) {
			factor = 1.0f;
		}
		return a * (1.0f - factor) + b * factor;
	}

	Vec3f get_position(uint32_t ring, uint32_t section) const
	{
		return mix(m_positions_0[ring], m_positions_1[ring], m_section_factors[section]);
	}

	Vec3f get_normal(uint32_t ring, uint32_t section) const
	{
		return mix(m_normals_0[ring], m_normals_1[ring], m_section_factors[section]);
	}

	Vec3f get_tex_coord(uint32_t ring, uint32_t section) const
	{
		return mix(m_tex_coords_0[ring], m_tex_coords_1[ring], m_section_factors[section]);
	}

	static std::vector<float> make_default_sections(uint32_t sections)
	{
		std::vector<float> result(sections + 1);
		const float s_step = 1.0f / sections;
		auto s = 0.0f;
		for (auto i = result.begin(), e = result.end(); i != e; ++i, s += s_step) {
			*i = s;
		}
		return result;
	}

	static std::vector<Vec3f> calculate_normals(
	        const std::vector<Vec3f> &pos, const std::vector<Vec3f> &nml)
	{
		if (!nml.empty()) {
			assert(pos.size() == nml.size());
			return nml;
		}
		std::vector<Vec3f> result(pos.size());

		const auto n = result.size() - 1;
		const auto tgnt = Vec3f(0.0, 0.0, -1.0);

		result[0] = normalize(cross(tgnt, pos[1] - pos[0]));
		for (auto i = 1u; i != n; ++i) {
			result[i] = normalize(cross(tgnt, pos[i + 1] - pos[i - 1]));
		}
		result[n] = normalize(cross(tgnt, pos[n] - pos[n - 1]));
		return result;
	}

	void check()
	{
		assert(m_rings > 1);
		assert(m_sections.size() > 2);
		assert(m_sections.size() == m_section_factors.size());

		assert(m_positions_0.size() == m_rings);
		assert(m_positions_1.size() == m_rings);
		assert(m_normals_0.size() == m_rings);
		assert(m_normals_1.size() == m_rings);
		assert(m_tex_coords_0.size() == m_rings);
		assert(m_tex_coords_1.size() == m_rings);
	}

	void calc_radius()
	{
		m_radius = 0;
		for (auto i = 0u; i != m_rings; ++i) {
			auto l0 = length(m_positions_0[i]);
			if (m_radius < l0) {
				m_radius = l0;
			}
			auto l1 = length(m_positions_1[i]);
			if (m_radius < l1) {
				m_radius = l1;
			}
		}
	}

public:
	using Shape::DefaultTag;

	RevolveY(
	        uint32_t sections, const std::vector<Vec3f> &positions, const std::vector<Vec3f> &normals,
	        const std::vector<Vec3f> &tex_coords)
	        : m_sections(make_default_sections(sections)), m_section_factors(m_sections.size(), 0.0f),
	          m_rings(positions.size()), m_positions_0(positions), m_positions_1(m_positions_0),
	          m_normals_0(calculate_normals(m_positions_0, normals)), m_normals_1(m_normals_0),
	          m_tex_coords_0(tex_coords), m_tex_coords_1(m_tex_coords_0)
	{
		check();
		calc_radius();
	}

	RevolveY(
	        const std::vector<float> &section_factors, const std::vector<Vec3f> &positions_0,
	        const std::vector<Vec3f> &positions_1, const std::vector<Vec3f> &normals_0,
	        const std::vector<Vec3f> &normals_1, const std::vector<Vec3f> &tex_coords_0,
	        const std::vector<Vec3f> &tex_coords_1)
	        : m_sections(make_default_sections(section_factors.size() - 1)),
	          m_section_factors(section_factors), m_rings(positions_0.size()), m_positions_0(positions_0),
	          m_positions_1(positions_1), m_normals_0(calculate_normals(m_positions_0, normals_0)),
	          m_normals_1(calculate_normals(m_positions_1, normals_1)), m_tex_coords_0(tex_coords_0),
	          m_tex_coords_1(tex_coords_1)
	{
		check();
		calc_radius();
	}

	uint32_t faceWinding() const { return GL_CW; }

	uint32_t positions(std::vector<float> &dest) const
	{
		dest.resize(m_rings * m_sections.size() * 3);
		auto k = 0u;
		for (uint32_t si = 0u, sn = m_sections.size(); si != sn; ++si) {
			const auto angle = m_sections[si] * c_2pi;
			const auto mat = Mat4f().rot("y", -angle);

			for (auto r = 0u; r != m_rings; ++r) {
				const auto in = Vec4f(get_position(r, si), 1);
				const auto out = mat * in;

				dest[k++] = out.x;
				dest[k++] = out.y;
				dest[k++] = out.z;
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t normals(std::vector<float> &dest) const
	{
		dest.resize(m_rings * m_sections.size() * 3);
		auto k = 0u;
		for (uint32_t si = 0u, sn = m_sections.size(); si != sn; ++si) {
			const auto angle = m_sections[si] * c_2pi;
			const auto mat = Mat4f().rot("y", angle);

			for (auto r = 0u; r != m_rings; ++r) {
				const auto in = get_normal(r, si);
				const auto out = mat * in;

				dest[k++] = float(out.x);
				dest[k++] = float(out.y);
				dest[k++] = float(out.z);
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t tangents(std::vector<float> &dest) const
	{
		dest.resize(m_rings * m_sections.size() * 3);
		auto k = 0u;
		const auto in = Vec4f(0.0, 0.0, -1.0, 0.0);

		for (uint32_t si = 0u, sn = m_sections.size(); si != sn; ++si) {
			const auto angle = m_sections[si] * c_2pi;
			const auto mat = Mat4f().rot("y", -angle);
			const auto out = mat * in;

			for (auto r = 0u; r != m_rings; ++r) {
				dest[k++] = out.x;
				dest[k++] = out.y;
				dest[k++] = out.z;
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t bitangents(std::vector<float> &dest) const
	{
		dest.resize(m_rings * m_sections.size() * 3);
		auto k = 0u;
		const auto target = Vec3f(0.0, 0.0, -1.0);

		for (uint32_t si = 0u, sn = m_sections.size(); si != sn; ++si) {
			const auto angle = m_sections[si] * c_2pi;
			const auto mat = Mat4f().rot("y", -angle);

			for (auto r = 0u; r != m_rings; ++r) {
				const auto normal = get_normal(r, si);
				const auto in = cross(normal, target);
				const auto out = mat * in;

				dest[k++] = out.x;
				dest[k++] = out.y;
				dest[k++] = out.z;
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t texCoordinates(std::vector<float> &dest) const
	{
		dest.resize(m_rings * m_sections.size() * 3);

		auto k = 0u;
		// const auto in = Vec4f(0.0, 0.0, -1.0, 0.0);

		for (uint32_t si = 0u, sn = m_sections.size(); si != sn; ++si) {
			const auto u_mult = m_sections[si];
			for (auto r = 0u; r != m_rings; ++r) {
				auto tc = get_tex_coord(r, si);
				dest[k++] = tc.x * u_mult;
				dest[k++] = tc.y;
				dest[k++] = tc.z;
			}
		}
		assert(k == dest.size());
		return 3;
	}

	void boundingSphere(Sphere3f &bounding_sphere) const { bounding_sphere = Sphere3f(ezero(), m_radius); }

	using IndexArray = std::vector<uint32_t>;

	uint32_t restartIndex() const { return 0xfffffffe; }  // not -1 (represents no restart)

	IndexArray indices(DefaultTag = DefaultTag()) const
	{
		const auto sn = m_sections.size() - 1;
		const auto n = sn * (2 * m_rings + 1);

		IndexArray indices(n);
		auto k = 0u;
		auto offs = 0u;
		for (auto s = 0u; s != sn; ++s) {
			for (auto r = 0u; r != m_rings; ++r) {
				indices[k++] = offs + r + m_rings;
				indices[k++] = offs + r;
			}
			offs += m_rings;
			indices[k++] = restartIndex();
		}
		assert(k == indices.size());
		return indices;
	}

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const
	{
		const auto sn = m_sections.size() - 1;
		const auto n = sn * (2 * m_rings + 1);

		spu::SpuCommand com;
		com.target = GL_ELEMENT_ARRAY_BUFFER;
		com.mode = GL_TRIANGLE_STRIP;
		com.first = uint32_t(0);
		com.count = uint32_t(n);
		com.flags = 0;

		return {com};
	}
};

}  // namespace spu::oglplus::shapes
