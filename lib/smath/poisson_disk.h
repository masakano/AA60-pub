//
// PoissonDisk :
//
#include <ssys/random_generator.h>
#include "sweep_and_prune.h"
#include "random_bitmap.h"

namespace spu {

template<class vec_t = Vec3f> class PoissonDisk {
public:
	using delta_func_t = std::function<float(const vec_t &)>;
	static constexpr float c_max_fails = 64;

	void exec(float radius, float step = 1.0f, bool is_2d = false, const delta_func_t &func = nullptr)
	{
		m_radius = radius;
		m_step = step;
		m_is2D = is_2d;
		m_func = func;
		m_bitmap.init(m_radius, m_step, m_is2D);
		m_sap.init(1);
		set_last_sap_point(vec_t(0));
		while (add_point()) {
			/* do nothing */
		}
	}

	bool add_point()
	{
		auto n_fail = 0;

		m_sap.init(m_sap.size() + 1);
		while (n_fail < c_max_fails) {
			auto result = m_bitmap.select();
			assert(result.hit);
			auto new_point = result.value;
			if (length(new_point) < m_radius) {
				set_last_sap_point(new_point);
				m_sap.sort();
				auto last_index = m_sap.size() - 1;
				if (!hit_any(last_index)) {
					auto delta = calc_delta(new_point);
					m_bitmap.set(new_point, delta);
					return true;
				}
			}
			n_fail++;
		}
		return false;
	}
	std::vector<vec_t> points() const { return m_sap.centerPoints(); }

private:
	RandomGenerator<float> m_frand;
	SweepAndPrune<vec_t> m_sap;
	RandomBitmap<vec_t> m_bitmap;

	delta_func_t m_func = nullptr;
	float m_radius = 1.0;
	float m_step = 1.0 / 8.0;
	bool m_is2D = false;

	float calc_delta(const vec_t point) { return m_func ? std::max(m_step, m_func(point)) : m_step; }

	bool hit_any(uint32_t index0) const
	{
		for (auto index1 = 0u; index1 < m_sap.size(); index1++) {
			if (index1 != index0 && m_sap.hit(index0, index1)) {
				return true;
			}
		}
		return false;
	}

	void set_last_sap_point(const vec_t &new_point)
	{
		auto &pair_points = m_sap.getPairPoints();
		auto last_index = m_sap.size() - 1;
		auto delta = calc_delta(new_point);

		pair_points[last_index * 2 + 0] = new_point - vec_t(delta * 0.5);
		pair_points[last_index * 2 + 1] = new_point + vec_t(delta * 0.5);
	}
};
}  // namespace spu
