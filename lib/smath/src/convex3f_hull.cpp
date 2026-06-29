//
// Convex3Hull :
//
#include <ssys/random_generator.h>
#include <smath/geometry.h>

namespace spu {
namespace convex_hull {
class Convex3Hull {
public:
	using iterator = std::vector<Vec3f>::iterator;

	struct Face {
		iterator it0, it1, it2;
		Plane3f plane;

		Face(iterator it0, iterator it1, iterator it2) : it0(it0), it1(it1), it2(it2)
		{
			plane = Plane3f(*it0, *it1, *it2);
		}
	};

	struct Edge {
		iterator it0, it1;
		Edge(const iterator &it0, const iterator &it1) : it0(it0), it1(it1) {}
		bool operator==(const Edge &e)
		{
			return ((it0 == e.it0 && it1 == e.it1) || (it0 == e.it1 && it1 == e.it0));
		}
	};

	RandomGenerator<float> frand;

	Vec3f m_center;
	std::vector<Face> m_faces;
	std::vector<Face> m_innerFaces;
	std::vector<Face> m_outerFaces;
	std::vector<Edge> m_edges;
	std::vector<Vec3f> m_points;
	std::vector<iterator> m_alives;
	std::vector<std::vector<int32_t>> m_outputs;

	Vec4f m_sumFace;
	int32_t m_maxCount;
	RandomGenerator<float> m_frand;

	Convex3Hull(const std::vector<Vec3f> &points, uint32_t max_count) : m_maxCount(max_count)
	{
		m_points = points;
	}

	void addFirstFaces(const std::vector<Face> &faces)
	{
		m_sumFace = ezero<Vec4f>();
		for (auto &face: faces) {
			m_sumFace += Vec4f(*face.it0, 1);
			m_sumFace += Vec4f(*face.it1, 1);
			m_sumFace += Vec4f(*face.it2, 1);
		}
		for (auto face: faces) {  // copy
			m_faces.push_back(face);
			auto &eq = m_faces.back().plane.eq;
			if (dot(eq, m_sumFace) > 0) {
				eq = -eq;
			}
		}
	}

	void addFace(const Face &face)
	{
		m_sumFace += Vec4f(*face.it0, 1);
		m_sumFace += Vec4f(*face.it1, 1);
		m_sumFace += Vec4f(*face.it2, 1);

		m_faces.push_back(face);
		auto &eq = m_faces.back().plane.eq;
		if (dot(eq, m_sumFace) > 0) {
			eq = -eq;
		}
	}

	auto removeFace(std::vector<Face>::iterator fit)
	{
		m_sumFace -= Vec4f(*fit->it0, 1);
		m_sumFace -= Vec4f(*fit->it1, 1);
		m_sumFace -= Vec4f(*fit->it2, 1);
		return m_faces.erase(fit);
	}

	void exec()
	{
		initAlives();
		initTetra();

		int32_t count;
		for (count = 0; m_alives.size() > 0 && count < m_maxCount; count++) {
			expand();
		}
		// printf("prune: %d/%d: left=%ld\n", count, m_maxCount, m_alives.size());

		for (auto &face: m_faces) {
			int32_t index0 = face.it0 - begin(m_points);
			int32_t index1 = face.it1 - begin(m_points);
			int32_t index2 = face.it2 - begin(m_points);

			std::vector<int32_t> output = {index0, index1, index2};
			m_outputs.emplace_back(output);
		}
	}

	auto support(const Vec3f dir)
	{
		auto det = [&](const iterator it) { return dot(dir, *it); };
		return vector_max(m_alives, det);
	}

	template<class T> auto maxPrune(T det)
	{
		auto ait = vector_max(m_alives, det);
		auto ret = *ait;
		m_alives.erase(ait);
		return ret;
	}

	auto supportPrune(const Vec3f dir)
	{
		auto det = [&](const iterator it) { return dot(dir, *it); };
		return maxPrune(det);
	}

	auto farestPrune(const Vec3f &point)
	{
		auto det = [&](const iterator it) { return length(*it - point); };
		return maxPrune(det);
	}

	auto farestPrune(const Edge &edge)
	{
		const auto &p0 = *edge.it0;
		const auto &p1 = *edge.it1;
		const auto dir = normalize(p1 - p0);

		auto det = [&](const iterator it) {
			auto dp = *it - p0;
			return length(dp - dir * dot(dir, dp));
		};
		return maxPrune(det);
	}

	auto farestPrune(const Face &face)
	{
		auto det = [&](const iterator it) { return std::abs(dot(face.plane.eq, Vec4f(*it, 1))); };
		return maxPrune(det);
	}

	auto isInside(const std::vector<Face> faces, const Vec4f &p4)
	{
		if (faces.empty()) return false;

		for (auto &face: faces) {
			if (dot(p4, face.plane.eq) > epsilon()) {
				return false;
			}
		}
		return true;
	}

	void prune(const std::vector<Face> inner_faces, const std::vector<Face> &outer_faces)
	{
		auto ait = begin(m_alives);
		while (ait != end(m_alives)) {
			auto p4 = Vec4f(**ait, 1);
			if (!isInside(inner_faces, p4) && isInside(outer_faces, p4)) {
				ait = m_alives.erase(ait);
			}
			else {
				++ait;
			}
		}
	}

	void prune() { prune(std::vector<Face>(), m_faces); }

	void initTetra()
	{
		auto center = Range3f(m_points).center();
		auto it0 = farestPrune(center);
		auto it1 = farestPrune(*it0);
		auto it2 = farestPrune(Edge(it0, it1));
		auto it3 = farestPrune(Face(it0, it1, it2));

		std::vector<Face> tetra_faces = {
		        Face(it0, it1, it2),
		        Face(it1, it2, it3),
		        Face(it2, it3, it0),
		        Face(it3, it0, it1),
		};
		addFirstFaces(tetra_faces);
		prune();
	}

	void addEdge(const Edge &edge)
	{
		auto eit = begin(m_edges);
		while (eit != end(m_edges)) {
			if (*eit == edge) {
				eit = m_edges.erase(eit);
				return;
			}
			++eit;
		}
		m_edges.push_back(edge);
	}

	void expand()
	{
		auto dir = normalize(Vec3f(frand() - 0.5, frand() - 0.5, frand() - 0.5));
		auto it0 = supportPrune(dir);

		m_edges.clear();
		m_innerFaces.clear();
		m_outerFaces.clear();

		auto p04 = Vec4f(*it0, 1);
		auto fit = begin(m_faces);
		while (fit != end(m_faces)) {
			if (dot(fit->plane.eq, p04) >= 0) {
				m_innerFaces.push_back(*fit);
				addEdge(Edge(fit->it0, fit->it1));
				addEdge(Edge(fit->it1, fit->it2));
				addEdge(Edge(fit->it2, fit->it0));
				fit = removeFace(fit);
			}
			else {
				++fit;
			}
		}

		for (auto &e: m_edges) {
			addFace(Face(it0, e.it0, e.it1));
			m_outerFaces.push_back(m_faces.back());
		}
		prune(m_innerFaces, m_outerFaces);
	}

	void initAlives()
	{
		for (auto it = begin(m_points); it != end(m_points); ++it) {
			m_alives.push_back(it);
		}
		auto gt = [&](const iterator it0, const iterator it1) { return greater(*it0, *it1); };
		auto eq = [&](const iterator it0, const iterator it1) { return equal(*it0, *it1); };
		vector_sort_and_uniq(m_alives, gt, eq);
	}
};
}  // namespace convex_hull

std::vector<std::vector<int32_t>> convex3f_hull(const std::vector<Vec3f> &points, int32_t max_count)
{
	assert(points.size() >= 4);
	convex_hull::Convex3Hull convexhull(points, max_count);
	convexhull.exec();
	return convexhull.m_outputs;
}
}  // namespace spu
