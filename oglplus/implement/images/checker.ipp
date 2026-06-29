

namespace spu::oglplus::images {

inline CheckerRedBlack::CheckerRedBlack(int32_t width, int32_t height, int32_t xrep, int32_t yrep)
        : Image(width, height, 1, 1, static_cast<uint8_t *>(nullptr), GL_RED, GL_RED)
{
	assert(width != 0 && height != 0);
	assert(xrep != 0 && yrep != 0);

	auto xdiv = width / xrep;
	auto ydiv = height / yrep;

	auto *p = this->begin<uint8_t>();
	for (auto j = 0; j != height; ++j) {
		auto y = j / ydiv;
		for (auto i = 0; i != width; ++i) {
			auto x = i / xdiv;
			auto c = ((x + y) % 2 == 0) ? 0x00 : 0xFF;
			assert(p != this->end<uint8_t>());
			*p = c;
			++p;
		}
	}
	assert(p == this->end<uint8_t>());
}

}  // namespace spu::oglplus::images
