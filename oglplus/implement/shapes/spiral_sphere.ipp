
namespace spu::oglplus::shapes {

inline uint32_t SpiralSphere::vertex_count() const
{
	return (m_bands * 2) * (m_divisions + 1) * (m_segments + 1) + (m_bands * 2) * (m_segments + 1);
}

inline void SpiralSphere::make_vectors(std::vector<float> &dest, uint32_t &k, double sign, double radius) const
{
	auto b_leap = (pi<double>()) / double(m_bands);
	auto b_step = b_leap / double(m_divisions);
	auto s_step = (pi<double>()) / double(m_segments);

	auto m = sign * radius;

	for (auto b = 0u; b != m_bands; ++b) {
		for (auto d = 0u; d != (m_divisions + 1); ++d) {
			auto b_offs = 0.0;
			for (auto s = 0u; s != (m_segments + 1); ++s) {
				auto b_angle = 2 * b * b_leap + d * b_step + b_offs;
				auto cb = cos(b_angle);
				auto sb = sin(b_angle);

				auto s_angle = s * s_step;
				auto cs = cos(s_angle);
				auto ss = sin(s_angle);

				dest[k++] = float(m * ss * cb);
				dest[k++] = float(m * cs);
				dest[k++] = float(m * ss * -sb);
				b_offs += ss * s_step;
			}
		}
	}
}

inline void SpiralSphere::make_tangents(std::vector<float> &dest, uint32_t &k, double sign) const
{
	auto b_leap = (pi<double>()) / double(m_bands);
	auto b_step = b_leap / double(m_divisions);
	auto s_step = (pi<double>()) / double(m_segments);

	auto m = sign;

	for (auto b = 0u; b != m_bands; ++b) {
		for (auto d = 0u; d != (m_divisions + 1); ++d) {
			auto b_offs = 0.0;
			for (auto s = 0u; s != (m_segments + 1); ++s) {
				auto b_angle = 2 * b * b_leap + d * b_step + b_offs;
				auto cb = cos(b_angle);
				auto sb = sin(b_angle);

				auto s_angle = s * s_step;
				auto ss = sin(s_angle);

				dest[k++] = float(m * -sb);
				dest[k++] = float(0);
				dest[k++] = float(m * -cb);
				b_offs += ss * s_step;
			}
		}
	}
}

inline void SpiralSphere::make_bitangents(std::vector<float> &dest, uint32_t &k, double sign) const
{
	auto b_leap = (pi<double>()) / double(m_bands);
	auto b_step = b_leap / double(m_divisions);
	auto s_step = (pi<double>()) / double(m_segments);

	auto m = sign;

	for (auto b = 0u; b != m_bands; ++b) {
		for (auto d = 0u; d != (m_divisions + 1); ++d) {
			auto b_offs = 0.0;
			for (auto s = 0u; s != (m_segments + 1); ++s) {
				auto b_angle = 2 * b * b_leap + d * b_step + b_offs;
				auto cb = cos(b_angle);
				auto sb = sin(b_angle);

				auto s_angle = s * s_step;
				auto cs = cos(s_angle);
				auto ss = sin(s_angle);

				auto tx = m * -sb;
				auto ty = 0.0;
				auto tz = m * -cb;

				auto nx = m * ss * cb;
				auto ny = m * cs;
				auto nz = m * ss * -sb;

				dest[k++] = float(ny * tz - nz * ty);
				dest[k++] = float(nz * tx - nx * tz);
				dest[k++] = float(nx * ty - ny * tx);

				b_offs += ss * s_step;
			}
		}
	}
}

inline void SpiralSphere::make_uv_coords(std::vector<float> &dest, uint32_t &k) const
{
	auto b_leap = 0.5 / double(m_bands);
	auto b_step = b_leap / double(m_divisions);
	auto s_step = 1.0 / double(m_segments);

	auto u = 0.0;
	for (auto b = 0u; b != m_bands; ++b) {
		for (auto d = 0u; d != (m_divisions + 1); ++d) {
			auto v = 1.0;
			for (auto s = 0u; s != (m_segments + 1); ++s) {
				dest[k++] = float(u);
				dest[k++] = float(v);
				v -= s_step;
			}
			u += b_step;
		}
		u += b_leap;
	}
}

inline void SpiralSphere::make_side_verts(std::vector<float> &dest, uint32_t &k) const
{
	auto b_leap = (pi<double>()) / double(m_bands);
	auto b_slip = b_leap * m_thickness * 0.5;
	auto s_step = (pi<double>()) / double(m_segments);

	auto m = m_radius + m_thickness * 0.5;
	auto g = -1.0;

	for (auto b = 0u; b != m_bands * 2; ++b) {
		auto b_offs = 0.0;
		for (auto s = 0u; s != (m_segments + 1); ++s) {
			auto b_angle = b * b_leap + b_offs + g * b_slip;
			auto cb = cos(b_angle);
			auto sb = sin(b_angle);

			auto s_angle = s * s_step;
			auto cs = cos(s_angle);
			auto ss = sin(s_angle);

			dest[k++] = float(m * ss * cb);
			dest[k++] = float(m * cs);
			dest[k++] = float(m * ss * -sb);
			b_offs += ss * s_step;
		}
		g *= -1.0;
	}
}

inline void SpiralSphere::make_side_norms(std::vector<float> &dest, uint32_t &k) const
{
	auto b_leap = (pi<double>()) / double(m_bands);
	auto s_step = (pi<double>()) / double(m_segments);

	auto m = 1.0;
	for (auto b = 0u; b != m_bands * 2; ++b) {
		auto b_offs = 0.0;
		for (auto s = 0u; s != (m_segments + 1); ++s) {
			auto b_angle = b * b_leap + b_offs;
			auto cb = cos(b_angle);
			auto sb = sin(b_angle);

			auto s_angle = s * s_step;
			auto ss = sin(s_angle);

			dest[k++] = float(m * -sb);
			dest[k++] = float(0);
			dest[k++] = float(m * cb);
			b_offs += ss * s_step;
		}
		m *= -1.0;
	}
}

inline void SpiralSphere::make_side_tgts(std::vector<float> &dest, uint32_t &k) const
{
	auto b_leap = (pi<double>()) / double(m_bands);
	auto s_step = (pi<double>()) / double(m_segments);

	auto m = -1.0;
	for (auto b = 0u; b != m_bands * 2; ++b) {
		auto b_offs = 0.0;
		for (auto s = 0u; s != (m_segments + 1); ++s) {
			auto b_angle = b * b_leap + b_offs;
			auto cb = cos(b_angle);
			auto sb = sin(b_angle);

			auto s_angle = s * s_step;
			auto cs = cos(s_angle);
			auto ss = sin(s_angle);

			dest[k++] = float(m * ss * -cb);
			dest[k++] = float(m * cs);
			dest[k++] = float(m * ss * -sb);
			b_offs += ss * s_step;
		}
		m *= -1.0;
	}
}

inline void SpiralSphere::make_side_btgs(std::vector<float> &dest, uint32_t &k) const
{
	auto b_leap = (pi<double>()) / double(m_bands);
	auto s_step = (pi<double>()) / double(m_segments);

	auto m = 1.0;
	for (auto b = 0u; b != m_bands * 2; ++b) {
		auto b_offs = 0.0;
		for (auto s = 0u; s != (m_segments + 1); ++s) {
			auto b_angle = b * b_leap + b_offs;
			auto cb = cos(b_angle);
			auto sb = sin(b_angle);

			auto s_angle = s * s_step;
			auto cs = cos(s_angle);
			auto ss = sin(s_angle);

			auto tx = m * ss * -cb;
			auto ty = m * cs;
			auto tz = m * ss * -sb;

			auto nx = m * sb;
			auto ny = 0.0;
			auto nz = m * -cb;

			dest[k++] = float(ny * tz - nz * ty);
			dest[k++] = float(nz * tx - nx * tz);
			dest[k++] = float(nx * ty - ny * tx);

			b_offs += ss * s_step;
		}
		m *= -1.0;
	}
}

inline void SpiralSphere::make_side_uvs(std::vector<float> &dest, uint32_t &k) const
{
	auto b_leap = 0.5 / double(m_bands);
	auto b_slip = b_leap * m_thickness * 0.5;
	auto s_step = 1.0 / double(m_segments);

	auto g = -1.0;

	for (auto b = 0u; b != m_bands * 2; ++b) {
		auto b_offs = 0.0;
		auto v = 1.0;
		for (auto s = 0u; s != (m_segments + 1); ++s) {
			dest[k++] = float(b * b_leap + b_offs + g * b_slip);
			dest[k++] = float(v);
			v -= s_step;
		}
		g *= -1.0;
	}
}

inline uint32_t SpiralSphere::positions(std::vector<float> &dest) const
{
	dest.resize(vertex_count() * 3);
	auto k = 0u;
	make_vectors(dest, k, 1.0, m_radius);
	make_vectors(dest, k, 1.0, m_radius + m_thickness);
	make_side_verts(dest, k);
	assert(k == dest.size());
	return 3;
}

inline uint32_t SpiralSphere::normals(std::vector<float> &dest) const
{
	dest.resize(vertex_count() * 3);
	auto k = 0u;
	make_vectors(dest, k, -1.0, 1.0);
	make_vectors(dest, k, 1.0, 1.0);
	make_side_norms(dest, k);
	assert(k == dest.size());
	return 3;
}

inline uint32_t SpiralSphere::tangents(std::vector<float> &dest) const
{
	dest.resize(vertex_count() * 3);
	auto k = 0u;
	make_tangents(dest, k, -1.0);
	make_tangents(dest, k, 1.0);
	make_side_tgts(dest, k);
	assert(k == dest.size());
	return 3;
}

inline uint32_t SpiralSphere::bitangents(std::vector<float> &dest) const
{
	dest.resize(vertex_count() * 3);
	auto k = 0u;
	make_bitangents(dest, k, -1.0);
	make_bitangents(dest, k, 1.0);
	make_side_btgs(dest, k);
	assert(k == dest.size());
	return 3;
}

inline uint32_t SpiralSphere::texCoordinates(std::vector<float> &dest) const
{
	dest.resize(vertex_count() * 2);
	auto k = 0u;
	make_uv_coords(dest, k);
	make_uv_coords(dest, k);
	make_side_uvs(dest, k);
	assert(k == dest.size());
	return 2;
}

inline SpiralSphere::IndexArray SpiralSphere::indices(SpiralSphere::DefaultTag /*unused*/) const
{
	assert((1 << (sizeof(uint16_t) * 8)) - 1 >= vertex_count());
	const auto m = (m_bands * 2) * (m_divisions * 2) * (m_segments + 1) + (m_bands * 8) * (m_segments + 1);
	IndexArray indices(m);
	auto k = 0u;
	auto eoffs = 0u;
	auto offs = 0u;
	const auto edge = m_segments + 1;
	const auto band = edge * (m_divisions + 1);
	const auto surface = m_bands * band;

	for (auto n = 0u; n != 2; ++n) {
		auto edge1 = n != 0u ? edge : 0;
		auto edge2 = n != 0u ? 0 : edge;
		for (auto b = 0u; b != m_bands; ++b) {
			for (auto d = 0u; d != m_divisions; ++d) {
				for (auto s = 0u; s != edge; ++s) {
					indices[k++] = offs + s + edge1;
					indices[k++] = offs + s + edge2;
				}
				offs += edge;
			}
			offs += edge;
		}
	}

	offs = 0;
	eoffs = 2 * surface;

	for (auto b = 0u; b != m_bands; ++b) {
		for (auto s = 0u; s != edge; ++s) {
			indices[k++] = eoffs + s;
			indices[k++] = offs + s;
		}
		offs += band;
		eoffs += edge * 2;
	}

	offs = m_divisions * edge;
	eoffs = 2 * surface + edge;

	for (auto b = 0u; b != m_bands; ++b) {
		for (auto s = 0u; s != edge; ++s) {
			indices[k++] = offs + s;
			indices[k++] = eoffs + s;
		}
		offs += band;
		eoffs += edge * 2;
	}

	offs = surface;
	eoffs = 2 * surface;

	for (auto b = 0u; b != m_bands; ++b) {
		for (auto s = 0u; s != edge; ++s) {
			indices[k++] = offs + s;
			indices[k++] = eoffs + s;
		}
		offs += band;
		eoffs += edge * 2;
	}

	offs = surface + m_divisions * edge;
	eoffs = 2 * surface + edge;

	for (auto b = 0u; b != m_bands; ++b) {
		for (auto s = 0u; s != edge; ++s) {
			indices[k++] = eoffs + s;
			indices[k++] = offs + s;
		}
		offs += band;
		eoffs += edge * 2;
	}

	assert(k == indices.size());
	return indices;
}

inline std::vector<spu::SpuCommand> SpiralSphere::instructions(SpiralSphere::DefaultTag /*unused*/) const
{
	std::vector<spu::SpuCommand> instructions;
	const auto edge = m_segments + 1;

	auto method = GL_ELEMENT_ARRAY_BUFFER;
	auto primitive_type = GL_TRIANGLE_STRIP;

	auto offs = 0;

	auto phase = 0;
	for (auto n = 0; n != 2; ++n) {
		for (auto b = 0u; b != m_bands; ++b) {
			for (auto d = 0u; d != m_divisions; ++d) {
				spu::SpuCommand com;
				com.target = method;
				com.mode = primitive_type;
				com.first = offs;
				com.count = uint32_t(edge * 2);
				com.flags = phase;

				instructions.push_back(com);
				offs += edge * 2;
			}
		}
		++phase;
	}
	for (auto n = 0; n != 4; ++n) {
		for (auto b = 0u; b != m_bands; ++b) {
			spu::SpuCommand com;
			com.target = method;
			com.mode = primitive_type;
			com.first = offs;
			com.count = uint32_t(edge * 2);
			com.flags = phase;

			instructions.push_back(com);
			offs += edge * 2;
		}
	}
	return instructions;
}

}  // namespace spu::oglplus::shapes
