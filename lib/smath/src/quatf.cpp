//
// Quatf :
//
#include <smath/quatf.h>

namespace spu {
static_assert(std::is_trivially_copyable<Quatf>::value, "not standard layout");
static_assert(std::is_trivially_copyable<Transformf>::value, "not standard layout");
static_assert(std::is_standard_layout<Transformf>::value, "not standard layout");

Quatf::Quatf(const Mat4f &mat)
{
	auto signf = [](float x) { return (x >= 0.0f) ? +1.0f : -1.0f; };

	auto r11 = mat.c[0].x;
	auto r21 = mat.c[0].y;
	auto r31 = mat.c[0].z;

	auto r12 = mat.c[1].x;
	auto r22 = mat.c[1].y;
	auto r32 = mat.c[1].z;

	auto r13 = mat.c[2].x;
	auto r23 = mat.c[2].y;
	auto r33 = mat.c[2].z;

	auto q0 = (r11 + r22 + r33 + 1.0F) / 4.0F;
	auto q1 = (r11 - r22 - r33 + 1.0F) / 4.0F;
	auto q2 = (-r11 + r22 - r33 + 1.0F) / 4.0F;
	auto q3 = (-r11 - r22 + r33 + 1.0F) / 4.0F;

	q0 = sqrtf(std::max(q0, 0.0F));
	q1 = sqrtf(std::max(q1, 0.0F));
	q2 = sqrtf(std::max(q2, 0.0F));
	q3 = sqrtf(std::max(q3, 0.0F));

	if (q0 >= q1 && q0 >= q2 && q0 >= q3) {
		q0 *= +1.0F;
		q1 *= signf(r32 - r23);
		q2 *= signf(r13 - r31);
		q3 *= signf(r21 - r12);
	}
	else if (q1 >= q0 && q1 >= q2 && q1 >= q3) {
		q0 *= signf(r32 - r23);
		q1 *= +1.0F;
		q2 *= signf(r21 + r12);
		q3 *= signf(r13 + r31);
	}
	else if (q2 >= q0 && q2 >= q1 && q2 >= q3) {
		q0 *= signf(r13 - r31);
		q1 *= signf(r21 + r12);
		q2 *= +1.0F;
		q3 *= signf(r32 + r23);
	}
	else {
		q0 *= signf(r21 - r12);
		q1 *= signf(r31 + r13);
		q2 *= signf(r32 + r23);
		q3 *= +1.0F;
	}
	*this = Quatf(q1, q2, q3, q0).normalize();
}

Quatf::operator Mat4f() const
{
	Mat4f m;

	auto xx = 2.0 * x * x;
	auto yy = 2.0 * y * y;
	auto zz = 2.0 * z * z;

	auto yz = 2.0 * y * z;
	auto zx = 2.0 * z * x;
	auto xy = 2.0 * x * y;

	auto wx = 2.0 * w * x;
	auto wy = 2.0 * w * y;
	auto wz = 2.0 * w * z;

	m.c[0] = Vec4f(1 - yy - zz, xy + wz, zx - wy, 0.0f);
	m.c[1] = Vec4f(xy - wz, 1 - xx - zz, yz + wx, 0.0f);
	m.c[2] = Vec4f(zx + wy, yz - wx, 1 - xx - yy, 0.0f);

	return m;
}

Quatf::Quatf(float radian, const Vec3f &axis)
        : Quatf(Vec4f(spu::normalize<Vec3f>(axis) * sinf(radian / 2), cosf(radian / 2)))
{
}

Quatf Quatf::slerp(const Quatf &q1, float rate) const
{
	auto v0 = Vec4f(*this);
	auto v1 = Vec4f(q1);

	auto dot = spu::dot(v0, v1);
	if (dot < 0.0) {  // for "long-path problem"
		v1 = -v1;
		dot = -dot;
	}
	if (dot < 1.0) {
		auto radian = acosf(dot);
		if (radian != 0) {
			auto d = sinf(radian);
			auto s0 = sinf((1.0 - rate) * radian);
			auto s1 = sinf(rate * radian);
			return Quatf((v0 * s0 + v1 * s1) / d);
		}
	}
	return Quatf(lerp(v0, v1, rate));
}

Quatf Quatf::from_target(const Vec3f &target, const Vec3f &effect, float radian)
{
	auto dot = spu::dot(target, effect);  // assummes unit vector
	auto axis = cross(effect, target);
	if (-1.0 < dot && dot < 1.0) {
		return Quatf(std::min(radian, acosf(dot)), axis);
	}
	else if (dot > 0) {
		return Quatf(0, 0, 0, 1);
	}
	aux_message(0, "cross product is zero. divide angle and try again\n");
	return Quatf(0, 0, 0, 1);
}

Quatf Quatf::from_target_and_axis(const Vec3f &target, const Vec3f &effect, const Vec3f &axis, float radian)
{
	auto effect0 = spu::normalize<Vec3f>((effect - axis * spu::dot(effect, axis)));
	auto target0 = spu::normalize<Vec3f>((target - axis * spu::dot(target, axis)));
	auto axis0 = cross(effect0, target0);
	auto dot = spu::dot<Vec3f>(target0, effect0);

	if (-1.0 < dot && dot < 1.0) {
		return Quatf(std::min(radian, acosf(dot)), axis0);
	}
	if (dot > 0) {
		return Quatf(0, 0, 0, 1);
	}
	aux_message(0, "cross product is zero. divide angle and try again\n");
	return Quatf(0, 0, 0, 1);
}

Quatf Quatf::from_eulerXYZ(const Vec3f &euler)
{
	auto qx = Quatf(euler.x, ex());
	auto qy = Quatf(euler.y, ey());
	auto qz = Quatf(euler.z, ez());
	return qx * qy * qz;
}

Quatf Quatf::from_eulerZYX(const Vec3f &euler)
{
	auto qx = Quatf(euler.x, ex());
	auto qy = Quatf(euler.y, ey());
	auto qz = Quatf(euler.z, ez());
	return qz * qy * qx;
}

/* ------------------------------------------------------------------------------------

        (c* = cos(r*), s* = sin(r*))

             |   1   0   0 |	     |  cy   0 -sy |	     |  cz  sz   0 |
        Rx = |   0  cx  sx |	Ry = |   0   1   0 |	Rz = | -sz  cz   0 |
             |   0 -sx  cx |	     |  sy   0  cy |	     |   0   0   1 |

                   | cz*cy  cz*sx*sy+sz*cx -cz*sy*cx+sz*sx |   | m00 m10 m20 |
        Rz*Ry*Rx = |-sz*cy -sz*sx*sy+cz*cx  sz*sy*cx+cz*sx | = | m01 m11 m21 |
                   |    sy          -cy*sx           cy*cx |   | m02 m12 m22 |

            sy = m02         y =  asin(m02)
        -sz/cz = m01/m00     z = -atan2(m01, m00)
         sx/cx = m12/m22     x =  atan2(m12, m22)

 --------------------------------------------------------------------------------------*/

Vec3f Quatf::to_euler() const
{
	auto m0 = Mat4f(inverse());

	auto m00 = m0.c[0].x;
	auto m01 = m0.c[0].y;
	auto m02 = m0.c[0].z;

	auto m12 = m0.c[1].z;
	auto m22 = m0.c[2].z;

	auto rx = -std::atan2(m12, m22);
	auto ry = std::asin(m02);
	auto rz = -std::atan2(m01, m00);

	return {rx, ry, rz};
}

void Transformf::report(const char *str) const
{
	if (str && *str) aux_printf("%s:\n", str);

	t.report("t");
	q.report("q");
}

}  // namespace spu
