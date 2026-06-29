

namespace spu::oglplus::images {

inline Squares::Squares(int32_t width, int32_t height, float ratio, int32_t xrep, int32_t yrep)
        : Image(width, height, 1, 1, static_cast<uint8_t *>(nullptr))
{
	assert(width != 0 && height != 0);
	assert(ratio > 0.0f && ratio <= 1.0f);
	assert(xrep != 0 && yrep != 0);

	auto *p = this->begin();

	auto rmin = (1.0f - ratio) * 0.5f;
	auto rmax = rmin + ratio;

	for (auto y = 0; y != height; ++y) {
		for (auto x = 0; x != width; ++x) {
			auto vx = float((x * xrep) % width) / width;
			auto vy = float((y * yrep) % height) / height;
			auto outside = ((vx < rmin) || (vx > rmax)) || ((vy < rmin) || (vy > rmax));
			*p++ = outside ? 0x00 : 0xFF;
		}
	}
	assert(p == this->end());
}

}  // namespace spu::oglplus::images
