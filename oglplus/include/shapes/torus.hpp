//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class Torus : public Shape {
private:
	double m_radius_out{1.0}, m_radius_in{0.5};
	uint32_t m_sections{36}, m_rings{24};

public:
	Torus() = default;

	Torus(double rad_out, double rad_in, uint32_t sects, uint32_t rings)
	        : m_radius_out(rad_out), m_radius_in(rad_in), m_sections(sects), m_rings(rings)
	{
	}

	uint32_t faceWinding() const { return GL_CCW; }

	uint32_t positions(std::vector<float> &dest) const
	{
		dest.resize((m_rings + 1) * (m_sections + 1) * 3);
		uint32_t k = 0;
		double r_step = (2.0 * pi<double>()) / double(m_rings);
		double s_step = (2.0 * pi<double>()) / double(m_sections);
		double r1 = m_radius_in;
		double r2 = m_radius_out - m_radius_in;

		for (auto r = 0u; r != (m_rings + 1); ++r) {
			double vx = cos(r * r_step);
			double vz = -sin(r * r_step);
			for (auto s = 0u; s != (m_sections + 1); ++s) {
				double vr = cos(s * s_step);
				double vy = sin(s * s_step);
				dest[k++] = float(vx * (r1 + r2 * (1.0 + vr)));
				dest[k++] = float(vy * r2);
				dest[k++] = float(vz * (r1 + r2 * (1.0 + vr)));
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t normals(std::vector<float> &dest) const
	{
		dest.resize((m_rings + 1) * (m_sections + 1) * 3);
		uint32_t k = 0;
		double r_step = (2.0 * pi<double>()) / double(m_rings);
		double s_step = (2.0 * pi<double>()) / double(m_sections);

		for (auto r = 0u; r != (m_rings + 1); ++r) {
			double nx = cos(r * r_step);
			double nz = -sin(r * r_step);
			for (auto s = 0u; s != (m_sections + 1); ++s) {
				double nr = cos(s * s_step);
				double ny = sin(s * s_step);
				dest[k++] = float(nx * nr);
				dest[k++] = float(ny);
				dest[k++] = float(nz * nr);
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t tangents(std::vector<float> &dest) const
	{
		dest.resize((m_rings + 1) * (m_sections + 1) * 3);
		uint32_t k = 0;
		double r_step = (2.0 * pi<double>()) / double(m_rings);

		for (auto r = 0u; r != (m_rings + 1); ++r) {
			double tx = -sin(r * r_step);
			double tz = -cos(r * r_step);
			for (auto s = 0u; s != (m_sections + 1); ++s) {
				dest[k++] = float(tx);
				dest[k++] = float(0);
				dest[k++] = float(tz);
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t bitangents(std::vector<float> &dest) const
	{
		dest.resize((m_rings + 1) * (m_sections + 1) * 3);
		uint32_t k = 0;
		double r_step = (2.0 * pi<double>()) / double(m_rings);
		double s_step = (2.0 * pi<double>()) / double(m_sections);

		double ty = 0.0;
		for (auto r = 0u; r != (m_rings + 1); ++r) {
			double tx = -sin(r * r_step);
			double tz = -cos(r * r_step);

			for (auto s = 0u; s != (m_sections + 1); ++s) {
				double ny = sin(s * s_step);
				double nr = cos(s * s_step);
				double nx = -tz * nr;
				double nz = tx * nr;

				dest[k++] = float(ny * tz - nz * ty);
				dest[k++] = float(nz * tx - nx * tz);
				dest[k++] = float(nx * ty - ny * tx);
			}
		}
		assert(k == dest.size());
		return 3;
	}

	uint32_t texCoordinates(std::vector<float> &dest) const
	{
		dest.resize((m_rings + 1) * (m_sections + 1) * 2);
		uint32_t k = 0;
		double r_step = 1.0 / double(m_rings);
		double s_step = 1.0 / double(m_sections);

		for (auto r = 0u; r != (m_rings + 1); ++r) {
			double u = r * r_step;
			for (auto s = 0u; s != (m_sections + 1); ++s) {
				double v = s * s_step;
				dest[k++] = float(u);
				dest[k++] = float(v);
			}
		}
		assert(k == dest.size());
		return 2;
	}

	void boundingSphere(Sphere3f &bounding_sphere) const
	{
		bounding_sphere = Sphere3f(ezero(), float(m_radius_out));
	}

	using IndexArray = std::vector<uint16_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const;
	IndexArray indices(QuadsTag) const;
	IndexArray indices(WithAdjacencyTag) const;

	uint16_t restartIndex() const { return 0xffff; }

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const;
	std::vector<spu::SpuCommand> instructions(QuadsTag) const;
	std::vector<spu::SpuCommand> instructions(WithAdjacencyTag) const;
};

}  // namespace spu::oglplus::shapes
#include <shapes/torus.ipp>
