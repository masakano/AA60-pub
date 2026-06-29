//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class Sphere : public Shape {
private:
	double m_radius{1.0};
	uint32_t m_sections{18}, m_rings{12};

public:
	Sphere() = default;

	Sphere(double radius, uint32_t sections, uint32_t rings)
	        : m_radius(radius), m_sections(sections), m_rings(rings)
	{
		assert(m_radius > 0);
		assert(m_sections > 0);
		assert(m_rings > 0);
	}

	double Radius() const { return m_radius; }

	void radius(double radius)
	{
		m_radius = radius;
		assert(m_radius > 0);
	}

	uint32_t sections() const { return m_sections; }

	void Sections(uint32_t sections)
	{
		m_sections = sections;
		assert(m_sections > 0);
	}

	uint32_t rings() const { return m_rings; }

	void Rings(uint32_t rings)
	{
		m_rings = rings;
		assert(m_rings > 0);
	}

	uint32_t faceWinding() const { return GL_CCW; }

	uint32_t normals(std::vector<float> &dest) const
	{
		dest.resize(((m_rings + 2) * (m_sections + 1)) * 3);
		uint32_t k = 0;
		double r_step = (1.0 * pi<double>()) / double(m_rings + 1);
		double s_step = (2.0 * pi<double>()) / double(m_sections);

		for (auto r = 0u; r != (m_rings + 2); ++r) {
			double r_lat = cos(r * r_step);
			double r_rad = sin(r * r_step);
			for (auto s = 0u; s != (m_sections + 1); ++s) {
				dest[k++] = float(r_rad * cos(s * s_step));
				dest[k++] = float(r_lat);
				dest[k++] = float(r_rad * -sin(s * s_step));
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t tangents(std::vector<float> &dest) const
	{
		dest.resize(((m_rings + 2) * (m_sections + 1)) * 3);
		uint32_t k = 0;
		double s_step = (2.0 * pi<double>()) / double(m_sections);

		for (auto r = 0u; r != (m_rings + 2); ++r) {
			for (auto s = 0u; s != (m_sections + 1); ++s) {
				dest[k++] = float(-sin(s * s_step));
				dest[k++] = float(0);
				dest[k++] = float(-cos(s * s_step));
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t bitangents(std::vector<float> &dest) const
	{
		dest.resize(((m_rings + 2) * (m_sections + 1)) * 3);
		uint32_t k = 0;
		double r_step = (1.0 * pi<double>()) / double(m_rings + 1);
		double s_step = (2.0 * pi<double>()) / double(m_sections);

		double ty = 0.0;
		for (auto r = 0u; r != (m_rings + 2); ++r) {
			double r_lat = cos(r * r_step);
			double r_rad = sin(r * r_step);
			double ny = r_lat;
			for (auto s = 0u; s != (m_sections + 1); ++s) {
				double tx = -sin(s * s_step);
				double tz = -cos(s * s_step);
				double nx = -r_rad * tz;
				double nz = r_rad * tx;

				dest[k++] = float(ny * tz - nz * ty);
				dest[k++] = float(nz * tx - nx * tz);
				dest[k++] = float(nx * ty - ny * tx);
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t positions(std::vector<float> &dest) const
	{
		uint32_t n = normals(dest);
		if (m_radius != 1.0) {
			for (auto i = dest.begin(), e = dest.end(); i != e; ++i) {
				*i *= m_radius;
			}
		}
		return n;
	}

	uint32_t texCoordinates(std::vector<float> &dest) const
	{
		dest.resize(((m_rings + 2) * (m_sections + 1)) * 2);
		uint32_t k = 0;
		double r_step = 1.0 / double(m_rings + 1);
		double s_step = 1.0 / double(m_sections);
		for (auto r = 0u; r != (m_rings + 2); ++r) {
			double r_lat = 1.0 - r * r_step;
			for (auto s = 0u; s != (m_sections + 1); ++s) {
				dest[k++] = float(s * s_step);
				dest[k++] = float(r_lat);
			}
		}
		assert(k == dest.size());
		return 2;
	}

	void boundingSphere(Sphere3f &bounding_sphere) const { bounding_sphere = Sphere3f(ezero(), m_radius); }

	using IndexArray = std::vector<uint16_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const;

	uint16_t restartIndex() const { return 0xffff; }

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const;
};

}  // namespace spu::oglplus::shapes
#include <shapes/sphere.ipp>
