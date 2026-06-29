//
// ConvexHull :
//
#include <smath/geometry.h>
#include <smath/vec.h>

namespace spu {
namespace convex_hull {

class ConvexHull {
public:
	using iterator = std::vector<Vec3f>::iterator;
	using Edge = std::pair<iterator, iterator>;

	std::vector<Vec3f> m_points;
	Vec3f m_normal;
	std::vector<Edge> m_edges;

	Plane3f separatePlane(const Edge &e)
	{
		const auto &p0 = *e.first;
		const auto &p1 = *e.second;
		return {p0, normalize(cross(m_normal, p1 - p0))};
	}

	void extend(const Edge &e0, std::vector<iterator> &alives)
	{
		auto plane0 = separatePlane(e0);
		auto max_d = 0.0f;
		auto max_ap = end(alives);

		for (auto ap = begin(alives); ap != end(alives); ++ap) {
			auto d = plane0.distance(**ap);
			if (d < max_d) {
				max_ap = ap;
				max_d = d;
			}
		}
		if (max_ap == end(alives)) {
			m_edges.push_back(e0);
			return;
		}

		Edge e1 = {e0.first, *max_ap};
		Edge e2 = {*max_ap, e0.second};

		alives.erase(max_ap);

		auto plane1 = separatePlane(e1);
		auto plane2 = separatePlane(e2);

		auto ap = begin(alives);
		while (ap != end(alives)) {
			auto p = **ap;
			if (plane0.distance(p) <= +epsilon() && plane1.distance(p) >= -epsilon()
			    && plane2.distance(p) >= -epsilon()) {
				ap = alives.erase(ap);
			}
			else {
				++ap;
			}
		}
		extend(e1, alives);
		extend(e2, alives);
	}

	Edge getFirstEdge()
	{
		auto b = begin(m_points);
		auto e = Edge(b, b + 1);
		auto plane = separatePlane(e);
		auto op = [&plane](const Vec3f &p) { return plane.distance(p); };

		auto p = vector_minmax(m_points, op);
		return {b + (p.first - b), b + (p.second - b)};  // forward iterator -> random iterator
	}

	void divide(const Edge &e, std::vector<iterator> &alives_h, std::vector<iterator> &alives_l)
	{
		auto plane = separatePlane(e);
		// for (auto i = 0u; i < m_points.size(); i++) {
		for (auto it = begin(m_points); it != end(m_points); ++it) {
			auto d = plane.distance(*it);
			if (d > +epsilon()) {
				alives_l.push_back(it);
			}
			if (d < -epsilon()) {
				alives_h.push_back(it);
			}
		}
	}

	void exec()
	{
		std::vector<iterator> alives_upper;
		std::vector<iterator> alives_lower;

		auto e = getFirstEdge();
		divide(e, alives_upper, alives_lower);

		extend(e, alives_upper);
		extend(Edge(e.second, e.first), alives_lower);
	}

	std::vector<int32_t> pack()
	{
		std::vector<int32_t> packed_edges;
		auto b = begin(m_points);
		for (const auto &edge: m_edges) {
			packed_edges.push_back(edge.first - b);
		}
		return packed_edges;
	}

	explicit ConvexHull(const std::vector<Vec3f> &points, const Vec3f &normal)
	        : m_points(points), m_normal(normal)
	{
	}
};
}  // namespace convex_hull

std::vector<int32_t> convex2f_hull(const std::vector<Vec3f> &points, const Vec3f &normal)
{
	if (points.size() < 2) {
		return {};
	}
	convex_hull::ConvexHull convexhull(points, normal);
	convexhull.exec();
	return convexhull.pack();
}
}  // namespace spu
