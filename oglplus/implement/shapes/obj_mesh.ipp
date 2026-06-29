
// #include <sstream>
#include <stdexcept>
#include <utility>

namespace spu::oglplus::shapes {

inline bool ObjMesh::load_index(
        uint32_t &value, uint32_t n_verts, std::string::const_iterator &i, std::string::const_iterator &e)
{
	bool neg = false;
	if ((i != e) && (*i == '-')) {
		neg = true;
		++i;
		while ((i != e) && ((std::isspace(*i)) != 0)) {
			++i;
		}
	}
	if ((i != e) && (*i >= '0') && (*i <= '9')) {
		value = 0;
		while ((i != e) && (*i >= '0') && (*i <= '9')) {
			value *= 10;
			value += *i - '0';
			++i;
		}
		if (neg) {
			assert(n_verts > value);
			value = n_verts - value;
		}
		return true;
	}
	return false;
}

inline bool ObjMesh::load_indices(
        VertIndices &indices, const VertIndices &counts, std::string::const_iterator &i,
        std::string::const_iterator &e)
{
	indices = VertIndices();

	while ((i != e) && ((std::isspace(*i)) != 0)) {
		++i;
	}
	if (load_index(indices.pos, counts.pos, i, e)) {
		if (i == e) {
			return true;
		}
		if (std::isspace(*i) != 0) {
			return true;
		}
		if (*i == '/') {
			++i;
			if (i == e) {
				return false;
			}
			if (*i != '/') {
				if (!load_index(indices.tex, counts.tex, i, e)) {
					return false;
				}
			}
			if (*i == '/') {
				++i;
				if (i == e) {
					return false;
				}
				if (std::isspace(*i) != 0) {
					return false;
				}
				if (!load_index(indices.nml, counts.nml, i, e)) {
					return false;
				}
			}
			return (i == e) || (std::isspace(*i) != 0);
		}
	}
	return false;
}

template<class iter_t>
inline void ObjMesh::load_meshes(const LoadingOptions &opts, iter_t names_begin, iter_t names_end, File &input)
{
	/*
	if (!input.good()) {
	        throw std::runtime_error("Obj file loader: Unable to read input.");
	}
	*/

	const double unused[3] = {0.0, 0.0, 0.0};
	std::vector<double> pos_data(unused, unused + 3);
	std::vector<double> nml_data(unused, unused + 3);
	std::vector<double> tex_data(unused, unused + 3);
	std::vector<double> mtl_data(1, 0);

	VertIndices n_attr;
	n_attr.pos = 1;
	n_attr.nml = 1;
	n_attr.tex = 1;
	n_attr.mtl = 1;
	std::vector<VertIndices> idx_data(1, VertIndices());
	m_mtl_names.emplace_back();

	std::vector<std::string> mesh_names;
	std::vector<uint32_t> mesh_offsets;
	std::vector<uint32_t> mesh_counts;

	auto curr_mtl = 0;
	std::string mtllib;

	const std::string vert_tags(" tnp");
	std::string line;

	// while (std::getline(input, line)) {
	while (input.getline(line)) {
		std::string::const_iterator b = line.begin();
		std::string::const_iterator i = b;
		std::string::const_iterator e = line.end();
		// while ((b < e) && e[-1] == '\r') {
		while ((b < e) && (e[-1] == '\r' || e[-1] == '\n')) {
			--e;
		}
		while ((i != e) && (std::isspace(*i) != 0)) {
			++i;
		}
		if (i == e) {
			continue;
		}
		if (*i == '#') {
			continue;
		}
		if (*i == 'm') {
			const char *s = "mtllib";
			if (std::find_end(i, e, s, s + 6) != i) {
				throw std::runtime_error("Obj file loader: Unknown tag at line: " + line);
			}
			i += 6;
			while ((i != e) && (std::isspace(*i) != 0)) {
				++i;
			}
			std::string::const_iterator f = i;
			while ((f != e) && (std::isspace(*f) == 0)) {
				++f;
			}
			mtllib = std::string(i, f);
		}
		else if (*i == 'u') {
			const char *s = "usemtl";
			if (std::find_end(i, e, s, s + 6) != i) {
				throw std::runtime_error("Obj file loader: Unknown tag at line: " + line);
			}
			i += 6;
			while ((i != e) && (std::isspace(*i) != 0)) {
				++i;
			}
			std::string::const_iterator f = i;
			while ((f != e) && (std::isspace(*f) == 0)) {
				++f;
			}

			std::string material;
			if (!mtllib.empty()) {
				material = mtllib + '#';
			}
			material.append(std::string(i, f));

			curr_mtl = uint32_t(m_mtl_names.size());
			m_mtl_names.push_back(material);
		}
		else if (*i == 'v') {
			++i;
			if (i == e) {
				throw std::runtime_error(
				        "Obj file loader: "
				        "Unexpected end of "
				        "line: "
				        + line);
			}
			char t = *i;
			++i;
			std::stringstream str(line.c_str() + std::distance(b, i));
			if (vert_tags.find(t) != std::string::npos) {
				double v[3] = {0.0, 0.0, 0.0};
				str >> v[0];
				str >> v[1];
				str >> v[2];
				if (t == ' ') {
					pos_data.insert(pos_data.end(), v, v + 3);
					++n_attr.pos;
				}
				if (t == 'n') {
					nml_data.insert(nml_data.end(), v, v + 3);
					++n_attr.nml;
				}
				if (t == 't') {
					tex_data.insert(tex_data.end(), v, v + 3);
					++n_attr.tex;
				}
			}
		}
		else if (*i == 'f') {
			++i;
			while ((i != e) && (std::isspace(*i) != 0)) {
				++i;
			}
			VertIndices vi1[3];
			for (auto n = 0u; n != 3; ++n) {
				if (!load_indices(vi1[n], n_attr, i, e)) {
					throw std::runtime_error(
					        "Obj file loader: Error reading "
					        "indices: "
					        + line);
				}
				vi1[n].mtl = curr_mtl;
			}
			idx_data.insert(idx_data.end(), vi1, vi1 + 3);
			VertIndices vi2[3] = {vi1[0], vi1[2], VertIndices()};
			while (load_indices(vi2[2], n_attr, i, e)) {
				vi2[2].mtl = curr_mtl;
				idx_data.insert(idx_data.end(), vi2, vi2 + 3);
				vi2[1] = vi2[2];
			}
		}
		else if (*i == 'o') {
			++i;
			while ((i != e) && (std::isspace(*i) != 0)) {
				++i;
			}
			if (!mesh_offsets.empty()) {
				mesh_counts.push_back(idx_data.size() - mesh_offsets.back());
			}
			mesh_names.emplace_back(i, e);
			mesh_offsets.push_back(idx_data.size());
		}
	}
	if (mesh_offsets.empty()) {
		if (!idx_data.empty()) {
			mesh_offsets.push_back(1);
			mesh_counts.push_back(idx_data.size() - 1);
		}
	}
	else {
		mesh_counts.push_back(idx_data.size() - mesh_offsets.back());
	}

	if (mesh_names.empty()) {
		mesh_names.emplace_back();
	}
	assert(mesh_names.size() == mesh_offsets.size());
	assert(mesh_names.size() == mesh_counts.size());

	auto ni = idx_data.size() - 1;
	auto mo = 0;

	m_pos_data.resize(ni * 3);
	m_nml_data.resize(ni * 3);
	m_tex_data.resize(ni * 3);
	m_mtl_data.resize(ni * 1);

	std::vector<uint32_t> meshes_to_load;

	if (names_begin == names_end) {
		meshes_to_load.resize(mesh_names.size());
		m_mesh_names.resize(mesh_names.size());
		for (auto m = 0u; m != mesh_names.size(); ++m) {
			meshes_to_load[m] = m;
			m_mesh_names[m] = std::move(mesh_names[m]);
		}
	}
	else {
		while (names_begin != names_end) {
			for (auto m = 0u; m != mesh_names.size(); ++m) {
				if (*names_begin == mesh_names[m]) {
					meshes_to_load.push_back(m);
					m_mesh_names.push_back(mesh_names[m]);
					break;
				}
			}
			++names_begin;
		}
	}

	for (auto l = 0u; l != meshes_to_load.size(); ++l) {
		auto m = meshes_to_load[l];
		auto ii = mesh_offsets[m];
		auto mc = mesh_counts[m];
		ni = ii + mc;
		while (ii != ni) {
			for (auto c = 0; c != 3; ++c) {
				auto oi = (ii - 1) * 3 + c;
				m_pos_data[oi] = pos_data[idx_data[ii].pos * 3 + c];
				m_nml_data[oi] = nml_data[idx_data[ii].nml * 3 + c];
				m_tex_data[oi] = tex_data[idx_data[ii].tex * 3 + c];
			}
			m_mtl_data[ii - 1] = idx_data[ii].mtl;
			++ii;
		}
		m_mesh_offsets.push_back(mo);
		m_mesh_counts.push_back(mc);
		mo += mc;
	}

	assert(m_pos_data.size() % 9 == 0);
	assert(m_pos_data.size() == m_tex_data.size());

	if (opts.load_tangents) {
		if (opts.load_tangents) {
			m_tgt_data.resize(m_pos_data.size());
		}
		if (opts.load_bitangents) {
			m_btg_data.resize(m_pos_data.size());
		}
		for (uint32_t f = 0u, nf = m_pos_data.size() / 9; f != nf; ++f) {
			for (auto v = 0u; v != 3; ++v) {
				uint32_t j[3] = {v, (v + 1) % 3, (v + 2) % 3};
				Vec3f p[3];
				Vec2f uv[3];

				for (auto k = 0; k != 3; ++k) {
					p[k]
					        = Vec3f(m_pos_data[f * 9 + j[k] * 3 + 0],
					                m_pos_data[f * 9 + j[k] * 3 + 1],
					                m_pos_data[f * 9 + j[k] * 3 + 2]);
					uv[k]
					        = Vec2f(m_tex_data[f * 9 + j[k] * 3 + 0],
					                m_tex_data[f * 9 + j[k] * 3 + 1]);
				}

				auto v0 = p[1] - p[0];
				auto v1 = p[2] - p[0];

				auto duv0 = uv[1] - uv[0];
				auto duv1 = uv[2] - uv[0];

				auto d = duv0.x * duv1.y - duv0.y * duv1.x;
				if (d != 0.0f) {
					d = 1.0f / d;
				}

				if (opts.load_tangents) {
					auto t = (duv1.y * v0 - duv0.y * v1) * d;
					auto nt = normalize(t);

					for (auto tv = 0; tv != 3; ++tv) {
						m_tgt_data[f * 9 + v * 3 + 0] = nt.x;
						m_tgt_data[f * 9 + v * 3 + 1] = nt.y;
						m_tgt_data[f * 9 + v * 3 + 2] = nt.z;
					}
				}

				if (opts.load_bitangents) {
					auto b = (duv0.x * v1 - duv1.x * v0) * d;
					auto nb = normalize(b);

					for (auto tv = 0; tv != 3; ++tv) {
						m_btg_data[f * 9 + v * 3 + 0] = nb.x;
						m_btg_data[f * 9 + v * 3 + 1] = nb.y;
						m_btg_data[f * 9 + v * 3 + 2] = nb.z;
					}
				}
			}
		}
	}
}

template<class iter_t>
inline void ObjMesh::call_load_meshes(File &input, iter_t names_begin, iter_t names_end, LoadingOptions opts)
{
	opts.load_tangents |= opts.load_bitangents;
	opts.load_bitangents |= opts.load_tangents;
	opts.load_texcoords |= opts.load_tangents;

	load_meshes(opts, std::move(names_begin), names_end, input);
}

inline bool ObjMesh::queryMeshIndex(const std::string &name, uint32_t &index) const
{
	auto p = std::find(m_mesh_names.begin(), m_mesh_names.end(), name);
	if (p == m_mesh_names.end()) {
		return false;
	}
	index = uint32_t(std::distance(m_mesh_names.begin(), p));
	return true;
}

inline uint32_t ObjMesh::getMeshIndex(const std::string &name) const
{
	auto result = 0u;
	if (!queryMeshIndex(name, result)) {
		throw std::runtime_error("ObjMesh: Unable to find index of mesh '" + name + "'");
	}
	return result;
}

inline Sphere3f ObjMesh::makeBoundingSphere() const
{
	auto min_x = m_pos_data[3];
	auto max_x = m_pos_data[3];
	auto min_y = m_pos_data[4];
	auto max_y = m_pos_data[4];
	auto min_z = m_pos_data[5];
	auto max_z = m_pos_data[5];

	for (uint32_t v = 0u, vn = m_pos_data.size() / 3; v != vn; ++v) {
		auto x = m_pos_data[v * 3 + 0];
		auto y = m_pos_data[v * 3 + 1];
		auto z = m_pos_data[v * 3 + 2];

		if (min_x > x) {
			min_x = x;
		}
		if (min_y > y) {
			min_y = y;
		}
		if (min_z > z) {
			min_z = z;
		}
		if (max_x < x) {
			max_x = x;
		}
		if (max_y < y) {
			max_y = y;
		}
		if (max_z < z) {
			max_z = z;
		}
	}

	auto c = Vec3f((min_x + max_x) * 0.5F, (min_y + max_y) * 0.5F, (min_z + max_z) * 0.5F);
	return Sphere3f(c, distance(c, Vec3f(min_x, min_y, min_z)));
}

inline std::vector<spu::SpuCommand> ObjMesh::instructions(uint32_t primitive) const
{
	std::vector<spu::SpuCommand> instr;
	for (auto m = 0u; m != m_mesh_offsets.size(); ++m) {
		spu::SpuCommand com;
		com.target = GL_ARRAY_BUFFER;
		com.mode = primitive;
		com.first = m_mesh_offsets[m];
		com.count = m_mesh_counts[m];
		com.flags = m;
		instr.push_back(com);
	}

	return instr;
}

}  // namespace spu::oglplus::shapes
