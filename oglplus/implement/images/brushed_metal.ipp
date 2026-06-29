
#include <cstdlib>

namespace spu::oglplus::images {

inline void BrushedMetalUByte::_make_pixel(
        uint8_t *b, const uint8_t *e, int32_t w, int32_t h, int32_t x, int32_t y, double /*c*/, uint8_t r,
        uint8_t g)
{
	while (x < 0) {
		x += w;
	}
	while (y < 0) {
		y += h;
	}
	if (x >= w) {
		x %= w;
	}
	if (y >= h) {
		y %= h;
	}
	auto *p = b + (y * w + x) * 3;
	auto *pr = p;
	auto *pg = p + 1;
	auto *pb = p + 2;

	assert((pr < e) && (pg < e) && (pb < e));
	*pr = r;
	*pg = g;
	*pb = (*pb + 8) % 0x100;
}

inline void BrushedMetalUByte::_make_scratch(
        uint8_t *b, uint8_t *e, int32_t w, int32_t h, int32_t x, int32_t y, double dx, double dy)
{
	if ((dx == 0) && (dy == 0)) {
		return;
	}
	auto r = dy / std::sqrt(dx * dx + dy * dy) * 0xFF;
	auto g = dx / std::sqrt(dx * dx + dy * dy) * 0xFF;

	if (dx > dy) {
		if (dx >= 0) {
			for (auto i = 0; i != dx; ++i) {
				auto c = double(i) / dx;
				auto j = dy * c;
				_make_pixel(b, e, w, h, x + i, y + j, c, r, g);
			}
		}
		else {
			for (auto i = 0; i != dx; --i) {
				auto c = double(i) / dx;
				auto j = dy * c;
				_make_pixel(b, e, w, h, x + i, y + j, c, r, g);
			}
		}
	}
	else {
		if (dy >= 0) {
			for (auto j = 0; j != dy; ++j) {
				auto c = double(j) / dy;
				auto i = dx * c;
				_make_pixel(b, e, w, h, x + i, y + j, c, r, g);
			}
		}
		else {
			for (auto j = 0; j != dy; --j) {
				auto c = double(j) / dy;
				auto i = dx * c;
				_make_pixel(b, e, w, h, x + i, y + j, c, r, g);
			}
		}
	}
}

inline BrushedMetalUByte::BrushedMetalUByte(
        int32_t width, int32_t height, uint32_t n_scratches, int32_t s_disp_min, int32_t s_disp_max,
        int32_t t_disp_min, int32_t t_disp_max)
        : Image(width, height, 1, 3, static_cast<uint8_t *>(nullptr))
{
	auto *p = this->begin();
	auto *e = this->end();

	for (auto *pp = p; pp != e; ++pp) {
		*pp = 0;
	}
	while ((n_scratches--) != 0u) {
		const auto n_segments = 1 + std::rand() % 4;
		auto x = std::rand() % width;
		auto y = std::rand() % height;
		for (auto seg = 0; seg != n_segments; ++seg) {
			auto dx = s_disp_min + std::rand() % (s_disp_max - s_disp_min + 1);
			auto dy = t_disp_min + std::rand() % (t_disp_max - t_disp_min + 1);

			_make_scratch(p, e, width, height, x, y, dx, dy);
			x += dx;
			y += dy;
		}
	}
}

}  // namespace spu::oglplus::images
