
namespace spu::oglplus::shapes {

inline Tetrahedrons::IndexArray Tetrahedrons::indices(WithAdjacencyTag /*unused*/) const
{
	const auto n = m_divisions;
	const auto a = n + 1;
	const auto b = a * a;
	IndexArray indices(n * n * n * 6 * 6);
	auto k = 0u;

	/*
	 *   (E)-----(F)
	 *   /|      /|
	 *  / |     / |
	 *(H)-----(G) |
	 * | (A)---|-(B)
	 * | /     | /
	 * |/      |/
	 *(D)-----(C)
	 *
	 */

	for (auto z = 0u; z != n; ++z) {
		for (auto y = 0u; y != n; ++y) {
			for (auto x = 0u; x != n; ++x) {
				auto A = (z + 0) * b + (y + 0) * a + (x + 0) + 1;
				auto B = (z + 0) * b + (y + 0) * a + (x + 1) + 1;
				auto C = (z + 1) * b + (y + 0) * a + (x + 1) + 1;
				auto D = (z + 1) * b + (y + 0) * a + (x + 0) + 1;
				auto E = (z + 0) * b + (y + 1) * a + (x + 0) + 1;
				auto F = (z + 0) * b + (y + 1) * a + (x + 1) + 1;
				auto G = (z + 1) * b + (y + 1) * a + (x + 1) + 1;
				auto H = (z + 1) * b + (y + 1) * a + (x + 0) + 1;
				auto O = 0;

				assert(A < a * a * a + 1);
				assert(B < a * a * a + 1);
				assert(C < a * a * a + 1);
				assert(D < a * a * a + 1);
				assert(E < a * a * a + 1);
				assert(F < a * a * a + 1);
				assert(G < a * a * a + 1);
				assert(H < a * a * a + 1);

				indices[k++] = C;
				indices[k++] = A;
				indices[k++] = B;
				indices[k++] = O;
				indices[k++] = G;
				indices[k++] = O;

				indices[k++] = B;
				indices[k++] = G;
				indices[k++] = A;
				indices[k++] = O;
				indices[k++] = F;
				indices[k++] = O;

				indices[k++] = E;
				indices[k++] = G;
				indices[k++] = F;
				indices[k++] = O;
				indices[k++] = A;
				indices[k++] = O;

				indices[k++] = E;
				indices[k++] = G;
				indices[k++] = A;
				indices[k++] = O;
				indices[k++] = H;
				indices[k++] = O;

				indices[k++] = A;
				indices[k++] = G;
				indices[k++] = D;
				indices[k++] = O;
				indices[k++] = H;
				indices[k++] = O;

				indices[k++] = D;
				indices[k++] = G;
				indices[k++] = A;
				indices[k++] = O;
				indices[k++] = C;
				indices[k++] = O;
			}
		}
	}

	assert(k == indices.size());
	return indices;
}

inline std::vector<spu::SpuCommand> Tetrahedrons::instructions(WithAdjacencyTag /*unused*/) const
{
	const auto n = m_divisions;
	spu::SpuCommand com;
	com.target = GL_ELEMENT_ARRAY_BUFFER;
	com.mode = GL_TRIANGLES_ADJACENCY;
	com.first = 0;
	com.count = uint32_t(n * n * n * 6 * 6);

	com.flags = 0;

	return {com};
}

}  // namespace spu::oglplus::shapes
