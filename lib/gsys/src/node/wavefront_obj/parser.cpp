//
// Parser :
//
#include "parser.h"
#include "metavert.h"
#include <smath/geometry.h>

namespace spu::gs_node::wavefront {
namespace {
const char *at(const std::vector<std::string> &list, uint32_t i, const char *def = "0")
{
	return i < list.size() ? list[i].c_str() : def;
}

const char *at(const std::vector<const char *> &list, uint32_t i, const char *def = "0")
{
	return i < list.size() ? list[i] : def;
}

int32_t mystoi(const std::string &str)
{
	if (str == "true") return 1;
	if (str == "false") return 0;
	return std::stoi(str);
}
}  // namespace

void Parser::readMtllib()
{
	const auto mtllib_path = m_current.top().file;
	const auto mtllib_dir = mtllib_path.parent_path();

	File file(mtllib_path, "r");
	m_paths.push_back(mtllib_path);

	std::string matname;
	auto matid = 0;
	while (file.getline(m_current.top().line)) {
		auto is_suspicious = false;
		auto list = extract_from_string(m_current.top().line, " \t\r\n", "", true);  // strip quate
		if (list.empty()) {
			continue;
		}
		const auto *key = list[0].c_str();

		auto it = c_blacklist.find(list[0]);
		if (it != end(c_blacklist)) {
			aux_message(
			        0, "'%s': deprecated (use '%s' instead)\n", it->first.c_str(),
			        it->second.c_str());
		}
		else if (list[0] == "newmtl") {
			is_suspicious |= list.size() != 2;
			matname = at(list, 1, "");
			m_mtllibs[matname].emplace_back("name", matname);
			m_mtllibs[matname].emplace_back("matid", matid++);
		}
		else if (vector_is_find(c_f4list, list[0])) {
			is_suspicious = list.size() != 5;
			m_mtllibs[matname].emplace_back(
			        key, Vec4f(std::stod(at(list, 1)), std::stod(at(list, 2)),
			                   std::stod(at(list, 3)), std::stod(at(list, 4, "1.0"))));
		}
		else if (vector_is_find(c_f3list, list[0])) {
			is_suspicious = list.size() != 4;
			m_mtllibs[matname].emplace_back(
			        key,
			        Vec3f(std::stod(at(list, 1)), std::stod(at(list, 2)), std::stod(at(list, 3))));
		}
		else if (vector_is_find(c_f2list, list[0])) {
			is_suspicious = list.size() != 3;
			m_mtllibs[matname].emplace_back(
			        key, Vec2f(std::stod(at(list, 1)), std::stod(at(list, 2))));
		}
		else if (vector_is_find(c_f1list, list[0])) {
			is_suspicious |= list.size() != 2;
			m_mtllibs[matname].emplace_back(key, std::stof(at(list, 1)));
		}
		else if (vector_is_find(c_i1list, list[0])) {
			is_suspicious |= list.size() != 2;
			m_mtllibs[matname].emplace_back(key, mystoi(at(list, 1)));
		}
		else {
			is_suspicious |= list.size() != 2;
			m_mtllibs[matname].emplace_back(key, at(list, 1));
		}
		if (is_suspicious) {
			aux_message(
			        0, "%s: argunment count mismatch at '%s'\n", mtllib_path.c_str(),
			        list.at(0).c_str());
		}
		m_mtllibs[matname].preserve();
	}
}

void Parser::readVertex()
{
	File file(m_current.top().file, "r");
	m_paths.push_back(m_current.top().file);

	std::string usemtl;

	m_metavert->clear();
	m_objVertices["v"].clear();
	m_objVertices["vt"].clear();
	m_objVertices["vn"].clear();

	while (file.getline(m_current.top().line)) {
		auto list = fastExtract(m_current.top().line);

		if (list.empty()) {
			continue;
		}

		auto &key = list[0];
		std::string keystr = key;
		if (vector_is_find(c_vertlist, keystr)) {
			m_objVertices[key].emplace_back(
			        std::stod(at(list, 1)), std::stod(at(list, 2)), std::stod(at(list, 3)));
		}

		else if (keystr == "f") {
			std::vector<Metaindex> face(list.size() - 1);
			for (auto i = 0u; i < list.size() - 1; i++) {
				int32_t k = 0;
				int32_t s = 1;
				int32_t x = 0;

				int32_t vtn[4] = {0, 0, 0, 0};  // v[3] for sentinel

				for (const char *p = list[i + 1]; *p != 0; p++) {
					switch (*p) {
					case '/':
						assert(k < 4);
						vtn[k++] = s > 0 ? x - 1 : -x;
						x = 0;
						s = 1;
						break;
					case '-': s = -1; break;
					default: x = x * 10 + (*p - '0'); break;
					}
				}
				vtn[k] = s > 0 ? x - 1 : -x;

				if (vtn[0] < 0) vtn[0] += m_objVertices["v"].size();
				if (vtn[1] < 0) vtn[1] += m_objVertices["vt"].size();
				if (vtn[2] < 0) vtn[2] += m_objVertices["vn"].size();

				face[i] = m_metavert->add(vtn);
			}
			m_objFaces.push_back(face);
		}
		else if (keystr == "g") {
			m_groups.emplace_back(at(list, 1, ""), m_objFaces.size());
			m_groups.back().usemtl = usemtl;
		}
		else if (keystr == "mtllib") {
			assert(list.size() > 1);
			const auto path = m_current.top().file.parent_path() / list.at(1);

			if (m_readMtllibs.find(path) == std::end(m_readMtllibs)) {
				m_readMtllibs.insert(path);
				m_current.push(path);
				readMtllib();
				m_current.pop();
			}
		}
		else if (keystr == "usemtl") {
			assert(list.size() > 1);

			if (m_groups.empty()) {
				m_groups.emplace_back("", m_objFaces.size());
			}
			if (!m_groups.back().usemtl.empty()) {
				m_groups.emplace_back(m_groups.back().name, m_objFaces.size());
			}
			m_groups.back().usemtl = usemtl = list.at(1);
		}
		else if (vector_is_find(c_skiplist, keystr)) {
			/* do nothing */
		}
		else {
			aux_message(1, "unsupported [%s]\n", key);
		}
	}
	aux_message(
	        1, "read complete. (faces=%d,vert=%d,texc=%d,norm=%d)\n", int(m_objFaces.size()),
	        int(m_objVertices["v"].size()), int(m_objVertices["vt"].size()),
	        int(m_objVertices["vn"].size()));
}

void Parser::load(const std::filesystem::path &name)
{
	assert(m_metavert == nullptr);
	m_metavert = new Metavert();  // pimpl

	// read
	{
		m_current.push(name);
		readVertex();
	}

	// make group
	{
		if (m_groups.empty()) {
			m_groups.emplace_back("", 0);
		}

		for (auto i = 0u; i < m_groups.size() - 1; i++) {
			m_groups[i].ranges[0].second = m_groups[i + 1].ranges[0].first;
		}
		m_groups.back().ranges[0].second = m_objFaces.size();

		std::vector<Group> new_groups;
		for (const auto &group: m_groups) {
			if (group.ranges[0].first == group.ranges[0].second) continue;
			auto op = [&](const Group &g) { return g.usemtl == group.usemtl; };
			auto ip = vector_find_if(new_groups, op);
			if (ip != end(new_groups)) {
				ip->ranges.insert(end(ip->ranges), begin(group.ranges), end(group.ranges));
			}
			else {
				new_groups.push_back(group);
			}
		}
		m_groups = new_groups;
	}

	// set soup
	std::vector<Mesh::Vertex> soup_vertices;
	std::vector<Mesh::Face> soup_faces;
	{
		soup_vertices = m_metavert->build(m_objVertices["v"], m_objVertices["vt"], m_objVertices["vn"]);
		for (const auto &obj_face: m_objFaces) {
			Mesh::Face face;
			for (const auto &index: obj_face) {
				face.push_back(m_metavert->get_index(index));
			}
			soup_faces.push_back(face);
		}
	}

	// build mesh
	{
		aux_message(1, "pack \"%s\"\n", m_current.top().file.c_str());
		m_meshes.clear();

		for (auto &group: m_groups) {
			Mesh mesh;
			mesh.getAttrs() = m_mtllibs[group.usemtl];

			auto &faces = mesh.getFaces();
			for (auto face_index = 0u; face_index < soup_faces.size(); face_index++) {
				for (auto &range: group.ranges) {
					if (range.first <= face_index && face_index < range.second) {
						faces.push_back(soup_faces[face_index]);
						break;
					}
				}
			}
			mesh.pack(soup_vertices);
			m_meshes.emplace_back(mesh);
		}
	}

	delete m_metavert;
	m_metavert = nullptr;
}

std::vector<const char *> Parser::fastExtract(const std::string &line)
{
	std::vector<const char *> list;

	bool is_blank = true;
	size_t idx = 0;

	for (const auto &c: line) {
		assert(idx < m_linebuf.size());
		auto &b = m_linebuf[idx];

		if ((b = c) == '#') {
			break;
		}
		if (isascii(b) && isspace(b) != 0) {
			is_blank = true;
			b = 0;
		}
		else if (is_blank) {
			is_blank = false;
			list.push_back(&b);
		}
		idx++;
	}
	m_linebuf[idx] = 0;
	return list;
}

}  // namespace spu::gs_node::wavefront
