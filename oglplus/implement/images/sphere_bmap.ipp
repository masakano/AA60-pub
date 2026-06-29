
#include <smath/vec.h>

namespace spu::oglplus::images {

inline SphereBumpMap::SphereBumpMap(int32_t width, int32_t height, int32_t xrep, int32_t yrep)
        : Image(width, height, 1, 4, static_cast<float *>(nullptr), GL_RGBA, GL_RGBA16F)
{
	assert(width != 0 && height != 0);
	assert(xrep != 0 && yrep != 0);

	// auto one   = float(1);
	auto invw = (2.0 * xrep) / width;
	auto invh = (2.0 * yrep) / height;

	auto hi = width / xrep;
	auto hj = height / yrep;

	auto *p = this->begin<float>();
	for (auto j = 0; j != height; ++j) {
		auto y = float((j % hj) - hj / 2) * invh;
		for (auto i = 0; i != width; ++i) {
			auto x = float((i % hi) - hi / 2) * invw;
			auto l = sqrtf(x * x + y * y);
			auto d2 = 1.0f - l * l;
			auto d = d2 > 0 ? sqrtf(d2) : 0;

			auto z = ez();
			auto n = Vec3f(-x, -y, d);
			auto v = (l >= 1.0) ? z : normalize(z + n);

			if (l >= 1.0) {
				d = 0;
			}
			assert(p != this->end<float>());
			*p = v.x;
			++p;
			*p = v.y;
			++p;
			*p = v.z;
			++p;
			*p = d;
			++p;
		}
	}
	assert(p == this->end<float>());
}

}  // namespace spu::oglplus::images
