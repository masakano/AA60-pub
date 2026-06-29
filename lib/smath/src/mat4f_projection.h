//
// Projection :
//
#pragma once
#include <smath/mat4f.h>

namespace spu::mat4f {

// conventional
// pers:       ortho:
// | x 0 0 0 | | x 0 0 * |
// | 0 y 0 0 | | 0 y 0 * |
// | 0 0 c d | | 0 0 c d |
// | 0 0-1 0 | | 0 0 0 1 | (c < 0, d < 0)

struct Projection {
	const Vec4f c_err = Vec4f(epsilon());

	bool is_ortho = false;
	bool is_pers = true;

	double x, y, c, d;

	Projection(const Mat4f &m)
	{
		is_ortho = is_good_ortho(m);
		is_pers = is_good_pers(m);

		x = m.c[0].x;
		y = m.c[1].y;
		c = m.c[2].z;
		d = m.c[3].z;
	}

	Projection(double fov, double aspect, double near, double far, bool is_fovy)
	{
		set_fov_aspect(fov, aspect, is_fovy);
		set_near_far(near, far);
	}

	void set_near_far(double near, double far)
	{
		if (is_ortho) {
			c = -(far - near) / 2.0;
			d = -(far + near) / 2.0;
		}
		else {
			c = -(far + near) / (far - near);
			d = -(2 * far * near) / (far - near);
		}
	}

	void set_fov_aspect(double fov, double aspect, bool is_fovy)
	{
		if (is_fovy) {
			if (is_ortho) {
				x = y / aspect;
			}
			else {
				y = 1.0 / tan(radians(fov * 0.5));
				x = y / aspect;
			}
		}
		else {
			if (is_ortho) {
				y = x * aspect;
			}
			else {
				x = 1.0 / tan(radians(fov * 0.5));
				y = x * aspect;
			}
		}
	}

	double fov(bool is_fovy) const
	{
		if (is_fovy) {
			return degrees(atan2(1.0, y)) * 2.0;
		}
		else {
			return degrees(atan2(1.0, x)) * 2.0;
		}
	}

	double aspect() const { return y / x; }

	double near() const
	{
		if (is_ortho) {
			return (d - 1) / c;
		}
		if (is_pers) {
			return d / (c - 1);
		}
		aux_error(true, "not valid matrix\n");
	}

	double far() const
	{
		if (is_ortho) {
			return (d + 1) / c;
		}
		if (is_pers) {
			return d / (c + 1);
		}
		aux_error(true, "not valid matrix\n");
	}

	void build_fov_aspect(Mat4f &m) const
	{
		if (is_ortho || is_pers) {
			m.c[0] = Vec4f(x, 0, 0, 0);
			m.c[1] = Vec4f(0, y, 0, 0);
		}
	}

	void build_near_far(Mat4f &m) const
	{
		if (is_ortho) {
			m.c[2] = Vec4f(0, 0, c, 0);
			m.c[3] = Vec4f(0, 0, d, 1);
		}
		if (is_pers) {
			m.c[2] = Vec4f(0, 0, c, -1);
			m.c[3] = Vec4f(0, 0, d, 0);
		}
	}

	bool valid() const { return is_ortho || is_pers; }

	bool is_good_ortho(const Mat4f &m) const
	{
		auto d = ((m.c[0].abs() > c_err) & Vec4i(0, -1, -1, -1))
		       | ((m.c[1].abs() > c_err) & Vec4i(-1, 0, -1, -1))
		       | ((m.c[2].abs() > c_err) & Vec4i(-1, -1, 0, -1));

		return d.pack() == 0;
	}
	bool is_good_pers(const Mat4f &m) const
	{
		auto d = ((m.c[0].abs() > c_err) & Vec4i(0, -1, -1, -1))
		       | ((m.c[1].abs() > c_err) & Vec4i(-1, 0, -1, -1))
		       | ((m.c[2].abs() > c_err) & Vec4i(-1, -1, 0, 0))
		       | ((m.c[3].abs() > c_err) & Vec4i(-1, -1, 0, -1));
		return d.pack() == 0;
	}
};
}  // namespace spu::mat4f
