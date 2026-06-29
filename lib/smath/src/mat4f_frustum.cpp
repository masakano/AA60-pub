//
// Mat4f :
//
#include <smath/mat4f.h>
#include <smath/convex3f.h>

namespace spu {
namespace {

/*

              -Z
       7  +---------+ 6
         /|        /|
        / |       / |
       /  |      /  |
    4 +---------+ 5 |
      | 3 +-----|---+ 2
      |  /      |  /
      | /       | /
      |/        |/
    0 +---------+ 1
          +Z
*/

std::vector<Vec3f> s_points3 = {
        {-1, -1, +1},
        {+1, -1, +1},
        {+1, -1, -1},
        {-1, -1, -1},
        {-1, +1, +1},
        {+1, +1, +1},
        {+1, +1, -1},
        {-1, +1, -1},
};

std::vector<std::vector<int32_t>> s_edges = {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 0},
        {4, 5},
        {5, 6},
        {6, 7},
        {7, 4},
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}
};

std::vector<std::vector<int32_t>> s_faces = {
        {3, 7, 4, 0}, // negative x
        {1, 5, 6, 2}, // positive x
        {0, 1, 2, 3}, // negative y
        {4, 7, 6, 5}, // positive y
        {2, 6, 7, 3}, // negative z
        {0, 4, 5, 1}, // positive z
};

std::vector<Vec4f> s_plane_equations = {
        {-1, +0, +0, -1}, // negative x
        {+1, +0, +0, -1}, // positive x
        {+0, -1, +0, -1}, // negative y
        {+0, +1, +0, -1}, // positive y
        {+0, +0, -1, -1}, // negative z
        {+0, +0, +1, -1}, // positive z
};

std::vector<Vec3f> quantize_and_uniq(const std::vector<Segment3f> &segments, float quantize_rate = 0.0f)
{
	std::vector<Vec3f> points;
	points.reserve(segments.size() * 2);
	for (auto &segment: segments) {
		points.push_back(segment.p0);
		points.push_back(segment.p1);
	}
	quantize_and_uniq(points, quantize_rate);
	return points;
}

}  // namespace

Mat4f::Mat4f(const Range3f &range, uint32_t mask)
{
	auto t = range.center();
	auto s = max(Vec3f(range.span() * 0.5f), Vec3f(epsilon()));

	t = select(mask, t, ezero());
	s = select(mask, s, eone());

	*this = Mat4f().scale(s).trans(t).inverse();
}

std::vector<Vec3f> Mat4f::points() const { return inverse().pers3(s_points3); }

std::vector<Plane3f> Mat4f::planes() const
{
	auto t = transpose4();
	return std::vector<Plane3f>{
	        Plane3f(t * s_plane_equations[0]),  // nx
	        Plane3f(t * s_plane_equations[1]),  // px
	        Plane3f(t * s_plane_equations[2]),  // ny
	        Plane3f(t * s_plane_equations[3]),  // py
	        Plane3f(t * s_plane_equations[4]),  // nz
	        Plane3f(t * s_plane_equations[5]),  // pz
	};
}

std::vector<Segment3f> Mat4f::segments() const
{
	auto points = this->points();

	std::vector<Segment3f> segments;
	segments.reserve(s_edges.size());
	for (auto &edge: s_edges) {
		segments.emplace_back(points[edge[0]], points[edge[1]]);
	}
	return segments;
}

Convex3f Mat4f::convex3f() const
{
	auto points = this->points();
	auto planes = this->planes();

	std::vector<Convex2f> convexes(6);
	for (auto i = 0; i < 6; i++) {
		std::vector<Vec3f> face = {
		        points[s_faces[i][0]],
		        points[s_faces[i][1]],
		        points[s_faces[i][2]],
		        points[s_faces[i][3]],
		};
		convexes[i] = Convex2f(face, planes[i]);
	}
	return Convex3f(convexes);
}

Range3f Mat4f::range() const { return Range3f(points()); }

bool Mat4f::inside(const Vec3f &point) const
{
	auto p4 = (*this) * Vec4f(point, 1);
	return p4.w > 0 && ((abs(p4) <= Vec4f(p4.w)).pack() & 0x0fff) == 0x0fff;
}

bool Mat4f::inside(const std::vector<Vec3f> &points, std::vector<Vec3f> *inside_points) const
{
	bool is_inside = false;
	for (auto &point: points) {
		if (inside(point)) {
			is_inside = true;
			if (!inside_points) {
				return true;
			}
			inside_points->push_back(point);
		}
	}
	return is_inside;
}

bool Mat4f::inside(const Plane3f &plane, std::vector<Vec3f> *inside_points) const
{
	float min_d, max_d;
	plane.support(points(), &min_d, &max_d);
	if (min_d > 0.0f && max_d < 0.0f) {
		return false;
	}
	std::vector<Segment3f> intersect_segments;
	if (convex3f().intersect(plane, &intersect_segments)) {
		if (inside_points) {
			vector_cat(*inside_points, quantize_and_uniq(intersect_segments));
		}
		return true;
	}
	return false;
}

bool Mat4f::intersect(const Mat4f &frustum) const
{
	const auto points1 = frustum.points();
	for (auto &plane: planes()) {
		float min_d, max_d;
		plane.support(points1, &min_d, &max_d);
		if (min_d > 0.0) return false;
	}
	const auto points0 = points();
	for (auto &plane: frustum.planes()) {
		float min_d, max_d;
		plane.support(points0, &min_d, &max_d);
		if (min_d > 0.0) return false;
	}
	return true;
}

#if 0
bool Mat4f::inside(const Mat4f &frustum, std::vector<Vec3f> *inside_points) const
{
	std::vector<Vec3f> points0;
	if (inside(frustum.points(), &points0)) {
		if (!inside_points) {
			return true;
		}
		if (points0.size() == 8) {
			vector_cat(*inside_points, points0);
			return true;
		}
	}

	std::vector<Vec3f> points1;
	frustum.inside(points(), &points1);
	if (points1.size() == 8) {
		if (!inside_points) {
			return true;
		}
		if (points0.size() == 8) {
			vector_cat(*inside_points, points1);
		}
		return true;
	}

	if (inside_points) {
		vector_cat(*inside_points, points0);
		vector_cat(*inside_points, points1);
	}

	std::vector<Segment3f> intersect_segments;
	if (convex3f().intersect(frustum.convex3f(), &intersect_segments)) {
		if (inside_points) {
			vector_cat(*inside_points, quantize_and_uniq(intersect_segments));
		}
		return true;
	}
	return false;
}
#endif
}  // namespace spu
