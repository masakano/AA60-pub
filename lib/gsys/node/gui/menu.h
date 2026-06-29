//
// Menu :
//
#pragma once

#include "base.h"

namespace spu::gs_node::gui {

class Menu : public Base {
public:
	struct Item {
		const char *key;
		int32_t value;
	};

	struct Desc {
		std::vector<std::vector<Item>> items;
		std::vector<int32_t *> value_ptrs;
	};

	explicit Menu(const char *name = nullptr) : Base(name) {}
	explicit Menu(const Attrs &attrs) : Menu() { init(attrs); }
	void init(const Attrs &attrs) override;
	void update() override;
	void changeState(const std::vector<const void *> &ptrs, const hash32_t &state) override;

private:
	struct MenuState : public State<int32_t> {
		Range2f range;
		int32_t group;
		int32_t value;
	};
	std::vector<MenuState> m_states;
};
}  // namespace spu::gs_node::gui
