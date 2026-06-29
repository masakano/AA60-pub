//
// Mesh :
//
#include <smath/geometry.h>
#include <smath/mesh.h>

namespace spu {

std::vector<int32_t> triangulate(const std::vector<Vec3f> &points);

void remesh(
        int32_t grid, std::vector<Mesh> &meshes, std::vector<Mesh::Vertex> &soup_vertices, bool is_divide_faces,
        bool is_resample_normal, bool is_resample_texcoord, bool is_full_merge, float position_notch,
        float texcoord_notch, float edge_weight);

namespace {

// TODO : take direction into account for bump texture !!
Mat4f select_texcoord_matrix(const Vec3f &normal)
{
	Vec3f n2 = normal * normal;

	// y-up
	if (n2.x >= n2.y && n2.x >= n2.z) {
		return {0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	}
	if (n2.y >= n2.z && n2.y >= n2.x) {
		return {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	}

	return {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
}
}  // namespace

Mesh::Face::Face(std::initializer_list<int32_t> list)
{
	for (const auto &item: list) {
		push_back(item);
	}
}
int32_t Mesh::Face::at(size_t index) const
{
	using base_t = std::vector<int32_t>;
	auto n = base_t::size();
	return base_t::at((index + n) % n);
}

std::vector<Mesh::Face> Mesh::Face::triangulate(const std::vector<Vertex> &soup_vertices) const
{
	std::vector<Face> faces;
	if (size() > 3) {
		std::vector<Vec3f> points;
		std::vector<int32_t> soup_indices;
		std::vector<int32_t> indices;
		auto normal = ezero();

		for (const auto &f: *this) {
			soup_indices.push_back(f);
			normal += soup_vertices[f].n;
			points.push_back(soup_vertices[f].p);
		}
		normal = normalize(normal);
		indices = spu::triangulate(points);

		for (auto i = 0u; i + 2 < indices.size(); i += 3) {
			Face face = {
			        soup_indices[indices[i + 0]],
			        soup_indices[indices[i + 1]],
			        soup_indices[indices[i + 2]],
			};
			faces.push_back(face);
		}
	}
	else {
		faces.push_back(*this);
	}
	return faces;
}

void Mesh::pack(const std::vector<Vertex> &soup_vertices)
{
	std::vector<int32_t> index_map(soup_vertices.size(), -1);

	m_vertices.clear();
	for (auto &face: m_faces) {
		for (auto &index: face) {
			if (index_map[index] == -1) {  // not found
				index_map[index] = m_vertices.size();
				m_vertices.push_back(soup_vertices[index]);
			}
			index = index_map[index];
		}
	}
	m_vertices.shrink_to_fit();
}

void Mesh::transform(std::vector<Vertex> &soup_vertices, const Mat4f &matrix)
{
	for (auto &vert: soup_vertices) {
		vert = matrix * vert;
	}
}

void Mesh::normalizeScale(std::vector<Vertex> &soup_vertices, float normalize_scale)
{
	auto range = Range3f(soup_vertices);
	auto center = range.center();
	auto span = range.span();
	auto max_span = std::max({span.x, span.y, span.z});
	if (max_span > 0) {
		auto scale = normalize_scale / max_span;
		for (auto &vert: soup_vertices) {
			vert.p = (vert.p - center) * scale;
		}
	}
}

void Mesh::alignOnXZPlane(std::vector<Vertex> &soup_vertices)
{
	auto range = Range3f(soup_vertices);
	for (auto &vert: soup_vertices) {
		vert.p.y -= range.p0.y;
	}
}

void Mesh::unpackAll(std::vector<Mesh> &meshes, std::vector<Vertex> &soup_vertices)
{
	auto base = soup_vertices.size();
	for (auto &mesh: meshes) {
		vector_cat(soup_vertices, mesh.m_vertices);
		for (auto &face: mesh.m_faces) {
			for (auto &index: face) {
				index += base;
			}
		}
		base += mesh.m_vertices.size();
		mesh.m_vertices.clear();
		mesh.m_vertices.shrink_to_fit();
	}
}

void Mesh::cat(const Mesh &mesh)
{
	auto base = m_vertices.size();
	vector_cat(m_vertices, mesh.m_vertices);
	for (const auto &face: mesh.m_faces) {
		Face new_face;
		for (const auto &index: face) {
			new_face.push_back(index + base);
		}
		m_faces.push_back(new_face);
	}
}

void Mesh::clear()
{
	m_vertices.clear();
	m_faces.clear();
	m_vertices.shrink_to_fit();
	m_faces.shrink_to_fit();
}

void Mesh::flipFace()
{
	for (auto &face: m_faces) {
		reverse(begin(face), end(face));
	}
}

void Mesh::flipNormal()
{
	for (auto &vert: m_vertices) {
		vert.n = -vert.n;
	}
}

void Mesh::generateNormals()
{
	for (auto &vert: m_vertices) {
		vert.n = ezero();
	}
	for (auto &face: m_faces) {
		for (size_t i = 0; i < face.size(); i++) {
			auto p0 = m_vertices[face.at(i + 0)].p;
			auto p1 = m_vertices[face.at(i - 1)].p;
			auto p2 = m_vertices[face.at(i + 1)].p;
			auto n = cross(p2 - p0, p1 - p0);
			m_vertices[face[i]].n += n;
		}
	}
	for (auto &vert: m_vertices) {
		vert.n = normalize(vert.n);
	}
}

void Mesh::generateTexcoords()
{
	// assume sigulated
	for (auto &face: m_faces) {
		auto is_valid = false;
		for (auto &index: face) {
			if (length(m_vertices[index].t) > epsilon()) {
				is_valid = true;
				break;
			}
		}

		if (!is_valid) {
			auto normal = ezero();
			for (auto &index: face) {
				normal += m_vertices[index].n;
			}
			normal = normalize(normal);

			auto texcoord_matrix = select_texcoord_matrix(normal);
			for (auto &index: face) {
				auto &v = m_vertices[index];
				v.t = texcoord_matrix * v.p;
			}
		}
	}
}

void Mesh::scaleTexcoords(const Vec3f &scale)
{
	for (auto &v: m_vertices) {
		v.t = scale * v.t;
	}
}

void Mesh::triangulate()
{
	std::vector<Face> new_faces;
	for (auto &face: m_faces) {
		auto tris = face.triangulate(m_vertices);
		vector_cat(new_faces, tris);
	}
	m_faces = new_faces;
}

void Mesh::singulate()
{
	auto soup_vertices = m_vertices;
	m_vertices.clear();
	for (auto &face: m_faces) {
		for (auto &index: face) {
			m_vertices.push_back(soup_vertices[index]);
			index = m_vertices.size() - 1;
		}
	}
}

void Mesh::postproc(const Attrs &global_attrs, Mesh &mesh)
{
	std::vector<Mesh> meshes = {mesh};
	std::vector<Vertex> soup_vertices;
	postproc(global_attrs, meshes, soup_vertices);
	mesh.getFaces() = meshes.at(0).getFaces();
	mesh.getVertices() = soup_vertices;
}

void Mesh::postproc(const Attrs &global_attrs, std::vector<Mesh> &meshes, std::vector<Vertex> &soup_vertices)
{
	auto remesh_attrs = global_attrs.select("remesh.");
	if (!remesh_attrs.empty()) {
		remeshAll(remesh_attrs, meshes);
	}

	for (auto &mesh: meshes) {
		auto attrs = mesh.m_attrs;
		attrs.append(global_attrs);
		attrs = attrs.uniq();

		auto is_flip_face = attrs.get("flip_face", 0);
		auto is_flip_normal = attrs.get("flip_normal", 0);
		auto is_singulate = attrs.get("signgulate", 0);
		auto is_all_flat = attrs.get("all_flat", 0);
		auto is_gen_texcoord = attrs.get("gen_texcoord", 0);
		auto is_gen_normal = attrs.get("gen_normal", 0);
		auto texcoord_scale = Vec3f(attrs.get("texcoord_scale", vec4f_t(1.0f)));

		mesh.triangulate();

		if (is_flip_face) {
			mesh.flipFace();
		}
		if (is_flip_normal) {
			mesh.flipNormal();
		}
		if (is_singulate || is_all_flat || is_gen_texcoord) {
			mesh.singulate();
		}
		if (is_gen_normal || is_all_flat || is_gen_texcoord) {
			mesh.generateNormals();
		}
		if (is_gen_texcoord) {
			mesh.generateTexcoords();
		}
		if (!equal(texcoord_scale, eone())) {
			mesh.scaleTexcoords(texcoord_scale);
		}
	}

	unpackAll(meshes, soup_vertices);

	global_attrs.peek("fit_to_xz_plane", "use 'align_on_xz_plane/normalize_scale' instead");
	global_attrs.peek("align_to_xz_plane", "use 'align_on_xz_plane' instead");

	auto zup_to_yup = global_attrs.get("zup_to_yup", 0);
	if (zup_to_yup) {
		auto transform_matrix = Mat4f().rot("x", pi() / 2);
		transform(soup_vertices, transform_matrix);
	}
	auto normalize_scale = global_attrs.get("normalize_scale", 0.0f);
	// auto normalize_scale = global_attrs.get("normalize_scale", 0.0);
	if (normalize_scale) {
		normalizeScale(soup_vertices, normalize_scale);
	}
	auto aling_on_xz_plane = global_attrs.get("align_on_xz_plane", 0);
	if (aling_on_xz_plane) {
		alignOnXZPlane(soup_vertices);
	}
	auto transform_matrix = global_attrs.get<Mat4f *>("transform_matrix", nullptr);
	if (transform_matrix) {
		transform(soup_vertices, *transform_matrix);
	}
}

uint32_t Mesh::getIndexCount() const
{
	auto n = 0;
	for (const auto &face: m_faces) {
		n += face.size();
	}
	return n;
}

Range3f Mesh::getRange() const
{
	Range3f range;
	range.invalidate();
	for (const auto &vert: m_vertices) {
		range.expand(vert.p);
	}
	return range;
}

float Mesh::getNotch() const
{
	float sum_notch = 0;
	for (const auto &face: m_faces) {
		Range3f range;
		range.invalidate();
		for (const auto &index: face) {
			auto p = m_vertices[index].p;
			range.expand(p);
		}
		sum_notch += length(range.span());
	}
	return m_faces.empty() ? 0 : sum_notch / m_faces.size();
}

void Mesh::remeshAll(const Attrs &attrs, std::vector<Mesh> &meshes)
{
	// attrs.report("remeshAll");
	auto grid = attrs.get("grid", 0);
	auto is_resample_normal = attrs.get("resample_normal", true);
	auto is_resample_texcoord = attrs.get("resample_texcoord", true);
	auto is_full_merge = attrs.get("full_merge", false);
	auto is_divide_faces = attrs.get("divide_faces", false);
	auto position_notch = attrs.get("position_notch", 0.01f);
	auto texcoord_notch = attrs.get("texcoord_notch", 0.01f);
	auto edge_weight = attrs.get("edge_weight", 256.0f);

	if (grid == 0) return;

	for (auto &mesh: meshes) {
		mesh.triangulate();
	}
	std::vector<Vertex> soup_vertices;
	unpackAll(meshes, soup_vertices);

	aux_message(
	        0, "grid=%d divide_faces=%d resample_normal=%d resample_texcoord=%d full_merge=%d\n", grid,
	        is_divide_faces, is_resample_normal, is_resample_texcoord, is_full_merge);
	aux_message(
	        0, "position_notch=%.3f texcood_notch=%.3f edge_weight=%.3f\n", position_notch, texcoord_notch,
	        edge_weight);

	remesh(grid, meshes, soup_vertices, is_divide_faces, is_resample_normal, is_resample_texcoord,
	       is_full_merge, position_notch, texcoord_notch, edge_weight);

	for (auto &mesh: meshes) {
		mesh.pack(soup_vertices);
	}
}

template<> size_t serialize(uint8_t *heap, bool is_dry, const Mesh::Face &object)
{
	const std::vector<int32_t> &base = object;
	return serialize(heap, is_dry, base);
}

template<> size_t deserialize(const uint8_t *heap, Mesh::Face &object)
{
	std::vector<int32_t> &base = object;
	return deserialize(heap, base);
}

template<> size_t serialize(uint8_t *heap, bool is_dry, const Mesh &object)
{
	const auto &attrs = object.getAttrs();
	const auto &vertices = object.getVertices();
	const auto &faces = object.getFaces();

	auto *hp = heap;
	hp += serialize(hp, is_dry, attrs);
	hp += serialize(hp, is_dry, vertices);
	hp += serialize(hp, is_dry, faces);
	return hp - heap;
}

template<> size_t deserialize(const uint8_t *heap, Mesh &object)
{
	auto &attrs = object.getAttrs();
	auto &vertices = object.getVertices();
	auto &faces = object.getFaces();

	const auto *hp = heap;

	hp += deserialize(hp, attrs);
	hp += deserialize(hp, vertices);
	hp += deserialize(hp, faces);
	attrs.preserve();  // DO NOT FORGET

	return hp - heap;
}
}  // namespace spu
