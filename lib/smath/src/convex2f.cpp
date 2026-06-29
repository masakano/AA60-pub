//
// Convex2f :
//
#include <smath/convex2f.h>

namespace spu {

Convex2f::Convex2f(const std::vector<Vec3f> &points, const std::vector<int32_t> &indices)
{
	for (const auto &index: indices) {
		m_points.push_back(points[index]);
	}
	remap();
}

Convex2f::Convex2f(const std::vector<Vec3f> &points, bool is_qhull)
{
	Vec3f center, normal;
	center_and_normal(points, center, normal);
	if (is_qhull) {
		for (auto &index: convex2f_hull(points, normal)) {
			m_points.push_back(points[index]);
		}
	}
	else {
		m_points = points;
	}
	m_plane = Plane3f(center, normal);
}

// Convex2f::Convex2f(const Vec3f &v0, const Vec3f &v1, const Vec3f &v2) : Convex2f({v0, v1, v2}, false) {}

void Convex2f::remap()
{
	Vec3f center;
	Vec3f normal;
	center_and_normal(m_points, center, normal);
	m_plane = Plane3f(center, normal);
}

void Convex2f::flip()
{
	reverse(begin(m_points), end(m_points));
	m_plane.eq = -m_plane.eq;
}

bool Convex2f::inside(const Vec3f &point) const
{
	auto n0 = 0.0f;
	auto d0 = m_points.back() - point;
	for (const auto &p: m_points) {
		auto d1 = p - point;
		auto n1 = dot(cross<Vec3f>(d0, d1), Vec3f(m_plane.eq));
		if (n0 == 0) {
			n0 = n1;
		}
		if (n0 * n1 < 0) {
			return false;
		}
		d0 = d1;
	}
	return true;
}

std::vector<Vec3f> Convex2f::inside(const std::vector<Vec3f> &points) const
{
	std::vector<Vec3f> inside_points;
	for (const auto &point: points) {
		if (inside(point)) {
			inside_points.push_back(point);
		}
	}
	return inside_points;
}

float Convex2f::distance(const Vec3f &point, Vec3f *nearest) const
{
	Vec3f nearest_point;
	m_plane.distance(point, &nearest_point);
	if (inside(nearest_point)) {
		if (nearest) *nearest = nearest_point;
		return 0;
	}

	auto min_distance = huge();
	auto p1 = m_points.back();
	for (const auto &p0: m_points) {
		auto segment_distance = Segment3f(p0, p1).distance(point, &nearest_point);
		if (segment_distance < min_distance) {
			min_distance = segment_distance;
			if (nearest) *nearest = nearest_point;
		}
		p1 = p0;
	}
	return min_distance;
}

bool Convex2f::intersect(const Line3f &line, Vec3f *cross_point) const
{
	float cross_distance = 0;
	if (m_plane.intersect(line, &cross_distance)) {
		auto cross_point0 = line.eye + cross_distance * line.dir;
		if (cross_point) {
			*cross_point = cross_point0;
		}
		return inside(cross_point0);
	}
	return false;
}

bool Convex2f::intersect(const Segment3f &segment, Vec3f *cross_point) const
{
	Vec3f cross_point0;
	if (m_plane.intersect(segment, &cross_point0)) {
		if (cross_point) {
			*cross_point = cross_point0;
		}
		return inside(cross_point0);
	}
	return false;
}

bool Convex2f::intersect(const Plane3f &plane, Segment3f *cross_segment) const
{
	auto ep0 = m_points.back();
	auto d0 = plane.distance(ep0);
	auto is_intersect1 = false;  // up cross
	auto is_intersect2 = false;  // down cross

	for (const auto &ep1: m_points) {
		auto d1 = plane.distance(ep1);
		if (d0 * d1 < 0) {
			if (cross_segment == nullptr) {
				return true;
			}
			if (d0 - d1 > 0) {
				is_intersect1 = true;
				cross_segment->p0 = lerp(ep0, ep1, d0 / (d0 - d1));
			}
			else {
				is_intersect2 = true;
				cross_segment->p1 = lerp(ep0, ep1, d0 / (d0 - d1));
			}
		}

		if (is_intersect1 && is_intersect2) {
			return true;
		}
		d0 = d1;
		ep0 = ep1;
	}
	return false;
}

bool Convex2f::intersect(const Convex2f &convex, Segment3f *cross_segment) const
{
	Segment3f segment;
	Segment3f segment0;
	Segment3f segment1;

	if (!convex.intersect(m_plane, &segment0)) {
		return false;
	}

	if (!intersect(convex.m_plane, &segment1)) {
		return false;
	}

	auto is_intersect = true;
	auto d0 = segment0.p1 - segment0.p0;
	auto d1 = segment1.p1 - segment1.p0;

	if (dot(d0, d0) < epsilon() || dot(d1, d1) < epsilon()) {
		return false;
	}

	if (dot(d0, d1) < 0) {
		std::swap(segment1.p0, segment1.p1);
	}

	auto ds = segment1.p0 - segment0.p0;
	auto de = segment1.p1 - segment0.p1;

	segment.p0 = dot(d0, ds) > 0 ? segment1.p0 : segment0.p0;
	segment.p1 = dot(d0, de) > 0 ? segment0.p1 : segment1.p1;

	is_intersect = dot(d0, segment.p1 - segment.p0) >= 0;

	if (is_intersect && (cross_segment)) {
		*cross_segment = segment;
	}
	return is_intersect;
}

std::vector<Vec3f> Convex2f::walls() const
{
	std::vector<Vec3f> wall_normals;

	Vec3f p0 = m_points.back();
	for (const auto &p1: m_points) {
		wall_normals.emplace_back(normalize(cross<Vec3f>(p1 - p0, Vec3f(m_plane.eq))));
		p0 = p1;
	}
	return wall_normals;
}

Convex2f Convex2f::cup(const Convex2f &convexB) const
{
	std::vector<Vec3f> points = m_points;
	vector_cat(points, convexB.m_points);
	return Convex2f(points, true);  // with quick hull
}

Convex2f Convex2f::cap(const Convex2f &convexB) const
{
	std::vector<Vec3f> points;
	const Convex2f &convexA = *this;

	points = convexB.inside(convexA.m_points);
	vector_cat(points, convexA.inside(convexB.m_points));

	// nA may be -nB
	auto nA = Vec3f(convexA.m_plane.eq);
	auto nB = Vec3f(convexB.m_plane.eq);
	auto normal = dot(nA, nB) > 0 ? normalize(nA + nB) : normalize(nA - nB);

	auto A0 = convexA.m_points.back();
	for (const auto &A1: convexA.m_points) {
		Mat4f m;
		m.c[0] = normalize(A1 - A0);            // tangent (line direction)
		m.c[2] = normal;                        // normal (plane normal)
		m.c[1] = cross<Vec3f>(m.c[2], m.c[0]);  // binormal (wall normal)
		m.c[3] = Vec4f(A0, 1);                  // origin (A0)

		m = m.unitary_inverse();
		auto a1 = m.ortho3(A1);

		auto B0 = convexB.m_points.back();
		for (const auto &B1: convexB.m_points) {
			auto b0 = m.ortho3(B0);
			auto b1 = m.ortho3(B1);

			// cross point ratio
			auto rB = b0.y - b1.y != 0 ? b0.y / (b0.y - b1.y) : 0.0;

			// cross point in tangent
			auto rA = ((1 - rB) * b0.x + rB * b1.x) / a1.x;

			if (0 < rA && rA < 1.0 && 0 < rB && rB < 1.0) {
				points.emplace_back(A0 + rA * (A1 - A0));
			}
			B0 = B1;
		}
		A0 = A1;
	}
	return Convex2f(points, true);  // with quick hull
}

void Convex2f::modulate(const std::vector<Vec3f> &points)
{
	assert(points.size() == m_points.size());

	Vec3f center, normal;
	center_and_normal(points, center, normal);

	m_points = points;
	m_plane = Plane3f(center, normal);
}

Convex2f operator*(const Mat4f &m, const Convex2f &c0)
{
	Convex2f c;
	c.m_points = m.pers3(c0.points());
	c.m_plane = m * c0.m_plane;
	return c;
}

}  // namespace spu
