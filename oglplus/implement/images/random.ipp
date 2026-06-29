
#include <cstdlib>

namespace spu::oglplus::images {

inline RandomRedUByte::RandomRedUByte(int32_t width, int32_t height, int32_t depth)
        : Image(width, height, depth, 1, static_cast<u_char *>(nullptr))
{
	auto p = this->begin();
	auto e = this->end();
	for (int32_t k = 0; k != depth; ++k) {
		for (int32_t j = 0; j != height; ++j) {
			for (int32_t i = 0; i != width; ++i) {
				assert(p != e);
				*p = std::rand() % 0x100;
				++p;
			}
		}
	}
	assert(p == e);
}

inline RandomRGBUByte::RandomRGBUByte(int32_t width, int32_t height, int32_t depth)
        : Image(width, height, depth, 3, static_cast<u_char *>(nullptr))
{
	auto p = this->begin();
	auto e = this->end();
	for (int32_t k = 0; k != depth; ++k) {
		for (int32_t j = 0; j != height; ++j) {
			for (int32_t i = 0; i != width; ++i) {
				for (int32_t c = 0; c != 3; ++c) {
					assert(p != e);
					*p = std::rand() % 0x100;
					++p;
				}
			}
		}
	}
	assert(p == e);
}

}  // namespace spu::oglplus::images
