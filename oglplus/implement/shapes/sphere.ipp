
namespace spu::oglplus::shapes {

inline Sphere::IndexArray Sphere::indices(Sphere::DefaultTag /*unused*/) const
{
	assert((1 << (sizeof(uint16_t) * 8)) - 1 >= ((m_rings + 2) * (m_sections + 1)));

	const auto n = (m_rings + 1) * (2 * (m_sections + 1) + 1);
	IndexArray indices(n);
	auto k = 0u;
	auto offs = 0u;
	for (auto r = 0u; r != (m_rings + 1); ++r) {
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

inline std::vector<spu::SpuCommand> Sphere::instructions(Sphere::DefaultTag /*unused*/) const
{
	const auto n = (m_rings + 1) * (2 * (m_sections + 1) + 1);

	spu::SpuCommand com;
	com.target = GL_ELEMENT_ARRAY_BUFFER;
	com.mode = GL_TRIANGLE_STRIP;
	com.first = 0;
	com.count = n;
	com.flags = 0;

	return {com};
}

}  // namespace spu::oglplus::shapes
