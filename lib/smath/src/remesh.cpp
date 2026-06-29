//
// Remesh :
//
#include <array>
#include <algorithm>
#include <numeric>

#include <smath/mesh.h>
#include <smath/geometry.h>

namespace spu {
namespace {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

class Remesh {
public:
	float m_positionNotch = 0.01;
	float m_texcoordNotch = 0.01;
	float m_edgeWeight = 256.0;

	uint32_t totalSize();
	void createFromMesh(
	        int32_t grid_count, const std::vector<Mesh> &meshes,
	        const std::vector<Mesh::Vertex> &soup_vertices);
	void deivdeQFaces();
	void addWeight();
	void mergePositions(bool is_resample_normal, bool is_resample_texcoord, bool is_full_merge);
	void removeQFaces();
	void sendToMesh(std::vector<Mesh> &meshes, std::vector<Mesh::Vertex> &soup_vertices);

private:
	union QIndex {
		struct {
			uint32_t key;
			int32_t index;
		};
		uint64_t value;

		QIndex() = default;
		QIndex(uint32_t key, int32_t index) : key(key), index(index) {}

		bool operator==(const QIndex &qi) const { return value == qi.value; }
		auto operator<=>(const QIndex &qi) const { return value <=> qi.value; }
	};

	struct QVertex {
		Mesh::Vertex vertex;
		int32_t meshid = 0;
		int32_t seamid = -1;
		int32_t refcount = 0;
		float weight = 1.0f;
		std::vector<int32_t> adjacents;
		QVertex() = default;
		QVertex(int32_t meshid, const Mesh::Vertex &vertex) : vertex(vertex), meshid(meshid) {}
	};

	struct QSoupVertices {
		std::vector<int32_t> sorted_keys;
		std::vector<QVertex> qvertices;
	};

	using QFace = std::array<QIndex, 3>;
	using QMesh = std::vector<int32_t>;

	std::vector<QFace> m_qsoupFaces;
	std::vector<std::vector<QMesh>> m_qmeshes;
	std::map<uint32_t, QSoupVertices> m_qsoupVertices;
	int32_t m_gridCount;
	float m_gridStep;
	Vec4i m_gridSize;
	Range3f m_gridRange;

	void initRange(const std::vector<Mesh::Vertex> &soup_vertices);
	void initQMeshes(const std::vector<Mesh> &meshes, const std::vector<Mesh::Vertex> &soup_vertices);
	void removeQVertices(std::function<bool(const std::vector<QVertex> &qvertices, int32_t index)> op);
	void createSortedKeys(QSoupVertices &qsoup_vertices);
	QIndex addQVertex(int32_t meshid, const Mesh::Vertex &vertex);
	int32_t addQFace(const QFace &qface);
	void createQVertexAdjacents();
	void propagateQFaceSeamid(QMesh &mesh, std::vector<int32_t> &seamids, int32_t faceid, int32_t seamid);
	std::vector<QMesh> splitQMesh(QMesh &mesh);
	void deivdeQFace(int32_t meshid, const QFace &qface, QMesh &qfaces);

	Vec4i gridof(const Vec3f &p) const { return Vec4i((p - m_gridRange.p0) / m_gridStep); }
	uint32_t keyof(const Vec4i &gp) const { return ((gp.x * m_gridSize.y) + gp.y) * m_gridSize.z + gp.z; }
	uint32_t keyof(const Vec3f &p) const { return keyof(gridof(p)); }

	QVertex &getVertex(const QIndex &qi) { return m_qsoupVertices[qi.key].qvertices[qi.index]; }
	const QVertex &getVertex(const QIndex &qi) const
	{
		return m_qsoupVertices.at(qi.key).qvertices.at(qi.index);
	}

	QFace &getQFace(int32_t qfaceid) { return m_qsoupFaces[qfaceid]; }
	const QFace &getQFace(int32_t qfaceid) const { return m_qsoupFaces[qfaceid]; }
	QIndex &atQFace(QFace &qface, size_t index) { return qface[index % 3]; }
	const QIndex &atQFace(const QFace &qface, size_t index) const { return qface[index % 3]; }

	bool equal(const QVertex &qv0, const QVertex &qv1) const
	{
		return qv0.meshid == qv1.meshid && qv0.seamid == qv1.seamid
		    && equal_almost(qv0.vertex.p, qv1.vertex.p, Vec3f(m_gridStep * m_positionNotch))
		    && equal_almost(qv0.vertex.t, qv1.vertex.t, Vec3f(m_texcoordNotch));
	}
	bool compare(const QVertex &qv0, const QVertex &qv1) const
	{
		return greater(qv1.vertex.p, qv0.vertex.p);
	}
};

uint32_t Remesh::totalSize()
{
	auto size = 0u;
	for (auto &pair: m_qsoupVertices) {
		size += pair.second.qvertices.size();
	}
	return size;
}

void Remesh::createSortedKeys(Remesh::QSoupVertices &qsoup_vertices)
{
	qsoup_vertices.sorted_keys.resize(qsoup_vertices.qvertices.size());
	std::iota(begin(qsoup_vertices.sorted_keys), end(qsoup_vertices.sorted_keys), 0);
	std::sort(
	        begin(qsoup_vertices.sorted_keys), end(qsoup_vertices.sorted_keys),
	        [&](int32_t index0, int32_t index1) {
		        return compare(qsoup_vertices.qvertices[index0], qsoup_vertices.qvertices[index1]);
	        });
}

Remesh::QIndex Remesh::addQVertex(int32_t meshid, const Mesh::Vertex &vertex)
{
	auto qvertex = QVertex(meshid, vertex);
	auto key = keyof(qvertex.vertex.p);
	auto &qsoup_vertices = m_qsoupVertices[key];
	auto &qvertices = qsoup_vertices.qvertices;
	auto eps = Vec3f(m_gridStep * m_positionNotch);
	auto pos = std::lower_bound(
	        begin(qsoup_vertices.sorted_keys), end(qsoup_vertices.sorted_keys), qvertex,
	        [&](int32_t index0, const QVertex &qv1) { return compare(qvertices[index0], qv1); });

	for (auto it = pos; it != begin(qsoup_vertices.sorted_keys);) {
		--it;
		auto index0 = *it;
		auto &qv0 = qvertices[index0];
		if (qvertex.vertex.p.x - qv0.vertex.p.x > eps.x) {
			break;
		}
		if (equal(qv0, qvertex)) {
			return QIndex(key, index0);
		}
	}
	for (auto it = pos; it != end(qsoup_vertices.sorted_keys); ++it) {
		auto index0 = *it;
		auto &qv0 = qvertices[index0];
		if (qv0.vertex.p.x - qvertex.vertex.p.x > eps.x) {
			break;
		}
		if (equal(qv0, qvertex)) {
			return QIndex(key, index0);
		}
	}

	qvertices.push_back(qvertex);
	auto index = int32_t(qvertices.size()) - 1;
	qsoup_vertices.sorted_keys.insert(pos, index);
	return QIndex(key, index);
}

int32_t Remesh::addQFace(const QFace &qface)
{
	m_qsoupFaces.push_back(qface);
	return m_qsoupFaces.size() - 1;
}

void Remesh::initRange(const std::vector<Mesh::Vertex> &soup_vertices)
{
	m_gridRange = Range3f(soup_vertices);
	m_gridRange.grow(Vec3f(1.01));  // 1% margin
	auto span = m_gridRange.span();

	m_gridStep = std::max({span.x, span.y, span.z}) / m_gridCount;
	m_gridSize = Vec4i(span / m_gridStep + 1);
}

void Remesh::initQMeshes(const std::vector<Mesh> &meshes, const std::vector<Mesh::Vertex> &soup_vertices)
{
	m_qsoupVertices.clear();
	m_qsoupFaces.clear();
	m_qmeshes.clear();
	for (auto &mesh: meshes) {
		auto meshid = &mesh - &meshes[0];
		QMesh qfaces;
		for (auto &face: mesh.getFaces()) {
			assert(face.size() == 3);  // triangle only

			auto qi0 = addQVertex(meshid, soup_vertices[face[0]]);
			auto qi1 = addQVertex(meshid, soup_vertices[face[1]]);
			auto qi2 = addQVertex(meshid, soup_vertices[face[2]]);
			QFace qface = {qi0, qi1, qi2};

			qfaces.push_back(addQFace(qface));
		}
		m_qmeshes.push_back({qfaces});
	}
}

void Remesh::createFromMesh(
        int32_t grid_count, const std::vector<Mesh> &meshes, const std::vector<Mesh::Vertex> &soup_vertices)
{
	m_gridCount = grid_count;
	initRange(soup_vertices);
	initQMeshes(meshes, soup_vertices);
	createQVertexAdjacents();

	for (auto &qmesh: m_qmeshes) {
		assert(qmesh.size() == 1);
		qmesh = splitQMesh(qmesh[0]);
		// printf("seams : %ld\n", qmesh.size());
	}
	createQVertexAdjacents();
}

void Remesh::deivdeQFace(int32_t meshid, const QFace &qface, Remesh::QMesh &qfaces)
{
	bool is_divided = false;

	auto qi0 = atQFace(qface, 0);
	auto qi1 = atQFace(qface, 1);
	auto qi2 = atQFace(qface, 2);

	auto qv0 = getVertex(qi0);
	auto qv1 = getVertex(qi1);
	auto qv2 = getVertex(qi2);

	auto d0 = distance(qv0.vertex.p, qv1.vertex.p);
	auto d1 = distance(qv1.vertex.p, qv2.vertex.p);
	auto d2 = distance(qv2.vertex.p, qv0.vertex.p);

	auto max_i = (d0 > d1 && d0 > d2) ? 0 : (d1 > d2 && d1 > d0) ? 1 : 2;
	auto max_d = (d0 > d1 && d0 > d2) ? d0 : (d1 > d2 && d1 > d0) ? d1 : d2;

	if (max_d > m_gridStep) {
		auto qi0 = atQFace(qface, max_i + 0);
		auto qi1 = atQFace(qface, max_i + 1);
		auto qi2 = atQFace(qface, max_i + 2);

		auto &qv0 = getVertex(qi0);
		auto &qv1 = getVertex(qi1);

		auto &v0 = qv0.vertex;
		auto &v1 = qv1.vertex;

		Mesh::Vertex v3 = {
		        (v0.p + v1.p) * 0.5,
		        (v0.t + v1.t) * 0.5,
		        normalize(v0.n + v1.n),
		        (v0.c + v1.c) * 0.5,
		        v0.aux,
		};

		auto qi3 = addQVertex(meshid, v3);

		QFace new_qface0 = {qi0, qi3, qi2};
		QFace new_qface1 = {qi3, qi1, qi2};

		deivdeQFace(meshid, new_qface0, qfaces);
		deivdeQFace(meshid, new_qface1, qfaces);

		is_divided = true;
	}

	if (!is_divided) {
		qfaces.push_back(addQFace(qface));
	}
}

void Remesh::createQVertexAdjacents()
{
	for (auto &pair: m_qsoupVertices) {
		for (auto &qvertex: pair.second.qvertices) {
			qvertex.adjacents.clear();
			qvertex.seamid = -1;
		}
	}
	for (auto &qmeshes: m_qmeshes) {
		auto meshid = &qmeshes - &m_qmeshes[0];
		for (auto &qmesh: qmeshes) {
			auto seamid = &qmesh - &qmeshes[0];
			for (auto &qfaceid: qmesh) {
				auto &face = getQFace(qfaceid);
				for (auto &qi: face) {
					auto &qvertex = getVertex(qi);
					qvertex.meshid = meshid;
					qvertex.seamid = seamid;
					qvertex.adjacents.push_back(qfaceid);
				}
			}
		}
	}
}

void Remesh::propagateQFaceSeamid(QMesh &mesh, std::vector<int32_t> &seamids, int32_t faceid0, int32_t seamid)
{
	std::vector<int32_t> faceids = {mesh[faceid0]};
	while (!faceids.empty()) {
		auto qfaceid = faceids.back();
		faceids.pop_back();
		if (seamids[qfaceid] != -1) {
			continue;
		}

		seamids[qfaceid] = seamid;
		auto &face = getQFace(qfaceid);
		for (auto &qi: face) {
			auto &qvertex = getVertex(qi);
			qvertex.seamid = seamid;
			for (auto adjacent_faceid: qvertex.adjacents) {
				if (seamids[adjacent_faceid] == -1) {
					faceids.push_back(adjacent_faceid);
				}
			}
		}
	}
}

std::vector<Remesh::QMesh> Remesh::splitQMesh(Remesh::QMesh &mesh)
{
	std::vector<int32_t> seamids(m_qsoupFaces.size(), -1);
	auto new_seamid = 0;
	for (auto faceid = 0; faceid < int32_t(mesh.size()); faceid++) {
		if (seamids[mesh[faceid]] == -1) {
			propagateQFaceSeamid(mesh, seamids, faceid, new_seamid);
			++new_seamid;
		}
	}

	std::vector<QMesh> results(new_seamid);
	for (auto faceid: mesh) {
		assert(seamids[faceid] >= 0);
		results[seamids[faceid]].push_back(faceid);
	}
	return results;
}

void Remesh::deivdeQFaces()
{
	for (auto &qmesh: m_qmeshes) {
		for (auto &qfaces: qmesh) {
			QMesh new_qfaces;
			for (auto qfaceid: qfaces) {
				deivdeQFace(&qmesh - &m_qmeshes[0], getQFace(qfaceid), new_qfaces);
			}
			qfaces = new_qfaces;
		}
	}
	createQVertexAdjacents();
}

void Remesh::addWeight()
{
	auto eps = Vec3f(m_gridStep * m_positionNotch);
	for (auto &pair: m_qsoupVertices) {
		auto &qvertices = pair.second.qvertices;
		auto &sorted_keys = pair.second.sorted_keys;

		for (auto i = 0; i < int32_t(sorted_keys.size()); i++) {
			auto &qv0 = qvertices[sorted_keys[i]];
			auto &p0 = qv0.vertex.p;
			for (auto j = i + 1; j < int32_t(sorted_keys.size()); j++) {
				auto &qv1 = qvertices[sorted_keys[j]];
				auto &p1 = qv1.vertex.p;
				if (p1.x - p0.x > eps.x) {
					break;
				}
				if (abs(p1.y - p0.y) > eps.y || abs(p1.z - p0.z) > eps.z) {
					continue;
				}
				qv0.weight = m_edgeWeight;
				qv1.weight = m_edgeWeight;
			}
		}
	}
}

void Remesh::mergePositions(bool is_resample_normal, bool is_resample_texcoord, bool is_full_merge)
{
	struct Average {
		Vec3f p = Vec3f(0);
		Vec3f n = Vec3f(0);
		Vec3f t = Vec3f(0);
		float w = 0;

		void add(const Vec3f &p, const Vec3f &n, const Vec3f &t, float w)
		{
			this->p += p * w;
			this->n += n * w;
			this->t += t * w;
			this->w += w;
		}
		void average()
		{
			p /= w;
			n = normalize(n);
			t /= w;
		}
	};

	auto count = 0;
	for (auto &pair: m_qsoupVertices) {
		printf("      merge %d / %ld\r", count++, m_qsoupVertices.size());
		fflush(stdout);

		auto &qvertices = pair.second.qvertices;
		auto full_p = Vec4f(0);
		auto full_n = Vec3f(0);

		std::vector<std::vector<Average>> seam_averages(m_qmeshes.size());
		std::vector<Average> mesh_averages(m_qmeshes.size());

		for (auto meshid = 0u; meshid < m_qmeshes.size(); meshid++) {
			seam_averages[meshid].resize(m_qmeshes[meshid].size());
		}
		for (auto &qvertex: qvertices) {
			auto meshid = qvertex.meshid;
			auto seamid = qvertex.seamid;
			auto &mesh_average = mesh_averages.at(meshid);
			auto &seam_average = seam_averages.at(meshid).at(seamid);
			auto w = qvertex.weight;
			auto &vertex = qvertex.vertex;

			full_p += Vec4f(vertex.p, 1) * w;
			full_n += vertex.n * w;

			mesh_average.add(vertex.p, vertex.n, vertex.t, w);
			seam_average.add(vertex.p, vertex.n, vertex.t, w);
		}
		full_p /= full_p.w;
		full_n = normalize(full_n);

		for (auto &average: mesh_averages) {
			average.average();
		}

		for (auto &averages: seam_averages) {
			for (auto &average: averages) {
				average.average();
			}
		}

		for (auto &qvertex: qvertices) {
			auto meshid = qvertex.meshid;
			auto seamid = qvertex.seamid;
			auto &mesh_average = mesh_averages.at(meshid);
			auto &seam_average = seam_averages.at(meshid).at(seamid);

			// qvertex.vertex.p = is_full_merge ? full_p : Vec4f(seam_average.p, 1);
			qvertex.vertex.p = is_full_merge ? full_p : Vec4f(mesh_average.p, 1);
			if (is_resample_normal) {
				// qvertex.vertex.n = is_full_merge ? full_n : seam_average.n;
				qvertex.vertex.n = is_full_merge ? full_n : mesh_average.n;
			}
			if (is_resample_texcoord) {
				qvertex.vertex.t = seam_average.t;
			}
		}
	}
}

void Remesh::removeQVertices(std::function<bool(const std::vector<QVertex> &qvs, int32_t index)> op)
{
	std::map<QIndex, QIndex> index_map;

	for (auto &pair: m_qsoupVertices) {
		auto key = pair.first;
		auto &qvertices = pair.second.qvertices;
		std::vector<QVertex> new_qvertices;
		for (auto index = 0; index < int32_t(qvertices.size()); index++) {
			if (!op(qvertices, index)) {
				new_qvertices.push_back(qvertices[index]);
			}
			index_map[QIndex(key, index)] = QIndex(key, new_qvertices.size() - 1);
		}
		pair.second.qvertices = new_qvertices;
		createSortedKeys(pair.second);
	}
	for (auto &qmesh: m_qmeshes) {
		for (auto &qfaces: qmesh) {
			for (auto qfaceid: qfaces) {
				auto &qface = getQFace(qfaceid);
				for (auto &index: qface) {
					index = index_map.at(index);
				}
			}
		}
	}
}

void Remesh::removeQFaces()
{
	for (auto &pair: m_qsoupVertices) {
		for (auto &qvertex: pair.second.qvertices) {
			qvertex.refcount = 0;
		}
	}

	auto face_count = 0;
	auto new_face_count = 0;
	for (auto &qmesh: m_qmeshes) {
		for (auto &qfaces: qmesh) {
			QMesh new_qfaces;
			for (auto qfaceid: qfaces) {
				auto &qface = getQFace(qfaceid);
				auto &qi0 = atQFace(qface, 0);
				auto &qi1 = atQFace(qface, 1);
				auto &qi2 = atQFace(qface, 2);

				auto &qv0 = getVertex(qi0);
				auto &qv1 = getVertex(qi1);
				auto &qv2 = getVertex(qi2);

				auto &p0 = qv0.vertex.p;
				auto &p1 = qv1.vertex.p;
				auto &p2 = qv2.vertex.p;

				auto area = length(cross(p1 - p0, p2 - p0));
				if (area > epsilon()) {
					new_qfaces.push_back(qfaceid);
					qv0.refcount++;
					qv1.refcount++;
					qv2.refcount++;
				}
			}
			aux_message(
			        1, "    faces: %5.2f%% : %ld/%ld \n",
			        float(new_qfaces.size()) / float(qfaces.size()) * 100, new_qfaces.size(),
			        qfaces.size());

			face_count += qfaces.size();
			new_face_count += new_qfaces.size();
			qfaces = new_qfaces;
		}
	}
	aux_message(
	        0, "total faces=%d->%d (%5.2f%%) grids=%ld\n", face_count, new_face_count,
	        float(new_face_count) / float(face_count) * 100, m_qsoupVertices.size());

	auto size0 = totalSize();
	auto op0 = [](const std::vector<QVertex> &qvertices, int32_t index) {
		return qvertices[index].refcount == 0;
	};
	removeQVertices(op0);

	auto size1 = totalSize();
	auto op1 = [&](const std::vector<QVertex> &qvertices, int32_t index0) {
		for (auto index = 0; index < index0; index++) {
			if (equal(qvertices[index0], qvertices[index])) {
				return true;
			}
		}
		return false;
	};
	removeQVertices(op1);
	auto size2 = totalSize();
	aux_message(
	        0, "total vertex: %d->%d->%d (%5.2f%%) \n", size0, size1, size2,
	        float(size2) / float(size0) * 100);
}

void Remesh::sendToMesh(std::vector<Mesh> &meshes, std::vector<Mesh::Vertex> &soup_vertices)
{
	std::map<uint32_t, int32_t> offsets;

	soup_vertices.clear();
	for (auto &pair: m_qsoupVertices) {
		offsets[pair.first] = soup_vertices.size();
		for (auto &qvertex: pair.second.qvertices) {
			// qvertex.vertex.n = qvertex.weight > 1.0 ? Vec3f(1, 0, 0) : Vec3f(0, 1, 0);
			soup_vertices.push_back(qvertex.vertex);
		}
	}
	meshes.resize(m_qmeshes.size());  // don't clear
	for (auto i = 0u; i < m_qmeshes.size(); i++) {
		auto &mesh = meshes[i];

		auto &faces = mesh.getFaces();
		faces.clear();

		for (auto &qfaces: m_qmeshes[i]) {
			for (auto qfaceid: qfaces) {
				auto &qface = getQFace(qfaceid);
				auto qi0 = atQFace(qface, 0);
				auto qi1 = atQFace(qface, 1);
				auto qi2 = atQFace(qface, 2);

				auto i0 = offsets[qi0.key] + qi0.index;
				auto i1 = offsets[qi1.key] + qi1.index;
				auto i2 = offsets[qi2.key] + qi2.index;

				Mesh::Face face = {i0, i1, i2};
				faces.emplace_back(face);
			}
		}
	}
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
}  // namespace
void remesh(
        int32_t grid, std::vector<Mesh> &meshes, std::vector<Mesh::Vertex> &soup_vertices, bool is_divide_faces,
        bool is_resample_normal, bool is_resample_texcoord, bool is_full_merge, float position_notch,
        float texcoord_notch, float edge_weight)
{
	Remesh remesh;
	remesh.m_positionNotch = position_notch;
	remesh.m_texcoordNotch = texcoord_notch;
	remesh.m_edgeWeight = edge_weight;

	remesh.createFromMesh(grid, meshes, soup_vertices);

	if (is_divide_faces) {
		remesh.deivdeQFaces();
	}
	remesh.addWeight();
	remesh.mergePositions(is_resample_normal, is_resample_texcoord, is_full_merge);
	remesh.removeQFaces();
	remesh.sendToMesh(meshes, soup_vertices);
}
}  // namespace spu
