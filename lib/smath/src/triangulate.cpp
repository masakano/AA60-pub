//
// Triangulate :
//
#include <smath/geometry.h>

namespace spu {
namespace {

class Triangulate {
public:
	explicit Triangulate(const std::vector<Vec3f> &points);
	std::vector<int32_t> exec();

private:
	std::vector<int32_t> m_indices;
	std::vector<Vec3f> m_points;
	Vec3f m_normal;

	std::tuple<int32_t, int32_t, int32_t> getIndices(int32_t i);
	std::tuple<Vec3f, Vec3f, Vec3f> getPoints(int32_t i);
	// Vec3f getNormal(int32_t i);
	bool isLegal(int32_t i);
	static bool isInside(const Vec3f &p, const Vec3f &p0, const Vec3f &p1, const Vec3f &p2);
};

Triangulate::Triangulate(const std::vector<Vec3f> &points) : m_points(points)
{
	Vec3f center;
	center_and_normal(points, center, m_normal);
	m_indices.resize(m_points.size());
	iota(begin(m_indices), end(m_indices), 0);
}

std::vector<int32_t> Triangulate::exec()
{
	std::vector<int32_t> tris;

	while (m_indices.size() > 3) {
		auto it = vector_find_if(m_indices, [&](int32_t i) { return isLegal(i); });
		if (it != std::end(m_indices)) {
			auto i = it - begin(m_indices);
			auto t = getIndices(i);
			tris.push_back(std::get<0>(t));
			tris.push_back(std::get<1>(t));
			tris.push_back(std::get<2>(t));
			m_indices.erase(it);
		}
		else {
			break;
		}
	}
	vector_cat(tris, m_indices);
	return tris;
}

std::tuple<int32_t, int32_t, int32_t> Triangulate::getIndices(int32_t i)
{
	auto n = m_indices.size();
	auto i0 = m_indices[i];
	auto i1 = m_indices[(i + 1) % n];
	auto i2 = m_indices[(i + n - 1) % n];
	return {i0, i1, i2};
}

std::tuple<Vec3f, Vec3f, Vec3f> Triangulate::getPoints(int32_t i)
{
	auto t = getIndices(i);
	auto p0 = m_points[std::get<0>(t)];
	auto p1 = m_points[std::get<1>(t)];
	auto p2 = m_points[std::get<2>(t)];
	return {p0, p1, p2};
}
#if 0
Vec3f Triangulate::getNormal(int32_t i)
{
	auto t = getPoints(i);
	auto &p0 = std::get<0>(t);
	auto &p1 = std::get<1>(t);
	auto &p2 = std::get<1>(t);
	return cross(p1 - p0, p2 - p0);
}
#endif
bool Triangulate::isInside(const Vec3f &p, const Vec3f &p0, const Vec3f &p1, const Vec3f &p2)
{
	auto n0 = cross(p0 - p, p1 - p);
	auto n1 = cross(p1 - p, p2 - p);
	auto n2 = cross(p2 - p, p0 - p);

	auto d0 = dot(n0, n1);
	auto d1 = dot(n1, n2);
	auto d2 = dot(n2, n0);

	return (d0 > 0 && d1 > 0 && d2 > 0) || (d0 < 0 && d1 < 0 && d2 < 0);
}

bool Triangulate::isLegal(int32_t i)
{
	auto t = getPoints(i);
	auto &p0 = std::get<0>(t);
	auto &p1 = std::get<1>(t);
	auto &p2 = std::get<1>(t);

	if (dot(cross(p1 - p0, p2 - p0), m_normal) < 0) {
		return false;
	}

	for (auto &index: m_indices) {
		if (isInside(m_points[index], p0, p1, p2)) {
			return false;
		}
	}
	return true;
}
}  // namespace
std::vector<int32_t> triangulate(const std::vector<Vec3f> &points)
{
	Triangulate triangulate(points);
	return triangulate.exec();
}
}  // namespace spu
