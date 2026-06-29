
namespace spu::oglplus::shapes {

inline uint32_t WickerTorus::positions(std::vector<float> &dest) const
{
	using T = float;
	dest.resize(
	        2 * 2 * 2 * m_rings * (m_sections * 3 + 1) * 3
	        + 2 * 2 * 2 * m_sections * (m_rings * 2 + 1) * 3);
	auto k = 0u;
	const auto t = m_thickness / m_radius_in;
	const auto r_step = (2.0 * pi<double>()) / double(m_rings);
	const auto s_step = (2.0 * pi<double>()) / double(m_sections);
	const auto r_slip = r_step * m_r_slip_coef;
	const auto s_slip = s_step * m_s_slip_coef;
	const auto r1 = m_radius_in;
	const auto r2 = m_radius_out - m_radius_in;

	for (auto f = 0; f != 2; ++f) {
		const auto f_sign = (f == 0) ? 1.0 : -1.0;
		const auto fdt = t * f_sign * 0.5;
		const auto rfs = f_sign * r_slip;
		for (auto r = 0u; r != m_rings; ++r) {
			const auto r_angle = r * r_step;
			const auto r_sign = (r % 2 == 0) ? 1.0 : -1.0;
			const auto rdt = t * r_sign * 2.0;
			for (auto s = 0u; s != m_sections; ++s) {
				const double sa[3]
				        = {s * s_step, (s + t) * s_step, (s + 1.0 - 2.0 * t) * s_step};
				const double rd[3]
				        = {fdt, fdt + ((s % 2 == 0) ? -rdt : rdt),
				           fdt + ((s % 2 == 0) ? -rdt : rdt)};
				for (auto p = 0; p != 3; ++p) {
					auto vr = std::cos(sa[p]);
					auto vy = std::sin(sa[p]);
					auto vs = 0.5 + vr * 0.5;
					for (auto d = 0; d != 2; ++d) {
						const auto d_sign = (d == 0) ? -1.0 : 1.0;

						auto rs_angle = r_angle + d_sign * rfs * (1.0 - 0.25 * vs);

						auto vx = std::cos(rs_angle);
						auto vz = std::sin(rs_angle);

						dest[k++] = T(vx * (r1 + r2 * (1.0 + vr) + rd[p] * vr));
						dest[k++] = T(vy * (r2 + rd[p]));
						dest[k++] = T(vz * (r1 + r2 * (1.0 + vr) + rd[p] * vr));
					}
				}
			}

			for (auto d = 0; d != 2; ++d) {
				const auto d_sign = (d == 0) ? -1.0 : 1.0;
				auto rs_angle = r_angle + d_sign * rfs * 0.75;

				auto vx = std::cos(rs_angle);
				auto vz = std::sin(rs_angle);
				dest[k++] = T(vx * (r1 + r2 * (2.0) + fdt));
				dest[k++] = T(0.0);
				dest[k++] = T(vz * (r1 + r2 * (2.0) + fdt));
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		const auto d_sign = (d == 0) ? 1.0 : -1.0;
		const auto rds = d_sign * r_slip;
		for (auto r = 0u; r != m_rings; ++r) {
			const auto r_angle = r * r_step;
			const auto r_sign = (r % 2 == 0) ? 1.0 : -1.0;
			const auto rdt = t * r_sign * 2.0;
			for (auto s = 0u; s != m_sections; ++s) {
				const double sa[3] = {s * s_step, (s + t) * s_step, (s + 1.0 - 2 * t) * s_step};
				const double rd[3]
				        = {0.0, 0.0 + ((s % 2 == 0) ? -rdt : rdt),
				           0.0 + ((s % 2 == 0) ? -rdt : rdt)};
				for (auto p = 0; p != 3; ++p) {
					auto vr = std::cos(sa[p]);
					auto vy = std::sin(sa[p]);
					auto vs = 0.5 + vr * 0.5;

					auto rs_angle = r_angle + rds * (1.0 - 0.25 * vs);

					auto vx = std::cos(rs_angle);
					auto vz = std::sin(rs_angle);

					for (auto f = 0; f != 2; ++f) {
						const auto f_sign = (f == 0) ? 1.0 : -1.0;
						const auto fdt = 0.5 * t * f_sign * d_sign;
						dest[k++] = T(vx * (r1 + r2 * (1.0 + vr) + (fdt + rd[p]) * vr));
						dest[k++] = T(vy * (r2 + (fdt + rd[p])));
						dest[k++] = T(vz * (r1 + r2 * (1.0 + vr) + (fdt + rd[p]) * vr));
					}
				}
			}
			auto rs_angle = r_angle + rds * 0.75;

			auto vx = std::cos(rs_angle);
			auto vz = std::sin(rs_angle);
			for (auto f = 0; f != 2; ++f) {
				const auto f_sign = (f == 0) ? 1.0 : -1.0;
				const auto fdt = 0.5 * t * f_sign * d_sign;
				dest[k++] = T(vx * (r1 + r2 * (2.0) + fdt));
				dest[k++] = T(0.0);
				dest[k++] = T(vz * (r1 + r2 * (2.0) + fdt));
			}
		}
	}

	for (auto f = 0; f != 2; ++f) {
		const auto f_sign = (f == 0) ? 1.0 : -1.0;
		const auto fdt = t * f_sign * 0.95;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const double sa[2] = {s_angle + s_slip * f_sign, s_angle - s_slip * f_sign};
			for (auto r = 0u; r != m_rings; ++r) {
				const auto r_angle = r * r_step;
				const double ra[2] = {r_angle + r_slip, r_angle + r_step - r_slip};
				for (auto p = 0; p != 2; ++p) {
					auto vx = std::cos(ra[p]);
					auto vz = std::sin(ra[p]);

					for (auto d = 0; d != 2; ++d) {
						auto vr = std::cos(sa[d]);
						auto vy = std::sin(sa[d]);

						dest[k++] = T(vx * (r1 + r2 * (1.0 + vr) + fdt * vr));
						dest[k++] = T(vy * (r2 + fdt));
						dest[k++] = T(vz * (r1 + r2 * (1.0 + vr) + fdt * vr));
					}
				}
			}
			auto vx = std::cos(r_slip);
			auto vz = std::sin(r_slip);

			for (auto d = 0; d != 2; ++d) {
				auto vr = std::cos(sa[d]);
				auto vy = std::sin(sa[d]);

				dest[k++] = T(vx * (r1 + r2 * (1.0 + vr) + fdt * vr));
				dest[k++] = T(vy * (r2 + fdt));
				dest[k++] = T(vz * (r1 + r2 * (1.0 + vr) + fdt * vr));
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		const auto d_sign = (d == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const auto sa = s_angle + s_slip * d_sign;
			const auto vr = std::cos(sa);
			const auto vy = std::sin(sa);
			for (auto r = 0u; r != m_rings; ++r) {
				const auto r_angle = r * r_step;
				const double ra[2] = {r_angle + r_slip, r_angle + r_step - r_slip};
				for (auto p = 0; p != 2; ++p) {
					auto vx = std::cos(ra[p]);
					auto vz = std::sin(ra[p]);

					for (auto f = 0; f != 2; ++f) {
						const auto f_sign = (f == 0) ? 1.0 : -1.0;
						const auto fdt = -t * d_sign * f_sign * 0.95;

						dest[k++] = T(vx * (r1 + r2 * (1.0 + vr) + fdt * vr));
						dest[k++] = T(vy * (r2 + fdt));
						dest[k++] = T(vz * (r1 + r2 * (1.0 + vr) + fdt * vr));
					}
				}
			}
			auto vx = std::cos(r_slip);
			auto vz = std::sin(r_slip);

			for (auto f = 0; f != 2; ++f) {
				const auto f_sign = (f == 0) ? 1.0 : -1.0;
				const auto fdt = -t * d_sign * f_sign * 0.5;

				dest[k++] = T(vx * (r1 + r2 * (1.0 + vr) + fdt * vr));
				dest[k++] = T(vy * (r2 + fdt));
				dest[k++] = T(vz * (r1 + r2 * (1.0 + vr) + fdt * vr));
			}
		}
	}

	assert(k == dest.size());
	return 3;
}

inline uint32_t WickerTorus::normals(std::vector<float> &dest) const
{
	using T = float;
	dest.resize(
	        2 * 2 * 2 * m_rings * (m_sections * 3 + 1) * 3
	        + 2 * 2 * 2 * m_sections * (m_rings * 2 + 1) * 3);
	auto k = 0u;
	const auto r_step = (2.0 * pi<double>()) / double(m_rings);
	const auto s_step = (2.0 * pi<double>()) / double(m_sections);
	const auto r_slip = r_step * m_r_slip_coef;
	const auto s_slip = s_step * m_s_slip_coef;
	const auto s_slop = (pi<double>()) / 4.0;

	for (auto f = 0; f != 2; ++f) {
		const auto f_sign = (f == 0) ? 1.0 : -1.0;
		for (auto r = 0u; r != m_rings; ++r) {
			auto vx = std::cos(r * r_step);
			auto vz = std::sin(r * r_step);
			const auto rslp = s_slop * f_sign;
			for (auto s = 0u; s != m_sections; ++s) {
				const double sa[3]
				        = {s * s_step + ((s % 2 == 0) ? -rslp : rslp), s * s_step, s * s_step};
				for (auto p = 0; p != 3; ++p) {
					auto vr = std::cos(sa[p]);
					auto vy = std::sin(sa[p]);
					for (auto d = 0; d != 2; ++d) {
						dest[k++] = T(f_sign * vx * vr);
						dest[k++] = T(f_sign * vy);
						dest[k++] = T(f_sign * vz * vr);
					}
				}
			}
			for (auto d = 0; d != 2; ++d) {
				dest[k++] = T(f_sign * vx);
				dest[k++] = T(f_sign * std::sin(-rslp));
				dest[k++] = T(f_sign * vz);
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		const auto d_sign = (d == 0) ? -1.0 : 1.0;
		for (auto r = 0u; r != m_rings; ++r) {
			auto vx = std::cos(r * r_step);
			auto vz = std::sin(r * r_step);
			for (auto s = 0u; s != m_sections; ++s) {
				for (auto p = 0; p != 3; ++p) {
					for (auto f = 0; f != 2; ++f) {
						dest[k++] = T(+vz * d_sign);
						dest[k++] = T(0);
						dest[k++] = T(-vx * d_sign);
					}
				}
			}
			for (auto f = 0; f != 2; ++f) {
				dest[k++] = T(+vz * d_sign);
				dest[k++] = T(0);
				dest[k++] = T(-vx * d_sign);
			}
		}
	}

	for (auto f = 0; f != 2; ++f) {
		const auto f_sign = (f == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const double sa[2] = {s_angle + s_slip * f_sign, s_angle - s_slip * f_sign};
			for (auto r = 0u; r != m_rings; ++r) {
				const auto r_angle = r * r_step;
				const double ra[2] = {r_angle + r_slip, r_angle + r_step - r_slip};
				for (auto p = 0; p != 2; ++p) {
					auto vx = std::cos(ra[p]);
					auto vz = std::sin(ra[p]);

					for (auto d = 0; d != 2; ++d) {
						auto vr = std::cos(sa[d]);
						auto vy = std::sin(sa[d]);

						dest[k++] = T(f_sign * vx * vr);
						dest[k++] = T(f_sign * vy);
						dest[k++] = T(f_sign * vz * vr);
					}
				}
			}
			auto vx = std::cos(r_slip);
			auto vz = std::sin(r_slip);

			for (auto d = 0; d != 2; ++d) {
				auto vr = std::cos(sa[d]);
				auto vy = std::sin(sa[d]);

				dest[k++] = T(f_sign * vx * vr);
				dest[k++] = T(f_sign * vy);
				dest[k++] = T(f_sign * vz * vr);
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		const auto d_sign = (d == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const auto sa = s_angle + s_slip * d_sign;
			const auto vr = std::sin(sa);
			const auto vy = std::cos(sa);
			for (auto r = 0u; r != m_rings; ++r) {
				const auto r_angle = r * r_step;
				const double ra[2] = {r_angle + r_slip, r_angle + r_step - r_slip};
				for (auto p = 0; p != 2; ++p) {
					auto vx = std::cos(ra[p]);
					auto vz = std::sin(ra[p]);

					for (auto f = 0; f != 2; ++f) {
						dest[k++] = T(d_sign * -vx * vr);
						dest[k++] = T(d_sign * vy);
						dest[k++] = T(d_sign * -vz * vr);
					}
				}
			}
			auto vx = std::cos(r_slip);
			auto vz = std::sin(r_slip);

			for (auto f = 0; f != 2; ++f) {
				dest[k++] = T(d_sign * -vx * vr);
				dest[k++] = T(d_sign * vy);
				dest[k++] = T(d_sign * -vz * vr);
			}
		}
	}

	assert(k == dest.size());
	return 3;
}

inline uint32_t WickerTorus::tangents(std::vector<float> &dest) const
{
	using T = float;
	dest.resize(
	        2 * 2 * 2 * m_rings * (m_sections * 3 + 1) * 3
	        + 2 * 2 * 2 * m_sections * (m_rings * 2 + 1) * 3);
	auto k = 0u;
	const auto r_step = (2.0 * pi<double>()) / double(m_rings);
	const auto s_step = (2.0 * pi<double>()) / double(m_sections);
	const auto r_slip = r_step * m_r_slip_coef;
	const auto s_slop = (pi<double>()) / 4.0;

	for (auto f = 0; f != 2; ++f) {
		const auto f_sign = (f == 0) ? 1.0 : -1.0;
		for (auto r = 0u; r != m_rings; ++r) {
			auto vx = std::cos(r * r_step);
			auto vz = std::sin(r * r_step);
			for (auto s = 0u; s != m_sections; ++s) {
				for (auto p = 0; p != 3; ++p) {
					for (auto d = 0; d != 2; ++d) {
						dest[k++] = T(+vz * f_sign);
						dest[k++] = T(0);
						dest[k++] = T(-vx * f_sign);
					}
				}
			}
			for (auto d = 0; d != 2; ++d) {
				dest[k++] = T(+vz * f_sign);
				dest[k++] = T(0);
				dest[k++] = T(-vx * f_sign);
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		const auto d_sign = (d == 0) ? 1.0 : -1.0;
		for (auto r = 0u; r != m_rings; ++r) {
			auto r_sign = r % 2 == 0 ? 1.0 : -1.0;
			auto vx = std::cos(r * r_step);
			auto vz = std::sin(r * r_step);
			const auto rslp = s_slop * r_sign;
			for (auto s = 0u; s != m_sections; ++s) {
				const double sa[3]
				        = {s * s_step + ((s % 2 == 0) ? rslp : -rslp), s * s_step, s * s_step};
				for (auto p = 0; p != 3; ++p) {
					auto vr = std::cos(sa[p]);
					auto vy = std::sin(sa[p]);
					for (auto f = 0; f != 2; ++f) {
						dest[k++] = T(d_sign * vx * vr);
						dest[k++] = T(d_sign * vy);
						dest[k++] = T(d_sign * vz * vr);
					}
				}
			}
			for (auto f = 0; f != 2; ++f) {
				dest[k++] = T(d_sign * vx);
				dest[k++] = T(d_sign * std::sin(rslp));
				dest[k++] = T(d_sign * vz);
			}
		}
	}

	for (auto f = 0; f != 2; ++f) {
		const auto f_sign = (f == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			for (auto r = 0u; r != m_rings; ++r) {
				const auto r_angle = r * r_step;
				const double ra[2] = {r_angle + r_slip, r_angle + r_step - r_slip};
				for (auto p = 0; p != 2; ++p) {
					auto vx = std::cos(ra[p]);
					auto vz = std::sin(ra[p]);

					for (auto d = 0; d != 2; ++d) {
						dest[k++] = T(+vz * f_sign);
						dest[k++] = T(0);
						dest[k++] = T(-vx * f_sign);
					}
				}
			}
			auto vx = std::cos(r_slip);
			auto vz = std::sin(r_slip);

			for (auto d = 0; d != 2; ++d) {
				dest[k++] = T(+vz * f_sign);
				dest[k++] = T(0);
				dest[k++] = T(-vx * f_sign);
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		const auto d_sign = (d == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			for (auto r = 0u; r != m_rings; ++r) {
				const auto r_angle = r * r_step;
				const double ra[2] = {r_angle + r_slip, r_angle + r_step - r_slip};
				for (auto p = 0; p != 2; ++p) {
					auto vx = std::cos(ra[p]);
					auto vz = std::sin(ra[p]);

					for (auto f = 0; f != 2; ++f) {
						dest[k++] = T(+vz * d_sign);
						dest[k++] = T(0);
						dest[k++] = T(-vx * d_sign);
					}
				}
			}
			auto vx = std::cos(r_slip);
			auto vz = std::sin(r_slip);

			for (auto f = 0; f != 2; ++f) {
				dest[k++] = T(+vz * d_sign);
				dest[k++] = T(0);
				dest[k++] = T(-vx * d_sign);
			}
		}
	}

	assert(k == dest.size());
	return 3;
}

inline uint32_t WickerTorus::bitangents(std::vector<float> &dest) const
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

inline uint32_t WickerTorus::texCoordinates(std::vector<float> &dest) const
{
	using T = float;
	dest.resize(
	        2 * 2 * 2 * m_rings * (m_sections * 3 + 1) * 2
	        + 2 * 2 * 2 * m_sections * (m_rings * 2 + 1) * 2);
	auto k = 0u;
	auto t = m_thickness / m_radius_in;
	auto r_step = 0.5 / double(m_rings);
	auto s_step = 1.0 / double(m_sections);
	auto r_slip = r_step * m_r_slip_coef;
	auto s_slip = s_step * t;

	for (auto f = 0; f != 2; ++f) {
		for (auto r = 0u; r != m_rings; ++r) {
			auto rv = 2 * r * r_step;
			for (auto s = 0u; s != m_sections; ++s) {
				const double sa[3] = {s * s_step, (s + t) * s_step, (s + 1.0 - 2 * t) * s_step};
				for (auto p = 0; p != 3; ++p) {
					auto v = sa[p];
					for (auto d = 0; d != 2; ++d) {
						auto u = rv + ((d + f) % 2) * r_step;
						dest[k++] = T(u);
						dest[k++] = T(v);
					}
				}
			}
			for (auto d = 0; d != 2; ++d) {
				auto u = rv + ((d + f) % 2) * r_step;
				dest[k++] = T(u);
				dest[k++] = T(1.0);
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		for (auto r = 0u; r != m_rings; ++r) {
			auto rv = 2 * r * r_step;
			for (auto s = 0u; s != m_sections; ++s) {
				const double sa[3] = {s * s_step, (s + t) * s_step, (s + 1.0 - 2 * t) * s_step};
				for (auto p = 0; p != 3; ++p) {
					auto v = sa[p];
					for (auto f = 0; f != 2; ++f) {
						auto u = rv + ((d + f) % 2) * r_step;
						dest[k++] = T(u);
						dest[k++] = T(v);
					}
				}
			}
			for (auto f = 0; f != 2; ++f) {
				auto u = rv + ((d + f) % 2) * r_step;
				dest[k++] = T(u);
				dest[k++] = T(1.0);
			}
		}
	}

	s_slip = s_step * m_s_slip_coef;

	for (auto f = 0; f != 2; ++f) {
		const auto f_sign = (f == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const double sa[2] = {s_angle + s_slip * f_sign, s_angle - s_slip * f_sign};
			for (auto r = 0u; r != m_rings; ++r) {
				const auto r_angle = 2 * r * r_step;
				const double ra[2] = {r_angle + r_slip, r_angle + r_step - r_slip};
				for (auto p = 0; p != 2; ++p) {
					auto u = ra[p];
					for (auto d = 0; d != 2; ++d) {
						auto v = sa[d];
						dest[k++] = T(u);
						dest[k++] = T(v);
					}
				}
			}

			auto u = 1.0 + r_slip;
			for (auto d = 0; d != 2; ++d) {
				auto v = sa[d];
				dest[k++] = T(u);
				dest[k++] = T(v);
			}
		}
	}

	for (auto d = 0; d != 2; ++d) {
		const auto d_sign = (d == 0) ? 1.0 : -1.0;
		for (auto s = 0u; s != m_sections; ++s) {
			const auto s_angle = s_step * 0.5 + s * s_step;
			const auto v = s_angle + s_slip * d_sign;
			for (auto r = 0u; r != m_rings; ++r) {
				const auto r_angle = 2 * r * r_step;
				const double ra[2] = {r_angle + r_slip, r_angle + r_step - r_slip};
				for (auto p = 0; p != 2; ++p) {
					auto u = ra[p];
					for (auto f = 0; f != 2; ++f) {
						dest[k++] = T(u);
						dest[k++] = T(v);
					}
				}
			}

			auto u = 1.0 + r_slip;
			for (auto f = 0; f != 2; ++f) {
				dest[k++] = T(u);
				dest[k++] = T(v);
			}
		}
	}

	assert(k == dest.size());
	return 2;
}

inline std::vector<spu::SpuCommand> WickerTorus::instructions(WickerTorus::DefaultTag /*unused*/) const
{
	std::vector<spu::SpuCommand> instructions;
	auto strip = 2 * (m_sections * 3 + 1);
	auto offs = 0;

	auto phase = 0;
	for (auto f = 0; f != 4; ++f) {
		for (auto r = 0u; r != m_rings; ++r) {
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

	strip = 2 * (m_rings * 2 + 1);

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

inline WickerTorus::IndexArray WickerTorus::indices(WickerTorus::EdgesTag /*unused*/) const
{
	const auto leap_1 = 2 * m_rings * (m_sections * 3 + 1);
	const auto leap_2 = 2 * m_sections * (m_rings * 2 + 1);
	IndexArray indices(2 * leap_1 + 2 * leap_2);
	auto k = 0u;

	for (auto f = 0; f != 2; ++f) {
		for (auto d = 0; d != 2; ++d) {
			auto i = f * leap_1 + d;
			for (auto r = 0u; r != m_rings; ++r) {
				for (auto s = 0u; s != m_sections; ++s) {
					for (auto p = 0; p != 3; ++p) {
						indices[k++] = i += 2;
					}
				}
				indices[k++] = i += 2;
			}
		}
	}

	for (auto f = 0; f != 2; ++f) {
		for (auto d = 0; d != 2; ++d) {
			auto i = 4 * leap_1 + f * leap_2 + d;
			for (auto s = 0u; s != m_sections; ++s) {
				for (auto r = 0u; r != m_rings; ++r) {
					for (auto p = 0; p != 2; ++p) {
						indices[k++] = i += 2;
					}
				}
				indices[k++] = i += 2;
			}
		}
	}

	assert(k == indices.size());

	return indices;
}

inline std::vector<spu::SpuCommand> WickerTorus::instructions(WickerTorus::EdgesTag /*unused*/) const
{
	std::vector<spu::SpuCommand> instructions;

	auto phase = 0;
	auto edge = (m_sections * 3 + 1);
	auto offs = 0;

	for (auto f = 0; f != 2; ++f) {
		for (auto d = 0; d != 2; ++d) {
			for (auto r = 0u; r != m_rings; ++r) {
				spu::SpuCommand com;
				com.target = GL_ELEMENT_ARRAY_BUFFER;
				com.mode = GL_LINE_LOOP;
				com.first = uint32_t(offs);
				com.count = uint32_t(edge - 1);
				com.flags = phase;

				instructions.push_back(com);
				offs += edge;
			}
		}
	}

	edge = (m_rings * 2 + 1);

	for (auto f = 0; f != 2; ++f) {
		for (auto d = 0; d != 2; ++d) {
			for (auto s = 0u; s != m_sections; ++s) {
				spu::SpuCommand com;
				com.target = GL_ELEMENT_ARRAY_BUFFER;
				com.mode = GL_LINE_LOOP;
				com.first = uint32_t(offs);
				com.count = uint32_t(edge - 1);
				com.flags = phase;

				instructions.push_back(com);
				offs += edge;
			}
		}
	}

	offs = 0;

	for (auto f = 0; f != 2; ++f) {
		for (auto r = 0u; r != m_rings; ++r) {
			spu::SpuCommand com;
			com.target = GL_ARRAY_BUFFER;
			com.mode = GL_LINES;
			com.first = uint32_t(offs);
			com.count = uint32_t(2 * edge);
			com.flags = phase;

			instructions.push_back(com);
			offs += 2 * edge;
		}
	}

	return instructions;
}

}  // namespace spu::oglplus::shapes
