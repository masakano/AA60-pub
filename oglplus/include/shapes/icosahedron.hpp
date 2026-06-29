//
//$<<Header>>$
//

#pragma once

#include <shapes/shape.hpp>

namespace spu::oglplus::shapes {

class IcosahedronBase {
protected:
	static const uint16_t *indices();
	static const double *positions();
};

class SimpleIcosahedron : public Shape, public IcosahedronBase {
public:
	uint32_t faceWinding() const { return GL_CCW; }

	using VertexAttribFunc = uint32_t (SimpleIcosahedron::*)(std::vector<float> &) const;

	uint32_t positions(std::vector<float> &dest) const
	{
		const auto *p = IcosahedronBase::positions();
		dest.assign(p, p + 12 * 3);
		return 3;
	}

	void boundingSphere(Sphere3f &bounding_sphere) const { bounding_sphere = Sphere3f(ezero(), 1.0); }

	using IndexArray = std::vector<uint16_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const
	{
		const auto *i = IcosahedronBase::indices();
		return IndexArray(i, i + 20 * 3);
	}

	uint16_t restartIndex() const { return 0xffff; }

	std::vector<spu::SpuCommand> instructions(uint32_t mode) const;

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const
	{
		return instructions(GL_TRIANGLES);
	}
};

class Icosahedron : public Shape, public IcosahedronBase {
public:
	uint32_t faceWinding() const { return GL_CW; }

	using VertexAttribFunc = uint32_t (Icosahedron::*)(std::vector<float> &) const;

	uint32_t positions(std::vector<float> &dest) const
	{
		dest.resize(20 * 3 * 3);

		const auto *p = IcosahedronBase::positions();
		const auto *i = IcosahedronBase::indices();

		for (auto f = 0; f != 20; ++f) {
			for (auto v = 0; v != 3; ++v) {
				for (auto c = 0; c != 3; ++c) {
					dest[f * 9 + v * 3 + c] = p[i[f * 3 + v] * 3 + c];
				}
			}
		}
		return 3;
	}

	uint32_t normals(std::vector<float> &dest) const
	{
		dest.resize(20 * 3 * 3);

		const auto *p = IcosahedronBase::positions();
		const auto *i = IcosahedronBase::indices();

		for (auto f = 0; f != 20; ++f) {
			auto v0 = Vec3f(
			        p[i[f * 3 + 0] * 3 + 0], p[i[f * 3 + 0] * 3 + 1], p[i[f * 3 + 0] * 3 + 2]);

			auto v1 = Vec3f(
			        p[i[f * 3 + 1] * 3 + 0], p[i[f * 3 + 1] * 3 + 1], p[i[f * 3 + 1] * 3 + 2]);
			auto v2 = Vec3f(
			        p[i[f * 3 + 2] * 3 + 0], p[i[f * 3 + 2] * 3 + 1], p[i[f * 3 + 2] * 3 + 2]);

			auto fn = normalize(cross(v1 - v0, v2 - v0));

			for (auto v = 0; v != 3; ++v) {
				for (auto c = 0; c != 3; ++c) {
					dest[f * 9 + v * 3 + c] = fn.f[c];
				}
			}
		}
		return 3;
	}

	void boundingSphere(Sphere3f &center_and_radius) const { center_and_radius = Sphere3f(ezero(), 1.0); }

	using IndexArray = std::vector<uint16_t>;

	IndexArray indices(DefaultTag = DefaultTag()) const { return IndexArray(); }

	uint16_t restartIndex() const { return 0xffff; }

	std::vector<spu::SpuCommand> instructions(uint32_t mode) const;

	std::vector<spu::SpuCommand> instructions(DefaultTag = DefaultTag()) const
	{
		return instructions(GL_TRIANGLES);
	}
};

}  // namespace spu::oglplus::shapes
#include <shapes/icosahedron.ipp>
