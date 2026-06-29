//
// Metaindex :
//
#pragma once
#include <smath/mesh.h>

namespace spu::gs_node::wavefront {

struct Metaindex {
	int32_t v;  // metavert index
	int32_t a;  // aux index
};

class Metavert {
private:
	struct Aux {
		int32_t base;  // metaindex base
		int32_t t;     // texcoord index
		int32_t n;     // normal index
	};
	std::vector<std::vector<Aux>> m_tables;

public:
	void clear() { m_tables.clear(); }

	Metaindex add(const int32_t vtn[3])
	{
		auto v = vtn[0];
		auto t = vtn[1];
		auto n = vtn[2];

		if (m_tables.size() <= uint32_t(v)) {
			m_tables.resize(v + 1);
		}

		auto &tables = m_tables[v];
		for (const auto &table: tables) {
			if (table.t == t && table.n == n) {
				Metaindex metaindex = {v, int32_t(&table - &tables[0])};
				return metaindex;
			}
		}
		tables.push_back({0, t, n});  // set base later
		Metaindex metaindex = {v, int32_t(tables.size()) - 1};
		return metaindex;
	}

	void get(const Metaindex &f, int32_t vtn[3])
	{
		vtn[0] = f.v;
		vtn[1] = m_tables[f.v][f.a].t;
		vtn[2] = m_tables[f.v][f.a].n;
	}

	void set(const Metaindex &f, const int32_t vtn[3])
	{
		m_tables[f.v][f.a].t = vtn[1];
		m_tables[f.v][f.a].n = vtn[2];
	}

	std::vector<Mesh::Vertex> build(
	        const std::vector<Vec3f> &vertices, const std::vector<Vec3f> &texcoords,
	        const std::vector<Vec3f> &normals)
	{
		std::vector<Mesh::Vertex> soup_vertices;

		auto is_texcoords = !texcoords.empty();
		auto is_normals = !normals.empty();

		if (!is_texcoords) {
			aux_message(1, "no texcoords. Fallback=(0,0,0)\n");
		}
		if (!is_normals) {
			aux_message(1, "no normals. Fallback=(0,0,0)\n");
		}

		auto base = 0;
		for (auto i = 0u; i < m_tables.size(); i++) {
			auto &tables = m_tables[i];
			for (auto j = 0u; j < tables.size(); j++) {
				auto &table = tables[j];

				table.base = base;

				auto p = Vec4f(vertices[i], 1);
				auto t = is_texcoords ? Vec4f(texcoords[table.t], 1) : Vec4f(0, 0, 0, 1);
				auto n = is_normals ? Vec4f(normals[table.n], 0) : Vec4f(0, 0, 0, 0);

				soup_vertices.emplace_back(Mesh::Vertex(p, t, n));
			}
			base += tables.size();
		}
		return soup_vertices;
	}

	int32_t get_index(const Metaindex &f) { return m_tables[f.v][0].base + f.a; }
};
}  // namespace spu::gs_node::wavefront
