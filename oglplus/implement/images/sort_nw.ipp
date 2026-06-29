
namespace spu::oglplus::images {

inline uint32_t SortNWMap::pot(uint32_t n) { return 1 << n; }

inline uint32_t SortNWMap::next_log(uint32_t n)
{
	auto pot = 1u;
	auto result = 0u;
	while (n > pot) {
		pot <<= 1;
		++result;
	}
	return result;
}

inline uint32_t SortNWMap::num_steps(uint32_t size)
{
	assert(size > 0);
	auto result = 0u;
	for (uint32_t i = 0, l = next_log(size); i != l; ++i) {
		result += i + 1;
	}
	return result;
}

inline SortNWMap::SortNWMap(uint32_t size) : Image(size, num_steps(size), 1, 1, static_cast<T *>(nullptr))
{
	this->m_format = GL_RED_INTEGER;
	this->m_internal = GL_R16UI;
	this->bzero();
	auto y = 0u;
	for (uint32_t m = next_log(size), l = 0; l < m; ++l) {
		for (auto b = l + 1; b > 0; --b) {
			auto p = pot(b);
			auto q = pot(b - 1);
			auto r = pot(l + 1 - b);
			auto x = 0;
			T enc;

			for (uint32_t c = 0, d = size / p; c < d; ++c) {
				auto direction = (c / r) % 2 == 0;
				for (auto e = 0u; e < q; ++e) {
					x = c * p + e;
					enc = q << 2;  // step
					enc |= 0x0;    // step sign (positive)
					enc |= direction ? 0x0 : 0x1;
					this->at<T>(x, y) = enc;

					x = x + q;
					enc = q << 2;  // step
					enc |= 0x2;    // step sign (negative)
					enc |= direction ? 0x0 : 0x1;
					this->at<T>(x, y) = enc;
				}
			}
			auto f = (size / p);
			for (uint32_t c = 0, d = size % q; c < d; ++c) {
				if (f * p + c + q < size) {
					auto direction = (f / r) % 2 == 0;
					x = f * p + c;
					enc = q << 2;  // step
					enc |= 0x0;    // step sign (positive)
					enc |= direction ? 0x0 : 0x1;
					this->at<T>(x, y) = enc;

					x = x + q;
					enc = q << 2;  // step
					enc |= 0x2;    // step sign (negative)
					enc |= direction ? 0x0 : 0x1;
					this->at<T>(x, y) = enc;
				}
			}
			++y;
		}
	}
}
}  // namespace spu::oglplus::images
