//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class Cage : public Shape {
private:
	Vec3f m_size;
	Vec3f m_barw;
	Vec3f m_divs;

	static const Mat4f &face_mat(uint32_t face);

	float face_size(uint32_t face, uint32_t axis) const
	{
		auto m = face_mat(face);
		auto row = Vec3f(m.c[0].f[axis], m.c[1].f[axis], m.c[2].f[axis]);
		return std::fabs(dot(row, m_size));
	}

	float face_barw(uint32_t face, uint32_t axis) const
	{
		auto m = face_mat(face);
		auto row = Vec3f(m.c[0].f[axis], m.c[1].f[axis], m.c[2].f[axis]);
		return std::fabs(dot(row, m_barw));
	}

	uint32_t face_divs(uint32_t face, uint32_t axis) const
	{
		auto m = face_mat(face);
		auto row = Vec3f(m.c[0].f[axis], m.c[1].f[axis], m.c[2].f[axis]);
		return std::fabs(dot(row, m_divs));
	}

	static Vec3f face_vec(uint32_t face, const Vec3f &vec) { return face_mat(face) * Vec4f(vec, 0); }

	template<typename Iter> static Iter write(Iter iter, const Vec3f &vec)
	{
		*iter++ = vec.x;
		*iter++ = vec.y;
		*iter++ = vec.z;
		return iter;
	}

	uint32_t vert_count() const;
	uint32_t pri() const { return vert_count(); }
	uint32_t index_count() const;

public:
	Cage() : m_size(1, 1, 1), m_barw(0.15, 0.15, 0.15), m_divs(4, 4, 4) {}

	Cage(double xs, double ys, double zs, double xb, double yb, double zb, uint32_t xd, uint32_t yd,
	     uint32_t zd)
	        : m_size(xs, ys, zs), m_barw(xb, yb, zb), m_divs(xd, yd, zd)
	{
		assert(xs > 0.0);
		assert(ys > 0.0);
		assert(zs > 0.0);

		assert(xd > 0);
		assert(yd > 0);
		assert(zd > 0);

		assert(xs > xb * (xd - 1));
		assert(ys > yb * (yd - 1));
		assert(zs > zb * (zd - 1));
	}

	uint32_t faceWinding() const { return GL_CW; }

	using VertexAttribFunc = uint32_t (Cage::*)(std::vector<float> &) const;

	uint32_t positions(std::vector<float> &dest) const;
	uint32_t normals(std::vector<float> &dest) const;
	uint32_t tangents(std::vector<float> &dest) const;
	uint32_t texCoordinates(std::vector<float> &dest) const;

	void boundingSphere(Sphere3f &bounding_sphere) const
	{
		bounding_sphere = Sphere3f(ezero(), length(m_size));
	}

	using IndexArray = std::vector<uint32_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const;

	uint32_t restartIndex() const { return 0xfffffffe; }  // not -1 (represents no restart)

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const;
};

}  // namespace spu::oglplus::shapes
#include <shapes/cage.ipp>
