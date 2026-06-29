//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class SpiralSphere : public Shape {
private:
	const double m_radius{1.0}, m_thickness{0.1};
	const uint32_t m_bands{4}, m_divisions{8}, m_segments{48};

	uint32_t vertex_count() const;

	void make_vectors(std::vector<float> &dest, uint32_t &k, double sign, double radius) const;
	void make_tangents(std::vector<float> &dest, uint32_t &k, double sign) const;
	void make_bitangents(std::vector<float> &dest, uint32_t &k, double sign) const;
	void make_uv_coords(std::vector<float> &dest, uint32_t &k) const;
	void make_side_verts(std::vector<float> &dest, uint32_t &k) const;
	void make_side_norms(std::vector<float> &dest, uint32_t &k) const;
	void make_side_tgts(std::vector<float> &dest, uint32_t &k) const;
	void make_side_btgs(std::vector<float> &dest, uint32_t &k) const;
	void make_side_uvs(std::vector<float> &dest, uint32_t &k) const;

public:
	SpiralSphere()

	        = default;

	SpiralSphere(double radius, double thickness, uint32_t bands, uint32_t divisions, uint32_t segments)
	        : m_radius(radius), m_thickness(thickness), m_bands(bands), m_divisions(divisions),
	          m_segments(segments)
	{
	}

	uint32_t faceWinding() const { return GL_CCW; }
	uint32_t positions(std::vector<float> &dest) const;
	uint32_t normals(std::vector<float> &dest) const;
	uint32_t tangents(std::vector<float> &dest) const;
	uint32_t bitangents(std::vector<float> &dest) const;
	uint32_t texCoordinates(std::vector<float> &dest) const;

	void boundingSphere(Sphere3f &bounding_sphere) const
	{
		bounding_sphere = Sphere3f(ezero(), m_radius + m_thickness);
	}

	using IndexArray = std::vector<uint16_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const;

	uint16_t restartIndex() const { return 0xffff; }

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const;
};

}  // namespace spu::oglplus::shapes
#include <shapes/spiral_sphere.ipp>
