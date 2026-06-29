//
// Mat4f :
//
#include <smath/mat4f.h>
#include <smath/geometry.h>
#include <cstdarg>
#include "mat4f_projection.h"
#include "mat4f_eric_inverse.h"

namespace spu {
static_assert(std::is_trivially_copyable<Mat4f>::value, "not standard layout");
static_assert(std::is_standard_layout<Mat4f>::value, "not standard layout");

void Mat4f::report(const char *str) const
{
	if (str && *str) aux_printf("%s:\n", str);

	for (auto y = 0; y < 4; y++) {
		for (const auto &x: c) {
			aux_printf("%12.6f ", x.f[y]);
		}
		aux_printf("\n");
	}
}

Mat4f Mat4f::unitary_inverse() const
{
	auto m1 = transpose3();
	m1.c[3] = Vec4f(-(m1 * c[3]), 1.0);
	return m1;
}

float Mat4f::det() const
{
	const auto *f = data();

	return f[0] * f[5] * f[10] * f[15] + f[0] * f[6] * f[11] * f[13] + f[0] * f[7] * f[9] * f[14]
	     + f[1] * f[4] * f[11] * f[14] + f[1] * f[6] * f[8] * f[15] + f[1] * f[7] * f[10] * f[12]
	     + f[2] * f[4] * f[9] * f[15] + f[2] * f[5] * f[11] * f[12] + f[2] * f[7] * f[8] * f[13]
	     + f[3] * f[4] * f[10] * f[13] + f[3] * f[5] * f[8] * f[14] + f[3] * f[6] * f[9] * f[12]
	     - f[0] * f[5] * f[11] * f[14] - f[0] * f[6] * f[9] * f[15] - f[0] * f[7] * f[10] * f[13]
	     - f[1] * f[4] * f[10] * f[15] - f[1] * f[6] * f[11] * f[12] - f[1] * f[7] * f[8] * f[14]
	     - f[2] * f[4] * f[11] * f[13] - f[2] * f[5] * f[8] * f[15] - f[2] * f[7] * f[9] * f[12]
	     - f[3] * f[4] * f[9] * f[14] - f[3] * f[5] * f[10] * f[12] - f[3] * f[6] * f[8] * f[13];
}

double Mat4f::precise_det() const
{
	double f[16];

	for (auto i = 0; i < 16; i++) {
		f[i] = data()[i];
	}

	return f[0] * f[5] * f[10] * f[15] + f[0] * f[6] * f[11] * f[13] + f[0] * f[7] * f[9] * f[14]
	     + f[1] * f[4] * f[11] * f[14] + f[1] * f[6] * f[8] * f[15] + f[1] * f[7] * f[10] * f[12]
	     + f[2] * f[4] * f[9] * f[15] + f[2] * f[5] * f[11] * f[12] + f[2] * f[7] * f[8] * f[13]
	     + f[3] * f[4] * f[10] * f[13] + f[3] * f[5] * f[8] * f[14] + f[3] * f[6] * f[9] * f[12]
	     - f[0] * f[5] * f[11] * f[14] - f[0] * f[6] * f[9] * f[15] - f[0] * f[7] * f[10] * f[13]
	     - f[1] * f[4] * f[10] * f[15] - f[1] * f[6] * f[11] * f[12] - f[1] * f[7] * f[8] * f[14]
	     - f[2] * f[4] * f[11] * f[13] - f[2] * f[5] * f[8] * f[15] - f[2] * f[7] * f[9] * f[12]
	     - f[3] * f[4] * f[9] * f[14] - f[3] * f[5] * f[10] * f[12] - f[3] * f[6] * f[8] * f[13];
}

Mat4f Mat4f::precise_inverse(bool *flag) const
{
	const int32_t c[16][6][3] = {
	        {{5, 10, 15}, {6, 11, 13}, {7, 9, 14},  {5, 11, 14}, {6, 9, 15},  {7, 10, 13}},
	        {{1, 11, 14}, {2, 9, 15},  {3, 10, 13}, {1, 10, 15}, {2, 11, 13}, {3, 9, 14} },
	        {{1, 6, 15},  {2, 7, 13},  {3, 5, 14},  {1, 7, 14},  {2, 5, 15},  {3, 6, 13} },
	        {{1, 7, 10},  {2, 5, 11},  {3, 6, 9},   {1, 6, 11},  {2, 7, 9},   {3, 5, 10} },
	        {{4, 11, 14}, {6, 8, 15},  {7, 10, 12}, {4, 10, 15}, {6, 11, 12}, {7, 8, 14} },
	        {{0, 10, 15}, {2, 11, 12}, {3, 8, 14},  {0, 11, 14}, {2, 8, 15},  {3, 10, 12}},
	        {{0, 7, 14},  {2, 4, 15},  {3, 6, 12},  {0, 6, 15},  {2, 7, 12},  {3, 4, 14} },
	        {{0, 6, 11},  {2, 7, 8},   {3, 4, 10},  {0, 7, 10},  {2, 4, 11},  {3, 6, 8}  },
	        {{4, 9, 15},  {5, 11, 12}, {7, 8, 13},  {4, 11, 13}, {5, 8, 15},  {7, 9, 12} },
	        {{0, 11, 13}, {1, 8, 15},  {3, 9, 12},  {0, 9, 15},  {1, 11, 12}, {3, 8, 13} },
	        {{0, 5, 15},  {1, 7, 12},  {3, 4, 13},  {0, 7, 13},  {1, 4, 15},  {3, 5, 12} },
	        {{0, 7, 9},   {1, 4, 11},  {3, 5, 8},   {0, 5, 11},  {1, 7, 8},   {3, 4, 9}  },
	        {{4, 10, 13}, {5, 8, 14},  {6, 9, 12},  {4, 9, 14},  {5, 10, 12}, {6, 8, 13} },
	        {{0, 9, 14},  {1, 10, 12}, {2, 8, 13},  {0, 10, 13}, {1, 8, 14},  {2, 9, 12} },
	        {{0, 6, 13},  {1, 4, 14},  {2, 5, 12},  {0, 5, 14},  {1, 6, 12},  {2, 4, 13} },
	        {{0, 5, 10},  {1, 6, 8},   {2, 4, 9},   {0, 6, 9},   {1, 4, 10},  {2, 5, 8}  },
	};

	Mat4f dst;
	double src[16];

	auto det = precise_det();

	if (std::abs(det) < epsilon<double>()) {
		if (flag) {
			*flag = false;
		}
		return {};
	}

	for (auto i = 0; i < 16; i++) {
		src[i] = data()[i];
	}

	for (auto j = 0; j < 16; j++) {
		double f[6];
		for (auto i = 0; i < 6; i++) {
			f[i] = src[c[j][i][0]] * src[c[j][i][1]] * src[c[j][i][2]];
		}
		dst.data()[j] = (f[0] + f[1] + f[2] - f[3] - f[4] - f[5]) / det;
	}
	if (flag) {
		*flag = true;
	}
	return dst;
}

bool Mat4f::set_orientation(const Vec3f *eye, const Vec3f *dir, const Vec3f *up)
{
	if (eye) c[3] = Vec4f(*eye, 1);
	if (dir) c[2] = -Vec4f(*dir, 0);
	if (up) c[1] = Vec4f(*up, 0);

	auto c2 = spu::normalize<Vec3f>(c[2]);
	if (equal(c2, ezero<Vec3f>())) return false;

	auto c0 = spu::normalize<Vec3f>(cross<Vec3f>(c[1], c2));
	if (equal(c0, ezero<Vec3f>())) return false;

	auto c1 = spu::normalize(cross<Vec3f>(c2, c0));
	if (equal(c1, ezero<Vec3f>())) return false;

	c[0] = c0;
	c[1] = c1;
	c[2] = c2;

	return true;
}

bool Mat4f::get_orientation(Vec3f *eye, Vec3f *dir, Vec3f *up) const
{
	if (eye) {
		*eye = c[3];
	}
	if (dir) {
		auto d = spu::normalize<Vec3f>(c[2]);
		if (equal(d, ezero<Vec3f>())) return false;
		*dir = -d;
	}
	if (up) {
		auto u = spu::normalize<Vec3f>(c[1]);
		if (equal(u, ezero<Vec3f>())) return false;
		*up = u;
	}
	return true;
}

bool Mat4f::set_projection(
        const float *fov, const float *aspect, const float *near, const float *far, bool is_fovy)
{
	const float c_max_nearfar_ratio = 0.0001;

	if (fov && aspect && near && far) {
		mat4f::Projection proj(*fov, *aspect, *near, *far, is_fovy);
		if (proj.valid()) {
			assert(*near <= *far);
			proj.build_fov_aspect(*this);
			proj.build_near_far(*this);
		}
		return proj.valid();
	}
	else if (fov == nullptr && aspect == nullptr && near && far) {
		mat4f::Projection proj(*this);
		if (proj.valid()) {
			auto new_near = std::max(*near, *far * c_max_nearfar_ratio);
			proj.set_near_far(new_near, *far);
			proj.build_near_far(*this);
		}
		return proj.valid();
	}
	else if (near == nullptr && far == nullptr) {
		mat4f::Projection proj(*this);
		if (proj.valid()) {
			auto new_fov = fov ? *fov : proj.fov(is_fovy);
			auto new_aspect = aspect ? *aspect : proj.aspect();
			proj.set_fov_aspect(new_fov, new_aspect, is_fovy);
			proj.build_fov_aspect(*this);
		}
		return proj.valid();
	}
	else {
		mat4f::Projection proj(*this);
		if (proj.valid()) {
			auto new_fov = fov ? *fov : proj.fov(is_fovy);
			auto new_aspect = aspect ? *aspect : proj.aspect();
			auto new_far = far ? *far : proj.far();
			auto new_near = near ? *near : proj.near();

			new_near = std::max(new_near, new_far * c_max_nearfar_ratio);

			proj.set_fov_aspect(new_fov, new_aspect, is_fovy);
			proj.set_near_far(new_near, new_far);
			proj.build_fov_aspect(*this);
			proj.build_near_far(*this);
		}
		return proj.valid();
	}
}

bool Mat4f::get_projection(float *fov, float *aspect, float *near, float *far, bool is_fovy) const
{
	mat4f::Projection proj(*this);
	if (proj.valid()) {
		if (fov) *fov = proj.fov(is_fovy);
		if (aspect) *aspect = proj.aspect();
		if (far) *far = proj.far();
		if (near) *near = proj.near();
	}
	return proj.valid();
}

Mat4f Mat4f::rot_axis(float radian, const Vec3f &axis) const { return Mat4f(Quatf(radian, axis)) * (*this); }

Mat4f Mat4f::rot(const char *axes, ...) const
{
	va_list args;
	va_start(args, axes);

	Quatf q;
	for (auto i = 0; axes[i]; i++) {
		auto s = axes[i];
		auto a = float(va_arg(args, double));

		switch (s) {
		case 'x': q = Quatf(a, -ex()) * q; break;
		case 'y': q = Quatf(a, -ey()) * q; break;
		case 'z': q = Quatf(a, -ez()) * q; break;
		case 'X': q = Quatf(radians(a), -ex()) * q; break;
		case 'Y': q = Quatf(radians(a), -ey()) * q; break;
		case 'Z': q = Quatf(radians(a), -ez()) * q; break;
		default: assert(0);
		}
	}
	va_end(args);
	return Mat4f(q) * (*this);
}

Mat4f Mat4f::direction_matrix(const Vec3f &eye, const Vec3f &dir)
{
	auto dz = spu::normalize<Vec3f>(dir);
	auto dx = cross_candidate(dz);
	auto dy = spu::normalize<Vec3f>(spu::cross<Vec3f>(dz, dx));
	dx = spu::normalize<Vec3f>(spu::cross(dy, dz));
	return Mat4f(dx, dy, dz, eye);
}

Mat4f Mat4f::cross_product_matrix(const Vec3f &v)
{
	// clang-format off
	return {
		 0.0, -v.z,  v.y, 0.0,
		 v.z,  0.0, -v.x, 0.0,
	        -v.y,  v.x,  0.0, 0.0,
		 0.0,  0.0,  0.0, 0.0
	};
	// clang-format on
}

Mat4f Mat4f::screentexc()
{
	// clang-format off
	return {
	        0.5, 0.0, 0.0, 0.5,
		0.0, 0.5, 0.0, 0.5,
		0.0, 0.0, 0.5, 0.5,
		0.0, 0.0, 0.0, 1.0};
	// clang-format on
}

Mat4f Mat4f::texcscreen()
{
	// clang-format off
	return {
	        2.0, 0.0, 0.0, -1.0,
		0.0, 2.0, 0.0, -1.0,
		0.0, 0.0, 2.0, -1.0,
		0.0, 0.0, 0.0, 1.0};
	// clang-format on
}

Mat4f Mat4f::texcfrag(const Vec4f &viewport)
{
	auto scale = Vec3f(viewport.sx, viewport.sy, 1.0);
	auto offset = Vec3f(viewport.ox, viewport.oy, 0.0);
	return Mat4f().scale(scale).trans(offset);
}

Mat4f Mat4f::fragtexc(const Vec4f &viewport)
{
	auto scale = Vec3f(viewport.sx, viewport.sy, 1.0);
	auto offset = Vec3f(viewport.ox, viewport.oy, 0.0);
	return Mat4f().trans(-offset).scale(1.0f / scale);
}

Mat4f Mat4f::projection(
        double left, double right, double bottom, double top, double near, double far, bool is_perspective)
{
	if (is_perspective) {
		const auto x = (2 * near) / (right - left);
		const auto y = (2 * near) / (top - bottom);
		const auto a = (right + left) / (right - left);
		const auto b = (top + bottom) / (top - bottom);
		const auto c = -(far + near) / (far - near);
		const auto d = -(2 * far * near) / (far - near);

		return {
		        Vec4f(x, 0, 0, 0),
		        Vec4f(0, y, 0, 0),
		        Vec4f(a, b, c, -1),
		        Vec4f(0, 0, d, 0),
		};
	}
	else {
		const auto A = 2.0 / (right - left);
		const auto B = 2.0 / (top - bottom);
		const auto C = -2.0 / (far - near);
		const auto tx = -(right + left) / (right - left);
		const auto ty = -(top + bottom) / (top - bottom);
		const auto tz = -(far + near) / (far - near);

		return {
		        Vec4f(A, 0.0, 0.0, 0.0),
		        Vec4f(0.0, B, 0.0, 0.0),
		        Vec4f(0.0, 0.0, C, 0.0),
		        Vec4f(tx, ty, tz, 1.0),
		};
	}
}

Mat4f Mat4f::projection(const Vec3f &ray, const Plane3f &plane, bool is_perspective)
{
	if (is_perspective) {
		Vec3f raycenter = ray;
		Vec4f eq = plane.eq;
		Mat4f m;
		Mat4f p;
		Mat4f q;
		eq.w += dot<Vec3f>(eq, raycenter);  // quick translate
		eq /= eq.w;

		p.c[3] = {-raycenter.x, -raycenter.y, -raycenter.z, 1};
		q.c[3] = {+raycenter.x, +raycenter.y, +raycenter.z, 1};

		m.c[0] = {1, 0, 0, -eq.x};
		m.c[1] = {0, 1, 0, -eq.y};
		m.c[2] = {0, 0, 1, -eq.z};
		m.c[3] = {0, 0, 0, 0};  // w=0

		return q * m * p;
	}
	else {
		Vec3f raydir = ray;

		float d = dot<Vec3f>(raydir, plane.eq);
		Vec4f eq = plane.eq / d;
		Mat4f m = {
		        {+1 - raydir.x * eq.x, -raydir.y * eq.x,     -raydir.z * eq.x,     0},
		        {-raydir.x * eq.y,     +1 - raydir.y * eq.y, -raydir.z * eq.y,     0},
		        {-raydir.x * eq.z,     -raydir.y * eq.z,     +1 - raydir.z * eq.z, 0},
		        {-raydir.x * eq.w,     -raydir.y * eq.w,     -raydir.z * eq.w,     1},
		};
		return m;
	}
}
// tangent to world
//  | p0.x p1.x pn.x |     | t0.x t1.x tn.x |
//  | p0.y p1.y pn.y | = M | t0.y t1.y tn.y |
//  | p0.z p1.z pn.z |     | t0.z t1.z tn.z |
//
// tangent  : mat.c[0]
// binormal : mat.c[1]
// normal   : mat.c[2]
//
Mat4f Mat4f::tangentworld3(const Vec3f &dp0, const Vec3f &dp1, const Vec3f &dt0, const Vec3f &dt1)
{
	Mat4f pmat = {dp0, dp1, spu::normalize<Vec3f>(spu::cross(dp0, dp1)), Vec4f(0, 0, 0, 1)};
	Mat4f tmat = {dt0, dt1, spu::normalize<Vec3f>(spu::cross(dt0, dt1)), Vec4f(0, 0, 0, 1)};

	return pmat * tmat.inverse();
}

Mat4f Mat4f::tangentworld4(
        const Vec3f &p0, const Vec3f &p1, const Vec3f &p2, const Vec3f &t0, const Vec3f &t1, const Vec3f &t2)
{
	auto tangentworld = tangentworld3(p1 - p0, p2 - p0, t1 - t0, t2 - t0);
	tangentworld.c[3] = Vec4f(p0 - tangentworld.ortho3(t0), 1);

	return tangentworld;
}

//
// 2D quads
//
Mat4f Mat4f::reprojection(const std::vector<Vec2f> &from, const std::vector<Vec2f> &to)
{
	assert(from.size() == 4);
	assert(to.size() == 4);

	auto square_to_quad = [&](const std::vector<Vec2f> &v) {
		Mat4f a;

		auto d1 = v[1] - v[2];
		auto d2 = v[3] - v[2];
		auto d3 = v[0] - v[1] + v[2] - v[3];

		auto denom = d1.x * d2.y - d2.x * d1.y;
		auto n1 = (d3.x * d2.y - d2.x * d3.y) / denom;
		auto n2 = (d1.x * d3.y - d3.x * d1.y) / denom;

		a.c[0] = Vec4f(v[1] - v[0] + n1 * v[1], 0, n1);
		a.c[1] = Vec4f(v[3] - v[0] + n2 * v[3], 0, n2);
		a.c[2] = Vec4f(0, 0, 1, 0);
		a.c[3] = Vec4f(v[0], 0, 1);

		return a;
	};

	auto quad_to_square = [&](const std::vector<Vec2f> &v) { return square_to_quad(v).inverse(); };

	return square_to_quad(to) * quad_to_square(from);
}

bool Mat4f::is_projectable(const std::vector<Vec3f> &points) const
{
	for (const auto &p: points) {
		if (ortho3(p).w < 0) {
			return false;
		}
	}
	return true;
}

Mat4f Mat4f::shift(const std::vector<Vec3f> &points) const
{
	if (!points.empty()) {
		auto range = Range3f(pers3(points));
		return Mat4f(Mat4f(range) * *this);
	}
	return *this;
}

Mat4f Mat4f::shift(const Rectf &from_viewport, const Rectf &to_viewport) const
{
	auto screenfrag0 = texcfrag(from_viewport) * screentexc();
	auto screenfrag1 = texcfrag(to_viewport) * screentexc();
	return screenfrag1.inverse() * screenfrag0 * *this;
}

// y-up only
Mat4f Mat4f::orbiting(
        const Vec3f &target, float esec, float radius, float radius_amplitude, float radius_period,
        float azimuth, float azimuth_period, float elevation, float elevation_amplitude, float elevation_period,
        const Vec3f &up)
{
	if (radius_period && radius_amplitude) {
		radius += sinf(esec / radius_period * radians(360.0)) * radius_amplitude;
	}
	if (azimuth_period) {
		azimuth += esec / azimuth_period * radians(360.0);
	}
	if (elevation_period && elevation_amplitude) {
		elevation += sinf(esec / elevation_period * radians(360.0)) * elevation_amplitude;
	}
	elevation = radians(elevation);

	auto q0 = Quatf(elevation, -ey());  // rot -y
	auto q1 = Quatf(azimuth, ez());     // rot z
	auto q2 = Quatf::from_target(up, ez());

	auto dir = q2 * q1 * q0 * -ex();  // negative
	auto eye = Vec3f(target - dir * radius);

	Mat4f m;
	m.set_orientation(&eye, &dir, &up);
	return m.inverse();
}

std::vector<Mat4f> Mat4f::cube_matrices(const Vec3f &capture_position)
{
	auto setaxis = [](const Vec3f &dx, const Vec3f &dy, const Vec3f &dz, const Vec3f &eye) {
		Mat4f rot = {Vec4f(dx, 0), Vec4f(dy, 0), Vec4f(dz, 0), Vec4f(0, 0, 0, 1)};
		return rot.trans(eye).inverse();
	};

	const auto px = ex();
	const auto py = ey();
	const auto pz = ez();
	const auto nx = -px;
	const auto ny = -py;
	const auto nz = -pz;

	std::vector<Mat4f> worldviews = {
	        setaxis(nz, ny, nx, capture_position),  // positive-x
	        setaxis(pz, ny, px, capture_position),  // negative-x
	        setaxis(px, pz, ny, capture_position),  // positive-y
	        setaxis(px, nz, py, capture_position),  // negative-y
	        setaxis(px, ny, nz, capture_position),  // positive-z
	        setaxis(nx, ny, pz, capture_position),  // negative-z
	};
	return worldviews;
}

Mat4f Mat4f::transpose3() const
{
	Mat4f m1;

	const auto vz = ezero<Vec4f>();
	const auto t0 = sse::unpacklo(c[0].fv, c[2].fv);  // |m21|m01|m20|m00|
	const auto t1 = sse::unpacklo(c[1].fv, vz.fv);    // |m31|m11|m30|m10|
	const auto t2 = sse::unpackhi(c[0].fv, c[2].fv);  // |m23|m03|m22|m02|
	const auto t3 = sse::unpackhi(c[1].fv, vz.fv);    // |m33|m13|m32|m12|

	m1.c[0] = sse::unpacklo(t0, t1);  // |m30|m20|m10|m00|
	m1.c[1] = sse::unpackhi(t0, t1);  // |m31|m21|m11|m01|
	m1.c[2] = sse::unpacklo(t2, t3);  // |m32|m22|m12|m02|

	return m1;
}

// see "xmmintrin.h" also
Mat4f Mat4f::transpose4() const
{
	Mat4f m1;

	const auto t0 = sse::unpacklo(c[0].fv, c[2].fv);  // |m21|m01|m20|m00|
	const auto t1 = sse::unpacklo(c[1].fv, c[3].fv);  // |m31|m11|m30|m10|
	const auto t2 = sse::unpackhi(c[0].fv, c[2].fv);  // |m23|m03|m22|m02|
	const auto t3 = sse::unpackhi(c[1].fv, c[3].fv);  // |m33|m13|m32|m12|

	m1.c[0] = sse::unpacklo(t0, t1);  // |m30|m20|m10|m00|
	m1.c[1] = sse::unpackhi(t0, t1);  // |m31|m21|m11|m01|
	m1.c[2] = sse::unpacklo(t2, t3);  // |m32|m22|m12|m02|
	m1.c[3] = sse::unpackhi(t2, t3);  // |m33|m23|m13|m03|
	return m1;
}

Mat4f Mat4f::inverse() const
{
	auto inv = eric::Inverse(*this);
	return inv;
}

}  // namespace spu
