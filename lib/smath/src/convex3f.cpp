//
// Convex3f :
//
#include <smath/convex3f.h>

namespace spu {

Convex3f::Convex3f(const std::vector<Convex2f> &convexes) : m_convexes(convexes) { autoflip(); }

Convex3f::Convex3f(const std::vector<std::vector<Vec3f>> &points_array)
{
	m_convexes.reserve(points_array.size());
	for (const auto &points: points_array) {
		m_convexes.emplace_back(points, 0);
	}
	autoflip();
}

Convex3f::Convex3f(const std::vector<Vec3f> &points, const std::vector<std::vector<int32_t>> &indices_array)
{
	m_convexes.reserve(indices_array.size());
	for (const auto &indices: indices_array) {
		m_convexes.emplace_back(points, indices);
	}
	autoflip();
}

Convex3f::Convex3f(const std::vector<Vec3f> &points) : Convex3f(points, convex3f_hull(points)) {}

void Convex3f::autoflip()
{
	Vec3f center = ezero();
	auto n = 0;

	for (auto &convex: m_convexes) {
		center += vector_accumulate(convex.points());
		n += convex.points().size();
	}
	center /= n;

	for (auto &convex: m_convexes) {
		if (convex.plane().distance(center) > 0) {
			convex.flip();
		}
	}
}

bool Convex3f::inside(const Vec3f &point) const
{
	for (const auto &convex: m_convexes) {
		if (convex.plane().distance(point) > 0) {
			return false;
		}
	}
	return true;
}

float Convex3f::distance(const Vec3f &point, Vec3f *nearest) const
{
	auto is_outside = false;
	auto min_distance = huge();

	Vec3f nearest_point;
	for (auto &convex2: m_convexes) {
		// auto distance = convex2.plane().distance(point, &nearest_point);
		auto distance = convex2.distance(point, &nearest_point);
		if (distance > 0) {  // outside
			is_outside = true;
			if (distance < min_distance) {
				min_distance = distance;
				if (nearest) *nearest = nearest_point;
			}
		}
	}
	if (is_outside) {
		return min_distance;
	}
	else {
		if (nearest) *nearest = point;
		return 0;
	}
}

bool Convex3f::intersect(const Plane3f &plane, std::vector<Segment3f> *intersect_segments) const
{
	bool is_intersect = false;
	for (const auto &convex: m_convexes) {
		Segment3f segment;
		if (convex.intersect(plane, &segment)) {
			if (intersect_segments) {
				is_intersect = true;
				intersect_segments->push_back(segment);
			}
			else {
				return true;  // return immediately
			}
		}
	}
	return is_intersect;
}

bool Convex3f::intersect(const Convex3f &convex3, std::vector<Segment3f> *intersect_segments) const
{
	bool is_intersect = false;
	for (const auto &convex2A: m_convexes) {
		for (const auto &convex2B: convex3.m_convexes) {
			Segment3f segment;
			if (convex2A.intersect(convex2B, &segment)) {
				if (intersect_segments) {
					is_intersect = true;
					intersect_segments->push_back(segment);
				}
				else {
					return true;  // return immediately
				}
			}
		}
	}
	return is_intersect;
}

bool Convex3f::penetrate(const Line3f &ray, float *t) const
{
	auto t_min = huge();
	for (auto i = 0u; i < m_convexes.size(); i++) {
		auto &plane0 = m_convexes[i].plane();
		auto t0 = 0.0f;
		if (plane0.intersect(ray, &t0)) {
			if (t0 < t_min) {
				auto p0 = ray.eye + t0 * ray.dir;
				auto is_outside = false;
				for (auto j = 0u; j < m_convexes.size(); j++) {
					if (j != i) {
						auto &plane1 = m_convexes[j].plane();
						if (plane1.distance(p0) > 0) {
							is_outside = true;
							break;
						}
					}
				}
				if (!is_outside) {
					t_min = t0;
				}
			}
		}
	}
	if (t) *t = t_min;
	return t_min < huge();
}

Range3f Convex3f::range() const
{
	Range3f range;
	range.invalidate();
	for (const auto &convex: m_convexes) {
		range.expand(convex.points());
	}
	return range;
}

void Convex3f::marge()
{
	auto c0 = begin(m_convexes);
	while (c0 != end(m_convexes)) {
		auto points = c0->points();
		auto is_marge = false;
		auto c1 = c0 + 1;

		while (c1 != end(m_convexes)) {
			const auto delta_eq = c0->plane().eq - c1->plane().eq;
			if (dot<Vec4f>(delta_eq, delta_eq) < epsilon()) {
				vector_cat(points, c1->points());
				c1 = m_convexes.erase(c1);  // c1 > c0
				is_marge = true;
			}
			else {
				++c1;
			}
		}
		if (is_marge) {
			*c0 = Convex2f(points, true);
		}
		++c0;
	}
	autoflip();
}

std::vector<Vec3f> Convex3f::points() const
{
	std::vector<Vec3f> all_points;
	for (auto &convex: m_convexes) {
		vector_cat(all_points, convex.points());
	}
	return all_points;
}

void Convex3f::modulate(const std::vector<Vec3f> &points)
{
	auto it = points.begin();
	for (auto &convex: m_convexes) {
		auto count = convex.points().size();
		convex.modulate(std::vector<Vec3f>(it, it + count));
		it += count;
	}
}

Convex3f Convex3f::makeCylinder(int32_t nx, int32_t ny)
{
	auto cylinder_point = [](const float u, const float v) {
		return Vec3f(cosf(radians(u * 360)), sin(radians(u * 360)), v * 2.0 - 1.0);
	};

	const auto du = 1.0f / float(nx);
	const auto dv = 1.0f / float(ny);

	Convex3f convex3;
	for (auto v = 0.0f; v < 1.0f; v += dv) {
		for (auto u = 0.0f; u < 1.0f; u += du) {
			std::vector<Vec3f> points = {
			        cylinder_point(u + 0 * du, v + 0 * dv),
			        cylinder_point(u + 1 * du, v + 0 * dv),
			        cylinder_point(u + 1 * du, v + 1 * dv),
			        cylinder_point(u + 0 * du, v + 1 * dv),

			};
			convex3.m_convexes.emplace_back(points, 0);
		}
	}

	std::vector<Vec3f> tops;
	std::vector<Vec3f> bottoms;

	for (auto u = 0.0f; u < 1.0f; u += du) {
		tops.push_back(cylinder_point(u, 0.0));
		bottoms.push_back(cylinder_point(u, 1.0));
	}
	convex3.m_convexes.emplace_back(tops, 0);
	convex3.m_convexes.emplace_back(bottoms, 0);
	convex3.autoflip();
	return convex3;
}

namespace {
class Simplex3f {
public:
	explicit Simplex3f(const Vec3f p[3], const Vec3f &c = ezero())
	{
		m_points[0] = p[0] - c;
		m_points[1] = p[1] - c;
		m_points[2] = p[2] - c;
	}

	float volume() { return std::abs(dot(cross(m_points[1], m_points[0]), m_points[2]) / 6.0f); }

	Mat4f moment()
	{
		Mat4f m;

		Vec3f t[3] = {
		        {m_points[0].x, m_points[1].x, m_points[2].x},
		        {m_points[0].y, m_points[1].y, m_points[2].y},
		        {m_points[0].z, m_points[1].z, m_points[2].z},
		};

		m.c[0].f[0] = inp(t[1], t[2]);
		m.c[1].f[1] = inp(t[2], t[0]);
		m.c[2].f[2] = inp(t[0], t[1]);

		m.c[0].f[1] = -crosp(t[1], t[0]);
		m.c[0].f[2] = -crosp(t[2], t[0]);

		m.c[1].f[0] = -crosp(t[0], t[1]);
		m.c[1].f[2] = -crosp(t[2], t[1]);

		m.c[2].f[0] = -crosp(t[0], t[2]);
		m.c[2].f[1] = -crosp(t[0], t[1]);

		return m;
	}

	static std::vector<Vec3f> triangulate(const Convex3f &convex3)
	{
		std::vector<Vec3f> tris;
		for (const auto &convex: convex3.convexes()) {
			const auto &points = convex.points();
			for (auto i = 2u; i < points.size(); i++) {
				tris.push_back(points[i]);
				tris.push_back(points[i - 1]);
				tris.push_back(points[0]);
			}
		}

		return tris;
	}

private:
	Vec3f m_points[3];

	static float inp(const Vec3f &x, const Vec3f &y) noexcept
	{
		return (x.x * x.x + x.y * x.y + x.z * x.z + x.x * x.y + x.y * x.z + x.z * x.x + y.x * y.x
		        + y.y * y.y + y.z * y.z + y.x * y.y + y.y * y.z + y.z * y.x);
	}

	static float crosp(const Vec3f &x, const Vec3f &y) noexcept
	{
		return (1.0 * x.x * y.x + 0.5 * x.x * y.y + 0.5 * x.x * y.z + 0.5 * x.y * y.x + 1.0 * x.y * y.y
		        + 0.5 * x.y * y.z + 0.5 * x.z * y.x + 0.5 * x.z * y.y + 1.0 * x.z * y.z);
	}
};
}  // namespace

Vec3f Convex3f::center_of_gravity() const { return vector_average(Simplex3f::triangulate(*this)); }

Mat4f Convex3f::moment() const
{
	auto triangles = Simplex3f::triangulate(*this);
	auto weight_center = vector_average(triangles);
	auto total_volume = 0.0f;

	Mat4f total_moment(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	for (auto i = 0u; i < triangles.size(); i += 3) {
		Simplex3f simplex(&triangles[i], weight_center);

		const float volume = simplex.volume();
		const Mat4f moment = simplex.moment().scale(volume);

		total_volume = total_volume + volume;
		total_moment = total_moment + moment;
	}

	total_moment = total_moment.scale(1.0 / total_volume);
	total_moment.c[3] = {0, 0, 0, 1};

	return total_moment;
}
}  // namespace spu
