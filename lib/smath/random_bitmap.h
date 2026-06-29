//
// RandomBitmap :
//
#include <ssys/random_generator.h>

namespace spu {

template<class vec_t = Vec3f> class RandomBitmap {
public:
	RandomBitmap() = default;

	RandomBitmap(float radius, float step, bool is_2d) { init(radius, step, is_2d); }

	void init(float radius, float step, bool is_2d)
	{
		m_radius = radius;
		m_step = step;
		m_is2D = is_2d;
		m_size = int32_t(2 * m_radius / m_step);
		auto map_size = m_is2D ? m_size * m_size : m_size * m_size * m_size;
		m_bitmap.resize(map_size / c_bit_size, 0);
	}

	optional_t<vec_t> select()
	{
		auto word_size = int32_t(m_bitmap.size());
		auto base_word_index = uint32_t(m_frand() * word_size);
		for (auto i = 0; i < word_size; i++) {
			auto word_index = (base_word_index + i) % word_size;
			if (m_bitmap[word_index] != ~0u) {
				auto base_bit_index = uint32_t(m_frand() * c_bit_size);
				for (auto j = 0u; j < c_bit_size; j++) {
					auto bit_index = (base_bit_index + j) % c_bit_size;
					auto bit_mask = (1lu << bit_index);
					if ((m_bitmap[word_index] & bit_mask) == 0) {
						auto index = word_index * c_bit_size + bit_index;
						auto point = index_to_point(index);
						auto jitter = vec_t(m_frand(), m_frand(), m_frand());
						point += m_step * jitter;
						if (m_is2D) point.z = 0;
						return {true, point};
					}
				}
			}
		}
		return {false, vec_t(0)};
	}
	void set(const vec_t &point, float delta)
	{
		delta = std::floor(delta / m_step) * m_step;
		for (auto dz = -delta; dz <= delta; dz += m_step) {
			for (auto dy = -delta; dy <= delta; dy += m_step) {
				for (auto dx = -delta; dx <= delta; dx += m_step) {
					auto adjacent_point = point + vec_t(dx, dy, dz);
					if (length(adjacent_point) < m_radius) {
						set(adjacent_point);
					}
				}
			}
		}
	}

	bool test(const vec_t &point)
	{
		auto pair = index_to_bitmap_index(point_to_index(point));
		return (m_bitmap[pair.first] & pair.second) != 0;
	}

	void set(const vec_t &point)
	{
		auto pair = index_to_bitmap_index(point_to_index(point));
		m_bitmap[pair.first] |= pair.second;
	}

	void clear(const vec_t &point)
	{
		auto pair = index_to_bitmap_index(point_to_index(point));
		m_bitmap[pair.first] &= ~pair.second;
	}

private:
	const uint32_t c_bit_size = sizeof(uint64_t) * 8;
	std::vector<uint64_t> m_bitmap;
	float m_radius = 0;
	float m_step = 0;
	uint32_t m_size = 0;
	bool m_is2D = false;
	RandomGenerator<float> m_frand;

	uint32_t point_to_index(const vec_t &point)
	{
		auto gp = Vec4i((point + m_radius) / m_step);
		if (m_is2D) gp.z = 0;
		return gp.x + gp.y * m_size + gp.z * (m_size * m_size);
	}

	vec_t index_to_point(uint32_t index)
	{
		vec_t gp;
		gp.x = index % m_size, index /= m_size;
		gp.y = index % m_size, index /= m_size;
		gp.z = index % m_size;
		auto point = gp * m_step - m_radius;
		if (m_is2D) point.z = 0;
		return point;
	}

	std::pair<uint32_t, uint64_t> index_to_bitmap_index(uint32_t index)
	{
		return {
		        index / c_bit_size,
		        1lu << (index % c_bit_size),
		};
	}
};
}  // namespace spu
