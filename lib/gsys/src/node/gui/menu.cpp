//
// Base :
//
#include <gsys/node/gui/menu.h>

namespace spu::gs_node::gui {

void Menu::init(const Attrs &attrs)
{
	Base::init(attrs);

	const auto c_indent = 4;
	const auto c_indent_str = std::string(c_indent, ' ');

	struct Action {
		std::string text;
		float x;
		float y;
		int32_t group;
		int32_t value;
	};
	std::vector<Action> actions;

	auto *desc = attrs.get<Desc *>("desc", nullptr);
	auto *name = attrs.get<const char *>("name", nullptr);
	auto *parent = attrs.get<Base *>("parent", nullptr);
	auto columns = getColumns(name, parent);

	assert(desc);
	auto item_max_length = 0;
	for (auto &items: desc->items) {
		auto op = [](const Item &item) { return int32_t(strlen(item.key)); };
		item_max_length = std::max(item_max_length, op(*vector_max(items, op)) + c_indent);
	}
	auto ndiv = std::max(1, columns / item_max_length);
	auto item_length = columns / ndiv;

	auto x = 0.0f;
	auto y = -1.0f;  // start with -1

	for (auto i = 0; i < int32_t(desc->items.size()); i++) {
		auto &items = desc->items[i];
		std::vector<const char *> raw_item_keys;
		for (auto &item: items) {
			raw_item_keys.push_back(item.key);
		}
		auto item_keys = Base::normalizestr(raw_item_keys, c_indent, item_length);
		for (auto j = 0; j < int32_t(items.size()); j++) {
			auto key = item_keys[j];
			auto value = items[j].value;
			actions.push_back({key, x + c_indent / 2, y, i, value});
			x += item_length;
			if (x + item_length > columns || j == int32_t(items.size()) - 1) {
				actions.back().text += '\n';
				x = 0;
				y--;
			}
		}
	}

	// compile
	auto &graphic = getGraphic(e_button);
	for (auto &a: actions) {
		MenuState s;
		gs_painter::Sprite::Vertex b;

		b.p = Vec3f(a.x, a.y * c_charheight, 0);
		b.t = Vec4f(0, 0, 1, 1);
		b.s = Vec4f(-0.75, -0.75, 0.75, 0.75);

		s.text = a.text;
		s.range = Range3f(b.p + Vec3f(b.s.x, b.s.y, 0), b.p + Vec3f(b.s.z, b.s.w, 0));
		s.group = a.group;
		s.value = a.value;
		s.is_featured = false;
		s.value_ptr = desc->value_ptrs[s.group];
		s.state = s.value_ptr == nullptr ? e_disabled : *s.value_ptr == s.value ? e_active : e_inactive;

		b.c = getColors()[s.state];
		graphic.vertices.push_back(b);
		m_states.push_back(s);
	}
	Base::bakeTextInternal<MenuState, int32_t>(m_states);
	Base::update();
}
void Menu::changeState(const std::vector<const void *> &ptrs, const hash32_t &state)
{
	Base::changeStateInternal<MenuState, int32_t>(m_states, ptrs, state);
}

void Menu::update()
{
	auto cursor = getCursor(getGesture()->curr().cursor);
	if (!isInside(cursor)) return;

	for (auto &s0: m_states) {
		s0.is_featured = false;
		if (s0.state != e_disabled && s0.range.inside(cursor)) {
			s0.is_featured = true;
			auto *gesture = getGesture();
			if (!gesture->prev().mouse_L && gesture->curr().mouse_L) {
				for (auto &s1: m_states) {
					if (s0.group == s1.group) {
						s1.state = (&s0 == &s1) ? e_active : e_inactive;
					}
				}
				*s0.value_ptr = s0.value;
				ms_lastUpdateCount = getSeconds().count();
			}
		}
	}

	auto &graphic = getGraphic(e_button);
	for (auto i = 0u; i < m_states.size(); i++) {
		auto &s = m_states[i];
		graphic.vertices[i].c = getColors()[s.is_featured ? e_featured : s.state];
	}
	Base::update();
}

}  // namespace spu::gs_node::gui
