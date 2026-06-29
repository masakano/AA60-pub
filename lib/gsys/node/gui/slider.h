//
// Slider :
//
#pragma once

#include "base.h"

namespace spu::gs_node::gui {

class Slider : public Base {
public:
	struct Desc {
		std::vector<const char *> items;
		std::vector<float *> value_ptrs;
		std::vector<Vec2f> value_minmaxs;
		std::vector<float> value_powers;
		int32_t line_spacing = 0;
	};

	explicit Slider(const char *name = nullptr) : Base(name) {}
	explicit Slider(const Attrs &attrs) : Slider() { init(attrs); }

	void init(const Attrs &attrs) override;
	void update() override;
	void changeState(const std::vector<const void *> &ptrs, const hash32_t &state) override;
	void save(File &file) const override;
	void load(File &file) override;

protected:
	struct SliderState : public State<float> {
		Vec2f value_minmax;
		float value_power;
	};

	std::vector<SliderState> m_states;
	Vec2f m_minmax = {0.0, 1.0};

	static float delerp(float v, const Vec2f &minmax);
	static float lerp(float r, const Vec2f &minmax);
};
}  // namespace spu::gs_node::gui
