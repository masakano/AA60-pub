//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class WickerTorus : public Shape {
private:
	const double m_radius_out{1.0}, m_radius_in{0.5}, m_thickness{0.005};
	const double m_r_slip_coef{0.25}, m_s_slip_coef{0.40};
	const uint32_t m_sections{24}, m_rings{24};

public:
	WickerTorus() = default;

	WickerTorus(double rad_out, double rad_in, double thickness, uint32_t sects, uint32_t rings)
	        : m_radius_out(rad_out), m_radius_in(rad_in), m_thickness(thickness), m_sections(sects),
	          m_rings(rings)
	{
		assert(m_sections % 2 == 0);
		assert(m_rings % 2 == 0);
		assert(m_thickness > 0.0);
		assert(m_thickness < m_radius_in);
		assert(m_radius_in < m_radius_out);
	}

	uint32_t faceWinding() const { return GL_CW; }
	uint32_t positions(std::vector<float> &dest) const;
	uint32_t normals(std::vector<float> &dest) const;
	uint32_t tangents(std::vector<float> &dest) const;
	uint32_t bitangents(std::vector<float> &dest) const;
	uint32_t texCoordinates(std::vector<float> &dest) const;

	void boundingSphere(Sphere3f &bounding_sphere) const
	{
		bounding_sphere = Sphere3f(ezero(), m_radius_out + m_thickness);
	}

	using IndexArray = std::vector<uint32_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const { return IndexArray(); }

	uint32_t restartIndex() const { return 0xfffffffe; }  // not -1 (represents no restart)

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const;

	IndexArray indices(EdgesTag) const;

	std::vector<spu::SpuCommand> instructions(EdgesTag) const;
};

}  // namespace spu::oglplus::shapes
#include <shapes/wicker_torus.ipp>
