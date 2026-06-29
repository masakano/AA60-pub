//
// Mesh :
//
#pragma once

#include <ssys/attrs.h>
#include "mat4f.h"

namespace spu {

/// polygon mesh
class Mesh {
public:
	struct Vertex {
		Vec3f p;
		Vec3f t;
		Vec3f n;
		Vec4f c;
		Vec4f aux;

		Vertex() = default;

		Vertex(const Vec3f &p, const Vec3f &t = ezero(), const Vec3f &n = ez(),
		       const Vec4f &c = eone<Vec4f>(), const Vec3f &aux = ezero())
		        : p(p), t(t), n(n), c(c), aux(aux)
		{
		}

		explicit operator Vec3f() const { return p; }

		friend bool operator==(const Vertex &v0, const Vertex &v1)
		{
			return memcmp(&v0, &v1, sizeof(v0)) == 0;
		}

		friend Vertex operator*(const Mat4f &m, const Vertex &v0)
		{
			Vertex v1;
			v1.p = m.pers3(v0.p);
			v1.t = v0.t;
			v1.n = normalize(m * Vec4f(v0.n, 0));  // no translate
			v1.c = v0.c;
			v1.aux = v0.aux;
			return v1;
		}
	};
	static_assert(std::is_trivially_copyable<Vertex>::value, "not copyable");

	class Face final : public std::vector<int32_t> {
	public:
		Face() = default;
		Face(std::initializer_list<int32_t> list);
		int32_t at(size_t index) const;
		vector<Face> triangulate(const std::vector<Vertex> &soup_vertices) const;
	};

	Mesh() = default;
	virtual ~Mesh() = default;

	uint32_t getIndexCount() const;
	Range3f getRange() const;
	float getNotch() const;

	void pack(const std::vector<Vertex> &soup_vertices);
	void cat(const Mesh &mesh);
	void clear();
	void flipFace();
	void flipNormal();
	void generateNormals();
	void generateTexcoords();
	void singulate();
	void triangulate();
	void scaleTexcoords(const Vec3f &scale);

	Attrs &getAttrs() { return m_attrs; }
	std::int32_t &getSignature() { return m_signature; }
	std::vector<Vertex> &getVertices() { return m_vertices; }
	std::vector<Face> &getFaces() { return m_faces; }

	const Attrs &getAttrs() const { return m_attrs; }
	const std::int32_t &getSignature() const { return m_signature; }
	const std::vector<Vertex> &getVertices() const { return m_vertices; }
	const std::vector<Face> &getFaces() const { return m_faces; }

	static void postproc(const Attrs &global_attrs, Mesh &mesh);
	static void postproc(
	        const Attrs &global_attrs, std::vector<Mesh> &meshes, std::vector<Vertex> &soup_vertices);

	static void unpackAll(std::vector<Mesh> &meshes, std::vector<Vertex> &soup_vertices);
	static void remeshAll(const Attrs &attrs, std::vector<Mesh> &meshes);
	static void transform(std::vector<Vertex> &soup_vertices, const Mat4f &matrix);
	static void normalizeScale(std::vector<Vertex> &soup_vertices, float normalize_scale);
	static void alignOnXZPlane(std::vector<Vertex> &soup_vertices);

private:
	Attrs m_attrs;
	int32_t m_signature = 0;
	std::vector<Vertex> m_vertices;
	std::vector<Face> m_faces;
};

template<> size_t serialize(uint8_t *heap, bool is_dry, const Mesh::Face &face);
template<> size_t deserialize(const uint8_t *heap, Mesh::Face &face);
template<> size_t serialize(uint8_t *heap, bool is_dry, const Mesh &mesh);
template<> size_t deserialize(const uint8_t *heap, Mesh &mesh);
}  // namespace spu
