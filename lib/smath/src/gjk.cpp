//
// Gjk :
//
#include <smath/gjk.h>
// #include <spu/spu.h>

// #define dbg_printf(...) spu_printf(0, __VA_ARGS__)
#define dbg_printf(...)

namespace spu {

Vec3f Gjk::support(const Vec3f &dir, const std::vector<Vec3f> &points) const
{
	return *vector_max(points, [&dir](const Vec3f &p) { return dot(dir, p); });
}

Vec3f Gjk::support(const Vec3f &dir) const
{
	auto pA = support(+dir, m_pointsA);
	auto pB = support(-dir, m_pointsB);
	return pA - pB;
}

Gjk::Gjk(const std::vector<Vec3f> &pointsA, const std::vector<Vec3f> &pointsB)
        : m_pointsA(pointsA), m_pointsB(pointsB)
{
}

Gjk::Status Gjk::gjk()
{
	// point
	{
		auto centerA = vector_average(m_pointsA);
		auto centerB = vector_average(m_pointsB);
		auto v0 = normalize(centerA - centerB);
		m_gjkCount = 0;

		m_p0 = m_p1 = m_p2 = m_p3 = support(v0);
		if (++m_gjkCount > m_maxGjkCount) return e_inside;
	}

	// segment
	{
		auto v1 = -normalize(m_p0);

		m_p1 = m_p2 = m_p3 = support(v1);
		if (++m_gjkCount > m_maxGjkCount) return e_inside;
		if (dot(m_p1, v1) <= 0) return e_outside;
	}

	// triangle
	{
		auto dx = m_p1 - m_p0;
		auto dy = cross_candidate(dx);
		auto v2 = normalize(cross(dx, dy));
		if (dot(v2, m_p0) > 0) v2 = -v2;

		m_p2 = m_p3 = support(v2);
		if (++m_gjkCount > m_maxGjkCount) return e_timeout;
		if (dot(m_p2, v2) <= 0) return e_outside;
	}

	// corn
	{
		auto get_triangle_normal = [](const Vec3f &p0, const Vec3f &p1, const Vec3f &p2) {
			auto n = normalize(cross(p1 - p0, p2 - p0));
			if (dot(n, p0) > 0) n = -n;
			return n;
		};

		auto v3 = get_triangle_normal(m_p0, m_p1, m_p2);
		m_p3 = support(v3);
		if (++m_gjkCount > m_maxGjkCount) return e_timeout;

		// corn loop
		for (int i = 0; i < m_maxGjkCount; i++) {
			auto n0 = get_triangle_normal(m_p0, m_p1, m_p2);
			auto n1 = get_triangle_normal(m_p1, m_p2, m_p3);
			auto n2 = get_triangle_normal(m_p2, m_p3, m_p0);
			auto n3 = get_triangle_normal(m_p3, m_p0, m_p1);

			auto d0 = dot(m_p0 - m_p1, n1);
			auto d1 = dot(m_p1 - m_p2, n2);
			auto d2 = dot(m_p2 - m_p3, n3);
			auto d3 = dot(m_p3 - m_p0, n0);

			auto dt0 = dot(m_p0, v3);
			auto dt1 = dot(m_p1, v3);
			auto dt2 = dot(m_p2, v3);
			auto dt3 = dot(m_p3, v3);

			dbg_printf("count = %d / %d\n", m_gjkCount, m_maxGjkCount);
			dbg_printf("dir = %8.5f, %8.5f, %8.5f, %8.5f\n", d0, d1, d2, d3);
			dbg_printf("dot = %8.5f, %8.5f, %8.5f, %8.5f\n", dt0, dt1, dt2, dt3);

			if (dt0 <= 0 && dt1 <= 0 && dt2 <= 0 && dt3 <= 0) {
				return e_outside;
			}
			if (d0 >= 0 && d1 >= 0 && d2 >= 0 && d3 >= 0) {
				return e_inside;
			}
			if (d0 < 0) {
				v3 = get_triangle_normal(m_p1, m_p2, m_p3);
				m_p0 = support(v3);
			}
			if (d1 < 0) {
				v3 = get_triangle_normal(m_p2, m_p3, m_p0);
				m_p1 = support(v3);
			}
			if (d2 < 0) {
				v3 = get_triangle_normal(m_p3, m_p0, m_p1);
				m_p2 = support(v3);
			}
			if (d3 < 0) {
				v3 = get_triangle_normal(m_p0, m_p1, m_p2);
				m_p3 = support(v3);
			}
		}
	}
	return e_timeout;
}

std::vector<Gjk::Face> Gjk::epa()
{
	std::vector<Edge> m_edges;

	auto add_edge = [&](const Edge &edge) {
		auto it = vector_find(m_edges, edge);
		if (it != std::end(m_edges)) {
			m_edges.erase(it);
		}
		else {
			m_edges.push_back(edge);
		}
	};

	std::vector<Face> m_faces = {
	        {m_p2, m_p1, m_p0},
	        {m_p3, m_p0, m_p1},
	        {m_p3, m_p1, m_p2},
	        {m_p3, m_p2, m_p0},
	};

	for (auto i = 0; i < m_maxEpaCount; i++) {
		auto nearest_face = *vector_max(m_faces, [](const Face &f) { return f.eq.w; });

		if (std::abs(nearest_face.eq.w) < 0.0001) {
			dbg_printf(
			        0, "zero face: %f %f %f %f\n", nearest_face.eq.x, nearest_face.eq.y,
			        nearest_face.eq.z, nearest_face.eq.w);
			return m_faces;
		}

		m_nearest = -nearest_face.eq.w * Vec3f(nearest_face.eq);

		if (distance(m_nearest, m_prevNearest) < epsilon()) {
			return m_faces;
		}
		m_prevNearest = m_nearest;

		auto v = normalize(m_nearest);
		auto p = support(v);

		dbg_printf("%2d: ", i);
		dbg_printf("near: %7.4f %7.4f %7.4f ", m_nearest.x, m_nearest.y, m_nearest.z);
		dbg_printf("v: %7.4f %7.4f %7.4f ", v.x, v.y, v.z);
		dbg_printf("p: %7.4f %7.4f %7.4f ", p.x, p.y, p.z);

		std::vector<Face> new_faces;
		m_edges.clear();
		for (auto &face: m_faces) {
			if (dot(Vec4f(p, 1), face.eq) < 0) {
				new_faces.push_back(face);
			}
			else {
				add_edge({face.p0, face.p1});
				add_edge({face.p1, face.p2});
				add_edge({face.p2, face.p0});
			}
		}

		for (auto &edge: m_edges) {
			new_faces.emplace_back(p, edge.p0, edge.p1);
		}
		dbg_printf("new edge=%ld new faces=%ld\n", m_edges.size(), new_faces.size());
		m_faces = new_faces;
	}
	// printf("m_faces=%ld\n", m_faces.size());
	return m_faces;
}

}  // namespace spu
