
namespace spu::oglplus::shapes {

inline uint32_t SimpleSubdivSphere::midpoint(uint32_t ia, uint32_t ib)
{
	auto ea = ia < ib ? ia : ib;
	auto eb = ia > ib ? ia : ib;

	Edge e(ea, eb);

	auto p = m_midpoints.find(e);
	if (p == m_midpoints.end()) {
		Vec3f va = {m_positions[ea * 3 + 0], m_positions[ea * 3 + 1], m_positions[ea * 3 + 2]};
		Vec3f vb = {m_positions[eb * 3 + 0], m_positions[eb * 3 + 1], m_positions[eb * 3 + 2]};

		auto mp = normalize((va + vb) * 0.5);
		auto result = m_positions.size();

		m_positions.insert(m_positions.end(), mp.data(), mp.data() + 3);

		assert(result % 3 == 0);
		result /= 3;

		m_midpoints[e] = result;

		return result;
	}

	return p->second;
}

inline void SimpleSubdivSphere::subdivide(uint32_t ia, uint32_t ib, uint32_t ic, uint32_t levels)
{
	auto iab = midpoint(ia, ib);
	auto ibc = midpoint(ib, ic);
	auto ica = midpoint(ic, ia);

	make_face(iab, ibc, ica, levels);
	make_face(ica, ia, iab, levels);
	make_face(iab, ib, ibc, levels);
	make_face(ibc, ic, ica, levels);
}

inline void SimpleSubdivSphere::make_face(uint32_t ia, uint32_t ib, uint32_t ic, uint32_t levels)
{
	if (levels != 0u) {
		subdivide(ia, ib, ic, levels - 1);
	}
	else {
		m_indices.push_back(ia);
		m_indices.push_back(ib);
		m_indices.push_back(ic);
	}
}

inline void SimpleSubdivSphere::init_icosah()
{
	static const double init_pos[12 * 3] = {
	        0.000,  1.000,  0.000,  0.894,  0.447,  0.000,  0.276, 0.447,  0.851,  -0.724, 0.447,  0.526,
	        -0.724, 0.447,  -0.526, 0.276,  0.447,  -0.851, 0.724, -0.447, 0.526,  -0.276, -0.447, 0.851,
	        -0.894, -0.447, 0.000,  -0.276, -0.447, -0.851, 0.724, -0.447, -0.526, 0.000,  -1.000, 0.000};

	m_positions.insert(m_positions.end(), init_pos, init_pos + 12 * 3);

	make_face(2, 1, 0, m_subdivs);
	make_face(3, 2, 0, m_subdivs);
	make_face(4, 3, 0, m_subdivs);
	make_face(5, 4, 0, m_subdivs);
	make_face(1, 5, 0, m_subdivs);
	make_face(11, 6, 7, m_subdivs);
	make_face(11, 7, 8, m_subdivs);
	make_face(11, 8, 9, m_subdivs);
	make_face(11, 9, 10, m_subdivs);
	make_face(11, 10, 6, m_subdivs);
	make_face(1, 2, 6, m_subdivs);
	make_face(2, 3, 7, m_subdivs);
	make_face(3, 4, 8, m_subdivs);
	make_face(4, 5, 9, m_subdivs);
	make_face(5, 1, 10, m_subdivs);
	make_face(2, 7, 6, m_subdivs);
	make_face(3, 8, 7, m_subdivs);
	make_face(4, 9, 8, m_subdivs);
	make_face(5, 10, 9, m_subdivs);
	make_face(1, 6, 10, m_subdivs);
}

inline void SimpleSubdivSphere::init_tetrah()
{
	static const double init_pos[4 * 3]
	        = {0.0,
	           1.0,
	           0.0,
	           -1.0 * std::sqrt(2.0) / 3.0,
	           -1.0 / 3.0,
	           -std::sqrt(2.0 / 3.0),
	           -1.0 * std::sqrt(2.0) / 3.0,
	           -1.0 / 3.0,
	           std::sqrt(2.0 / 3.0),
	           +2.0 * std::sqrt(2.0) / 3.0,
	           -1.0 / 3.0,
	           0.0};

	m_positions.insert(m_positions.end(), init_pos, init_pos + 4 * 3);

	make_face(3, 2, 1, m_subdivs);
	make_face(3, 0, 2, m_subdivs);
	make_face(1, 0, 3, m_subdivs);
	make_face(2, 0, 1, m_subdivs);
}

inline void SimpleSubdivSphere::init_octoh()
{
	const auto px = 0;
	const auto nx = 1;
	const auto py = 2;
	const auto ny = 3;
	const auto pz = 4;
	const auto nz = 5;

	m_positions.resize(6 * 3, 0);

	m_positions[px * 3 + 0] = 1;
	m_positions[nx * 3 + 0] = -1;
	m_positions[py * 3 + 1] = 1;
	m_positions[ny * 3 + 1] = -1;
	m_positions[pz * 3 + 2] = 1;
	m_positions[nz * 3 + 2] = -1;

	make_face(px, py, pz, m_subdivs);
	make_face(pz, py, nx, m_subdivs);
	make_face(nx, ny, pz, m_subdivs);
	make_face(pz, ny, px, m_subdivs);
	make_face(nz, py, px, m_subdivs);
	make_face(nx, py, nz, m_subdivs);
	make_face(nz, ny, nx, m_subdivs);
	make_face(px, ny, nz, m_subdivs);
}

inline SimpleSubdivSphere::SimpleSubdivSphere(uint32_t subdivs, InitialShape init_shape) : m_subdivs(subdivs)
{
	if (init_shape == InitialShape::Icosahedron) {
		init_icosah();
	}
	else if (init_shape == InitialShape::Octohedron) {
		init_octoh();
	}
	else if (init_shape == InitialShape::Tetrahedron) {
		init_tetrah();
	}
	else {
		assert(!"Invalid initial shape!");
	}
}

inline std::vector<spu::SpuCommand> SimpleSubdivSphere::instructions(uint32_t mode) const
{
	spu::SpuCommand com;
	com.target = GL_ELEMENT_ARRAY_BUFFER;
	com.mode = mode;
	com.first = 0;
	com.count = m_indices.size();
	com.flags = 0;

	return {com};
}

}  // namespace spu::oglplus::shapes
