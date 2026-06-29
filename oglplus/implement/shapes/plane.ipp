
namespace spu::oglplus::shapes {

inline Plane::IndexArray Plane::indices(Plane::DefaultTag /*unused*/) const
{
	auto k = 0u;
	auto offs = 0u;
	auto leap = m_udiv + 1;

	auto pri = m_vdiv * (2 * leap + 1);
	IndexArray indices(pri);

	for (auto j = 0u; j != m_vdiv; ++j) {
		for (auto i = 0u; i != leap; ++i) {
			indices[k++] = offs + i;
			indices[k++] = offs + i + leap;
		}
		offs += leap;

		indices[k++] = restartIndex();
	}
	assert(k == indices.size());
	return indices;
}

inline std::vector<spu::SpuCommand> Plane::instructions(Plane::DefaultTag /*unused*/) const
{
	auto pri = m_vdiv * (2 * (m_udiv + 1) + 1);
	spu::SpuCommand com;
	com.target = GL_ELEMENT_ARRAY_BUFFER;
	com.mode = GL_TRIANGLE_STRIP;
	com.first = 0;
	com.count = pri;
	com.flags = 0;

	return {com};
}

inline Plane::IndexArray Plane::indices(Plane::PatchesTag /*unused*/) const
{
	auto k = 0u;
	auto offs = 0u;
	auto leap = m_udiv + 1;
	IndexArray indices(m_vdiv * m_udiv * 6);

	for (auto j = 0u; j != m_vdiv; ++j) {
		for (auto i = 0u; i != m_udiv; ++i) {
			indices[k++] = offs + i;
			indices[k++] = offs + i + leap;
			indices[k++] = offs + i + 1;
			indices[k++] = offs + i + 1;
			indices[k++] = offs + i + leap;
			indices[k++] = offs + i + leap + 1;
		}
		offs += leap;
	}
	assert(k == indices.size());
	return indices;
}

inline std::vector<spu::SpuCommand> Plane::instructions(Plane::PatchesTag /*unused*/) const
{
	spu::SpuCommand com;
	com.target = GL_ELEMENT_ARRAY_BUFFER;
	com.mode = GL_PATCHES;
	com.first = 0;
	com.count = uint32_t(m_vdiv * m_udiv * 6);

	com.flags = 0;

	return {com};
}

inline Plane::IndexArray Plane::indices(Plane::EdgesTag /*unused*/) const
{
	auto k = 0u;
	auto leap = m_udiv + 1;
	IndexArray indices(1 + 2 * (m_udiv + m_vdiv));

	for (auto i = 0u; i != leap; ++i) {
		indices[k++] = i;
	}
	for (auto j = 0u; j != m_vdiv; ++j) {
		indices[k++] = (j + 2) * leap - 1;
	}
	for (auto i = 0u; i != m_udiv; ++i) {
		indices[k++] = (leap * (m_vdiv + 1)) - 2 - i;
	}
	for (auto j = 0u; j != m_vdiv; ++j) {
		indices[k++] = (m_vdiv - j - 1) * leap;
	}

	assert(k == indices.size());
	return indices;
}

inline std::vector<spu::SpuCommand> Plane::instructions(Plane::EdgesTag /*unused*/) const
{
	spu::SpuCommand com;
	com.target = GL_ELEMENT_ARRAY_BUFFER;
	com.mode = GL_LINE_STRIP;
	com.first = 0;
	com.count = uint32_t(1 + 2 * (m_udiv + m_vdiv));
	com.flags = 0;

	return {com};
}

}  // namespace spu::oglplus::shapes
