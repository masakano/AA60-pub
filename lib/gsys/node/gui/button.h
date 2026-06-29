//
// Button :
//
#pragma once

#include "base.h"

namespace spu::gs_node::gui {

class Button : public Base {
public:
	struct Desc {
		std::vector<const char *> items;
		std::vector<int32_t *> value_ptrs;
	};

	explicit Button(const char *name = nullptr) : Base(name) {}

	explicit Button(const Attrs &attrs) : Button() { init(attrs); }
	void init(const Attrs &attrs) override;
	void update() override;
	void changeState(const std::vector<const void *> &ptrs, const hash32_t &state) override;

private:
	struct ButtonState : public State<int32_t> {
		Range2f range;
	};
	std::vector<ButtonState> m_states;
};
}  // namespace spu::gs_node::gui
