
// #include <lib/incl_begin.ipp>
// #include <lib/incl_end.ipp>
// #include <math/angle.hpp>
#include <cstdlib>

namespace spu::oglplus::images {

inline void Cloud::adjust_sphere(Vec3f &center, float &radius) const
{
	float c[3] = {center.x, center.y, center.z};
	for (auto i = 0u; i != 3; ++i) {
		if (c[i] < -1.0f) {
			radius *= 0.5f;
			c[i] = -1.0f + radius;
		}
		if (c[i] > 1.0f) {
			radius *= 0.5f;
			c[i] = 1.0f - radius;
		}
		if ((c[i] - radius) < -1.0f) {
			auto d = (-1.0f - c[i] + radius) * 0.51f;
			assert(d >= 0.0f);
			radius -= d;
			c[i] += d;
		}
		if ((c[i] + radius) > 1.0f) {
			auto d = (c[i] + radius - 1.0f) * 0.51f;
			assert(d >= 0.0f);
			radius -= d;
			c[i] -= d;
		}
	}
	center = Vec3f(c[0], c[1], c[2]);
}

inline bool Cloud::apply_sphere(const Vec3f &center, float radius)
{
	assert(radius > 0.0f);
	auto something_updated = false;

	auto c = center * 0.5f + Vec3f(0.5f, 0.5f, 0.5f);
	auto r = radius * 0.5f;
	auto w = width();
	auto h = height();
	auto d = depth();
	auto *data = begin();

	for (int32_t k = (c.z - r) * d, ke = (c.z + r) * d; k != ke; ++k) {
		for (int32_t j = (c.y - r) * h, je = (c.y + r) * h; j != je; ++j) {
			for (int32_t i = (c.x - r) * w, ie = (c.x + r) * w; i != ie; ++i) {
				assert(k >= 0 && k < d);
				assert(j >= 0 && j < h);
				assert(i >= 0 && i < w);
				auto n = k * w * h + j * w + i;
				auto b = data[n];
				if (b != 0xFF) {
					auto cd = float(b) / float(0xFF);
					auto p = Vec3f(float(i) / w, float(j) / h, float(k) / d);
					auto nd = (r - distance(c, p)) / r;
					if (nd < 0.0f) {
						nd = 0.0f;
					}
					nd = std::sqrt(nd);
					nd += cd;
					if (nd > 1.0f) {
						nd = 1.0f;
					}
					data[n] = 0xFF * nd;
					something_updated = true;
				}
			}
		}
	}
	return something_updated;
}

inline float Cloud::rand_u() { return float(std::rand()) / float(RAND_MAX); }

inline float Cloud::rand_s() { return (rand_u() - 0.5f) * 2.0f; }

inline void Cloud::make_spheres(Vec3f center, float radius)
{
	adjust_sphere(center, radius);
	if (radius < m_min_radius) {
		return;
	}
	if (!apply_sphere(center, radius)) {
		return;
	}
	auto sub_radius = radius * m_sub_scale;
	auto i = 0;
	auto n = int32_t((8.0f * radius * radius) / (sub_radius * sub_radius));
	while (i != n) {
		auto c_2pi = pi() * 2.0f;
		auto rad = radius * (1.0f + rand_s() * m_sub_variance * 0.5f);
		auto rho = rand_u() * c_2pi;
		auto phi = rand_s() * c_2pi / 4.0f;

		auto s_rho = sinf(rho);
		auto c_rho = cosf(rho);
		auto s_phi = sinf(phi);
		auto c_phi = cosf(phi);

		make_spheres(
		        center + Vec3f(rad * c_phi * c_rho, rad * s_phi, rad * c_phi * s_rho),
		        sub_radius * (1.0f + rand_s() * m_sub_variance));
		++i;
	}
}

inline Cloud::Cloud(
        int32_t width, int32_t height, int32_t depth, const Vec3f &origin, float init_radius, float sub_scale,
        float sub_variance, float min_radius)
        : Image(width, height, depth, 1, static_cast<uint8_t *>(nullptr)), m_sub_scale(sub_scale),
          m_sub_variance(sub_variance), m_min_radius(min_radius)
{
	std::fill(this->begin(), this->end(), u_char(0));
	make_spheres(origin, init_radius);
}

inline Cloud2D::Cloud2D(const Cloud &cloud)
        : Image(cloud.width(), cloud.height(), 1, 3, static_cast<uint8_t *>(nullptr))
{
	auto *p = this->begin();
	auto *e = this->end();
	auto w = width();
	auto h = height();
	auto d = cloud.depth();
	for (int32_t j = 0; j != h; ++j) {
		for (int32_t i = 0; i != w; ++i) {
			auto depth_near = 0;
			auto depth_far = 0;
			auto total_density = 0;
			for (int32_t k = 0; k != d; ++k) {
				auto ppos = cloud.pixelPos(i, j, k);
				auto c = ((const uint8_t *)cloud.data())[ppos];

				// auto c = cloud.componentAs<uint8_t>(i, j, k, 0);
				if (depth_near == 0) {
					if (c != 0) {
						depth_near = (256 * k) / d;
						depth_far = depth_near;
					}
				}
				else if (depth_far == depth_near) {
					if (c == 0) {
						depth_far = (256 * k) / d;
					}
				}
				total_density += c;
			}
			assert(depth_far >= depth_near);
			auto avg_density
			        = ((depth_far - depth_near) > 0) ? total_density / (depth_far - depth_near) : 0;
			assert(p != e);
			*p = depth_near;
			++p;
			assert(p != e);
			*p = depth_far;
			++p;
			assert(p != e);
			*p = u_char(avg_density);
			++p;
		}
	}
	assert(p == e);
}

}  // namespace spu::oglplus::images
