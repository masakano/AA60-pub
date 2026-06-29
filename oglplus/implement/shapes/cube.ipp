
namespace spu::oglplus::shapes {

inline uint32_t Cube::positions(std::vector<float> &dest) const
{
	/*
	 *   (E)-----(A)
	 *   /|      /|
	 *  / |     / |
	 *(F)-----(B) |
	 * | (H)---|-(D)
	 * | /     | /
	 * |/      |/
	 *(G)-----(C)
	 *
	 */
	using T = double;
	using V = float;
	const T half_x = T(m_sx) / T(2);
	const T half_y = T(m_sy) / T(2);
	const T half_z = T(m_sz) / T(2);
	const V c[8][3] = {
	        {V(m_ox + half_x), V(m_oy + half_y), V(m_oz - half_z)}, //(A)
	        {V(m_ox + half_x), V(m_oy + half_y), V(m_oz + half_z)}, //(B)
	        {V(m_ox + half_x), V(m_oy - half_y), V(m_oz + half_z)}, //(C)
	        {V(m_ox + half_x), V(m_oy - half_y), V(m_oz - half_z)}, //(D)
	        {V(m_ox - half_x), V(m_oy + half_y), V(m_oz - half_z)}, //(E)
	        {V(m_ox - half_x), V(m_oy + half_y), V(m_oz + half_z)}, //(F)
	        {V(m_ox - half_x), V(m_oy - half_y), V(m_oz + half_z)}, //(G)
	        {V(m_ox - half_x), V(m_oy - half_y), V(m_oz - half_z)}  //(H)
	};
	const auto A = 0;
	const auto B = 1;
	const auto C = 2;
	const auto D = 3;
	const auto E = 4;
	const auto F = 5;
	const auto G = 6;
	const auto H = 7;

	dest.resize(108);
	auto p = dest.begin();

	*p++ = c[A][0];
	*p++ = c[A][1];
	*p++ = c[A][2];
	*p++ = c[D][0];
	*p++ = c[D][1];
	*p++ = c[D][2];
	*p++ = c[B][0];
	*p++ = c[B][1];
	*p++ = c[B][2];
	*p++ = c[C][0];
	*p++ = c[C][1];
	*p++ = c[C][2];
	*p++ = c[B][0];
	*p++ = c[B][1];
	*p++ = c[B][2];
	*p++ = c[D][0];
	*p++ = c[D][1];
	*p++ = c[D][2];

	*p++ = c[A][0];
	*p++ = c[A][1];
	*p++ = c[A][2];
	*p++ = c[B][0];
	*p++ = c[B][1];
	*p++ = c[B][2];
	*p++ = c[E][0];
	*p++ = c[E][1];
	*p++ = c[E][2];
	*p++ = c[F][0];
	*p++ = c[F][1];
	*p++ = c[F][2];
	*p++ = c[E][0];
	*p++ = c[E][1];
	*p++ = c[E][2];
	*p++ = c[B][0];
	*p++ = c[B][1];
	*p++ = c[B][2];

	*p++ = c[B][0];
	*p++ = c[B][1];
	*p++ = c[B][2];
	*p++ = c[C][0];
	*p++ = c[C][1];
	*p++ = c[C][2];
	*p++ = c[F][0];
	*p++ = c[F][1];
	*p++ = c[F][2];
	*p++ = c[G][0];
	*p++ = c[G][1];
	*p++ = c[G][2];
	*p++ = c[F][0];
	*p++ = c[F][1];
	*p++ = c[F][2];
	*p++ = c[C][0];
	*p++ = c[C][1];
	*p++ = c[C][2];

	*p++ = c[F][0];
	*p++ = c[F][1];
	*p++ = c[F][2];
	*p++ = c[G][0];
	*p++ = c[G][1];
	*p++ = c[G][2];
	*p++ = c[E][0];
	*p++ = c[E][1];
	*p++ = c[E][2];
	*p++ = c[H][0];
	*p++ = c[H][1];
	*p++ = c[H][2];
	*p++ = c[E][0];
	*p++ = c[E][1];
	*p++ = c[E][2];
	*p++ = c[G][0];
	*p++ = c[G][1];
	*p++ = c[G][2];

	*p++ = c[H][0];
	*p++ = c[H][1];
	*p++ = c[H][2];
	*p++ = c[G][0];
	*p++ = c[G][1];
	*p++ = c[G][2];
	*p++ = c[D][0];
	*p++ = c[D][1];
	*p++ = c[D][2];
	*p++ = c[C][0];
	*p++ = c[C][1];
	*p++ = c[C][2];
	*p++ = c[D][0];
	*p++ = c[D][1];
	*p++ = c[D][2];
	*p++ = c[G][0];
	*p++ = c[G][1];
	*p++ = c[G][2];

	*p++ = c[E][0];
	*p++ = c[E][1];
	*p++ = c[E][2];
	*p++ = c[H][0];
	*p++ = c[H][1];
	*p++ = c[H][2];
	*p++ = c[A][0];
	*p++ = c[A][1];
	*p++ = c[A][2];
	*p++ = c[D][0];
	*p++ = c[D][1];
	*p++ = c[D][2];
	*p++ = c[A][0];
	*p++ = c[A][1];
	*p++ = c[A][2];
	*p++ = c[H][0];
	*p++ = c[H][1];
	*p++ = c[H][2];

	assert(p == dest.end());

	return 3;
}

inline uint32_t Cube::normals(std::vector<float> &dest) const
{
	const float n[6][3] = {
	        {+1, 0,  0 },
                {0,  +1, 0 },
                {0,  0,  +1},
                {-1, 0,  0 },
                {0,  -1, 0 },
                {0,  0,  -1}
        };
	dest.resize(108);

	auto vi = dest.begin();

	for (int32_t f = 0; f != 6; ++f) {
		for (int32_t v = 0; v != 6; ++v) {
			for (int32_t c = 0; c != 3; ++c) {
				*vi++ = n[f][c];
			}
		}
	}
	assert(vi == dest.end());
	return 3;
}

inline uint32_t Cube::tangents(std::vector<float> &dest) const
{
	const float n[6][3] = {
	        {0,  0, -1},
                {+1, 0, 0 },
                {+1, 0, 0 },
                {0,  0, +1},
                {-1, 0, 0 },
                {-1, 0, 0 }
        };

	dest.resize(108);
	auto vi = dest.begin();
	for (int32_t f = 0; f != 6; ++f) {
		for (int32_t v = 0; v != 6; ++v) {
			for (int32_t c = 0; c != 3; ++c) {
				*vi++ = n[f][c];
			}
		}
	}
	assert(vi == dest.end());
	return 3;
}

inline uint32_t Cube::texCoordinates(std::vector<float> &dest) const
{
	const float n[6][2] = {
	        {+1, +1},
                {+1, 0 },
                {0,  +1},
                {0,  0 },
                {0,  +1},
                {+1, 0 }
        };

	dest.resize(108);
	auto vi = dest.begin();
	for (int32_t f = 0; f != 6; ++f) {
		for (int32_t v = 0; v != 6; ++v) {
			for (int32_t c = 0; c != 2; ++c) {
				*vi++ = n[v][c];
			}
			*vi++ = f;
		}
	}
	assert(vi == dest.end());
	return 3;
}

inline std::vector<spu::SpuCommand> Cube::instructions(Cube::DefaultTag) const
{
	spu::SpuCommand com;
	com.target = GL_ARRAY_BUFFER;
	com.mode = GL_TRIANGLES;
	com.first = 0;
	com.count = 36;

	com.flags = 0;

	return {com};
}

inline Cube::IndexArray Cube::indices(Cube::EdgesTag) const
{
	/*
	 *   (E)-----(A)
	 *   /|      /|
	 *  / |     / |
	 *(F)-----(B) |
	 * | (H)---|-(D)
	 * | /     | /
	 * |/      |/
	 *(G)-----(C)
	 *
	 */
	uint16_t indices[24] = {
	        0,  1,  5,
	        2,  //+x
	        19, 22, 23,
	        18,  //-x
	        6,  7,  10,
	        11,  //+y
	        26, 29, 24,
	        25,  //-y
	        12, 13, 16,
	        17,  //+z
	        31, 35, 32,
	        30  //-z
	};
	return IndexArray(indices, indices + 24);
}

inline std::vector<spu::SpuCommand> Cube::instructions(Cube::EdgesTag) const
{
	std::vector<spu::SpuCommand> instructions;
	for (auto r = 0; r != 6; ++r) {
		spu::SpuCommand com;
		com.target = GL_ELEMENT_ARRAY_BUFFER;
		com.mode = GL_LINE_LOOP;
		com.first = uint32_t(r * 4);
		com.count = uint32_t(4);
		com.flags = 0;
		instructions.push_back(com);
	}
	return instructions;
}

}  // namespace spu::oglplus::shapes
