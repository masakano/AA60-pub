
namespace spu::oglplus::shapes {

inline uint32_t TwistedTorus::positions(std::vector<float> &dest) const
{
	using T = float;
	dest.resize(2 * 2 * 2 * m_sections * (m_rings + 1) * 3);
	auto k = 0u;
	const auto t = m_thickness / m_radius_in;
	const auto r_twist = double(m_twist) / double(m_rings);
	const auto r_step = (2.0 * pi<double>()) / double(m_rings);
	const auto s_step = (2.0 * pi<double>()) / double(m_sections);
	const auto s_slip = s_step * m_s_slip_coef;
	const auto r1 = m_radius_in;
	const auto r2 = m_radius_out - m_radius_in;

	for (auto f = 0; f != 2; ++f) {
		const auto f_sign = (f == 0) ? 1.0 : -1.0;
		const auto fdt = t * f_sign * 0.95;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const double sa[2] = {s_angle + s_slip * f_sign, s_angle - s_slip * f_sign};
			for (auto r = 0u; r != m_rings + 1; ++r) {
				const auto r_angle = r * r_step;
				auto vx = std::cos(r_angle);
				auto vz = std::sin(r_angle);

				const auto ta = s_step * r * r_twist;

				for (auto d = 0; d != 2; ++d) {
					auto vr = std::cos(sa[d] + ta);
					auto vy = std::sin(sa[d] + ta);

					dest[k++] = T(vx * (r1 + r2 * (1.0 + vr) + fdt * vr));
					dest[k++] = T(vy * (r2 + fdt));
					dest[k++] = T(vz * (r1 + r2 * (1.0 + vr) + fdt * vr));
				}
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		const auto d_sign = (d == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const auto sa = s_angle + s_slip * d_sign;
			for (auto r = 0u; r != m_rings + 1; ++r) {
				const auto r_angle = r * r_step;
				const auto ta = s_step * r * r_twist;
				const auto vr = std::cos(sa + ta);
				const auto vy = std::sin(sa + ta);

				auto vx = std::cos(r_angle);
				auto vz = std::sin(r_angle);

				for (auto f = 0; f != 2; ++f) {
					const auto f_sign = (f == 0) ? 1.0 : -1.0;
					const auto fdt = -t * d_sign * f_sign * 0.95;

					dest[k++] = T(vx * (r1 + r2 * (1.0 + vr) + fdt * vr));
					dest[k++] = T(vy * (r2 + fdt));
					dest[k++] = T(vz * (r1 + r2 * (1.0 + vr) + fdt * vr));
				}
			}
		}
	}
	assert(k == dest.size());
	return 3;
}

inline uint32_t TwistedTorus::normals(std::vector<float> &dest) const
{
	using T = float;
	dest.resize(2 * 2 * 2 * m_sections * (m_rings + 1) * 3);
	auto k = 0u;
	const auto r_twist = double(m_twist) / double(m_rings);
	const auto r_step = (2.0 * pi<double>()) / double(m_rings);
	const auto s_step = (2.0 * pi<double>()) / double(m_sections);
	const auto s_slip = s_step * m_s_slip_coef;

	for (auto f = 0; f != 2; ++f) {
		const auto f_sign = (f == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const double sa[2] = {s_angle + s_slip * f_sign, s_angle - s_slip * f_sign};
			for (auto r = 0u; r != m_rings + 1; ++r) {
				const auto r_angle = r * r_step;
				auto vx = std::cos(r_angle);
				auto vz = std::sin(r_angle);

				const auto ta = s_step * r * r_twist;

				for (auto d = 0; d != 2; ++d) {
					auto vr = std::cos(sa[d] + ta);
					auto vy = std::sin(sa[d] + ta);

					dest[k++] = T(f_sign * vx * vr);
					dest[k++] = T(f_sign * vy);
					dest[k++] = T(f_sign * vz * vr);
				}
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		const auto d_sign = (d == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const auto sa = s_angle + s_slip * d_sign;
			for (auto r = 0u; r != m_rings + 1; ++r) {
				const auto ta = s_step * r * r_twist;
				const auto vr = std::sin(sa + ta);
				const auto vy = std::cos(sa + ta);
				const auto r_angle = r * r_step;
				auto vx = std::cos(r_angle);
				auto vz = std::sin(r_angle);

				for (auto f = 0; f != 2; ++f) {
					dest[k++] = T(d_sign * -vx * vr);
					dest[k++] = T(d_sign * vy);
					dest[k++] = T(d_sign * -vz * vr);
				}
			}
		}
	}

	assert(k == dest.size());
	return 3;
}

inline uint32_t TwistedTorus::tangents(std::vector<float> &dest) const
{
	using T = float;
	std::vector<T> poss;
	positions(poss);

	dest.resize(2 * 2 * 2 * m_sections * (m_rings + 1) * 3);

	assert(dest.size() == poss.size());
	auto k = 0u;
	for (auto f = 0; f != 2; ++f) {
		auto foff = f * m_sections * (m_rings + 1) * 2;
		for (auto s = 0u; s != m_sections; ++s) {
			auto s0 = s * (m_rings + 1) * 2;
			for (auto r = 0u; r != m_rings + 1; ++r) {
				auto s1 = s0;
				auto r0 = r;
				auto r1 = r + 1;
				if (r == m_rings) {
					s1 = ((s + m_twist) % m_sections) * (m_rings + 1) * 2;
					r1 = 1;
				}

				for (auto d = 0; d != 2; ++d) {
					std::size_t k0 = foff + s0 + r0 * 2 + d;
					std::size_t k1 = foff + s1 + r1 * 2 + d;

					T tx = poss[k1 * 3 + 0] - poss[k0 * 3 + 0];
					T ty = poss[k1 * 3 + 1] - poss[k0 * 3 + 1];
					T tz = poss[k1 * 3 + 2] - poss[k0 * 3 + 2];
					T tl = std::sqrt(tx * tx + ty * ty + tz * tz);

					assert(tl > T(0));

					dest[k++] = tx / tl;
					dest[k++] = ty / tl;
					dest[k++] = tz / tl;
				}
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		std::size_t doff = d * m_sections * (m_rings + 1) * 2;
		for (auto s = 0u; s != m_sections; ++s) {
			auto s0 = s * (m_rings + 1) * 2;
			for (auto r = 0u; r != m_rings + 1; ++r) {
				auto s1 = s0;
				auto r0 = r;
				auto r1 = r + 1;
				if (r == m_rings) {
					s1 = ((s + m_twist) % m_sections) * (m_rings + 1) * 2;
					r1 = 1;
				}

				for (auto f = 0; f != 2; ++f) {
					std::size_t k0 = doff + s0 + r0 * 2 + f;
					std::size_t k1 = doff + s1 + r1 * 2 + f;

					T tx = poss[k1 * 3 + 0] - poss[k0 * 3 + 0];
					T ty = poss[k1 * 3 + 1] - poss[k0 * 3 + 1];
					T tz = poss[k1 * 3 + 2] - poss[k0 * 3 + 2];
					T tl = std::sqrt(tx * tx + ty * ty + tz * tz);

					assert(tl > T(0));

					dest[k++] = tx / tl;
					dest[k++] = ty / tl;
					dest[k++] = tz / tl;
				}
			}
		}
	}

	assert(k == dest.size());
	return 3;
}

inline uint32_t TwistedTorus::bitangents(std::vector<float> &dest) const
{
	using T = float;
	std::vector<T> nmls;
	std::vector<T> tgts;
	normals(nmls);
	tangents(tgts);
	assert(nmls.size() == tgts.size());
	assert(nmls.size() % 3 == 0);

	dest.resize(nmls.size());

	auto k = 0u;

	while (k != dest.size()) {
		T nx = nmls[k + 0];
		T ny = nmls[k + 1];
		T nz = nmls[k + 2];

		T tx = tgts[k + 0];
		T ty = tgts[k + 1];
		T tz = tgts[k + 2];

		dest[k++] = T(ny * tz - nz * ty);
		dest[k++] = T(nz * tx - nx * tz);
		dest[k++] = T(nx * ty - ny * tx);
	}

	assert(k == dest.size());
	return 3;
}

inline uint32_t TwistedTorus::texCoordinates(std::vector<float> &dest) const
{
	using T = float;
	dest.resize(2 * 2 * 2 * m_sections * (m_rings + 1) * 2);
	auto k = 0u;
	auto t = m_thickness / m_radius_in;
	auto r_step = 0.5 / double(m_rings);
	auto s_step = 1.0 / double(m_sections);
	auto s_slip = s_step * t;

	s_slip = s_step * m_s_slip_coef;

	for (auto f = 0; f != 2; ++f) {
		const auto f_sign = (f == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const double sa[2] = {s_angle + s_slip * f_sign, s_angle - s_slip * f_sign};
			for (auto r = 0u; r != m_rings + 1; ++r) {
				const auto r_angle = 2 * r * r_step;
				const auto u = r_angle;
				for (auto d = 0; d != 2; ++d) {
					auto v = sa[d];
					dest[k++] = T(u);
					dest[k++] = T(v);
				}
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		const auto d_sign = (d == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const auto v = s_angle + s_slip * d_sign;
			for (auto r = 0u; r != m_rings + 1; ++r) {
				const auto r_angle = 2 * r * r_step;
				const auto u = r_angle;
				for (auto f = 0; f != 2; ++f) {
					dest[k++] = T(u);
					dest[k++] = T(v);
				}
			}
		}
	}

	assert(k == dest.size());
	return 2;
}

inline std::vector<spu::SpuCommand> TwistedTorus::instructions(TwistedTorus::DefaultTag /*unused*/) const
{
	std::vector<spu::SpuCommand> instructions;
	auto strip = 2 * (m_rings + 1);
	auto offs = 0;

	auto phase = 0;

	for (auto f = 0; f != 4; ++f) {
		for (auto s = 0u; s != m_sections; ++s) {
			spu::SpuCommand com;
			com.target = GL_ARRAY_BUFFER;
			com.mode = GL_TRIANGLE_STRIP;
			com.first = uint32_t(offs);
			com.count = uint32_t(strip);
			com.flags = phase;
			instructions.push_back(com);
			offs += strip;
		}
		++phase;
	}
	return instructions;
}

}  // namespace spu::oglplus::shapes
