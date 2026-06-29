
namespace spu::oglplus::shapes {

inline std::vector<spu::SpuCommand> SimpleIcosahedron::instructions(uint32_t mode) const
{
	spu::SpuCommand com;
	com.target = GL_ELEMENT_ARRAY_BUFFER;
	com.mode = mode;
	com.first = 0;
	com.count = 20 * 3;

	com.flags = 0;

	return {com};
}

inline std::vector<spu::SpuCommand> Icosahedron::instructions(uint32_t mode) const
{
	spu::SpuCommand com;
	com.target = GL_ARRAY_BUFFER;
	com.mode = mode;
	com.first = 0;
	com.count = 20 * 3;

	com.flags = 0;

	return {com};
}

inline const uint16_t *IcosahedronBase::indices()
{
	static const uint16_t indices[20 * 3]
	        = {2, 1,  0, 3, 2,  0, 4,  3,  0,  5, 4, 0, 1, 5, 0, 11, 6, 7, 11, 7,
	           8, 11, 8, 9, 11, 9, 10, 11, 10, 6, 1, 2, 6, 2, 3, 7,  3, 4, 8,  4,
	           5, 9,  5, 1, 10, 2, 7,  6,  3,  8, 7, 4, 9, 8, 5, 10, 9, 1, 6,  10};
	return indices;
}

inline const double *IcosahedronBase::positions()
{
	static const double positions[12 * 3] = {
	        0.000,  1.000,  0.000,  0.894,  0.447,  0.000,  0.276, 0.447,  0.851,  -0.724, 0.447,  0.526,
	        -0.724, 0.447,  -0.526, 0.276,  0.447,  -0.851, 0.724, -0.447, 0.526,  -0.276, -0.447, 0.851,
	        -0.894, -0.447, 0.000,  -0.276, -0.447, -0.851, 0.724, -0.447, -0.526, 0.000,  -1.000, 0.000};
	return positions;
}

}  // namespace spu::oglplus::shapes
