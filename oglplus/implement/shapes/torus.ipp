
namespace spu::oglplus::shapes {

inline Torus::IndexArray Torus::indices(Torus::DefaultTag /*unused*/) const
{
	const auto n = m_rings * (2 * (m_sections + 1) + 1);
	assert((1 << (sizeof(uint16_t) * 8)) - 1 >= n);
	IndexArray indices(n);
	auto k = 0u;
	auto offs = 0u;
	for (auto r = 0u; r != (m_rings); ++r) {
		for (auto s = 0u; s != (m_sections + 1); ++s) {
			indices[k++] = offs + s;
			indices[k++] = offs + s + (m_sections + 1);
		}
		offs += m_sections + 1;

		indices[k++] = restartIndex();
	}
	assert(k == indices.size());
	return indices;
}

inline Torus::IndexArray Torus::indices(Torus::QuadsTag /*unused*/) const
{
	const auto n = m_rings * (4 * m_sections + 1);
	assert((1 << (sizeof(uint16_t) * 8)) - 1 >= n);
	IndexArray indices(n);
	auto k = 0u;
	auto offs = 0u;
	for (auto r = 0u; r != (m_rings); ++r) {
		for (auto s = 0u; s != (m_sections); ++s) {
			indices[k++] = offs + s;
			indices[k++] = offs + s + (m_sections + 1);
			indices[k++] = offs + s + 1;
			indices[k++] = offs + s + (m_sections + 1) + 1;
		}

		indices[k++] = restartIndex();
		offs += m_sections + 1;
	}
	assert(k == indices.size());
	return indices;
}

inline Torus::IndexArray Torus::indices(WithAdjacencyTag /*unused*/) const
{
	const auto m = m_rings * (m_sections + 1);
	const auto n = m_rings * (4 * (m_sections + 1) + 1);
	assert((1 << (sizeof(uint16_t) * 8)) - 1 >= n);
	IndexArray indices(n);
	auto k = 0u;
	auto offs = 0u;

	for (auto r = 0u; r != (m_rings); ++r) {
		indices[k++] = offs;
		indices[k++] = offs + (2 * m_sections);
		indices[k++] = offs + (m_sections + 1);
		for (auto s = 0u; s != m_sections; ++s) {
			indices[k++] = (offs + m - (m_sections + 1)) % m + s + 1;
			indices[k++] = offs + s + 1;
			indices[k++] = (offs + 2 * (m_sections + 1)) % m + s;
			indices[k++] = offs + (m_sections + 1) + s + 1;
		}
		indices[k++] = offs + 1;

		indices[k++] = restartIndex();
		offs += m_sections + 1;
	}
	assert(k == indices.size());
	return indices;
}

inline std::vector<spu::SpuCommand> Torus::instructions(Torus::DefaultTag /*unused*/) const
{
	const auto n = m_rings * (2 * (m_sections + 1) + 1);
	spu::SpuCommand com;
	com.target = GL_ELEMENT_ARRAY_BUFFER;
	com.mode = GL_TRIANGLE_STRIP;
	com.first = 0;
	com.count = n;
	com.flags = 0;

	return {com};
}

inline std::vector<spu::SpuCommand> Torus::instructions(Torus::QuadsTag /*unused*/) const
{
	const auto n = m_rings * (4 * m_sections + 1);
	spu::SpuCommand com;
	com.target = GL_ELEMENT_ARRAY_BUFFER;
	com.mode = GL_LINES_ADJACENCY;
	com.first = 0;
	com.count = n;
	com.flags = 0;

	return {com};
}

inline std::vector<spu::SpuCommand> Torus::instructions(Torus::WithAdjacencyTag /*unused*/) const
{
	const auto n = m_rings * (4 * (m_sections + 1) + 1);
	spu::SpuCommand com;
	com.target = GL_ELEMENT_ARRAY_BUFFER;
	com.mode = GL_TRIANGLE_STRIP_ADJACENCY;
	com.first = 0;
	com.count = n;
	com.flags = 0;

	return {com};
}

}  // namespace spu::oglplus::shapes
