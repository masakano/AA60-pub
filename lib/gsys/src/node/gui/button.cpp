//
// Base :
//
#include <gsys/node/gui/button.h>

namespace spu::gs_node::gui {

void Button::init(const Attrs &attrs)
{
	Base::init(attrs);

	const auto c_indent = 4;
	auto *desc = attrs.get<Desc *>("desc", nullptr);
	auto *name = attrs.get<const char *>("name", nullptr);
	auto *parent = attrs.get<Base *>("parent", nullptr);

	auto columns = getColumns(name, parent);
	auto op = [](const char *s) { return int32_t(strlen(s)); };

	assert(desc);
	auto item_max_length = op(*vector_max(desc->items, op)) + c_indent;

	auto ndiv = columns / item_max_length;
	auto item_length = columns / ndiv;
	auto items = Base::normalizestr(desc->items, c_indent, item_length);

	auto x = 0.0f;
	auto y = -1.0f;  // start with -1
	auto &graphic = getGraphic(e_button);

	for (auto &item: items) {
		auto i = &item - &items[0];
		ButtonState s;
		gs_painter::Sprite::Vertex b;

		b.p = Vec3f(x + c_indent / 2, y * c_charheight, 0);
		b.t = Vec4f(0, 0, 1, 1);
		b.s = Vec4f(-0.75, -0.75, 0.75, 0.75);

		s.text = item;
		s.value_ptr = desc->value_ptrs[i];
		s.range = Range3f(b.p + Vec3f(b.s.x, b.s.y, 0), b.p + Vec3f(b.s.z, b.s.w, 0));
		s.state = s.value_ptr ? e_active : e_disabled;
		s.is_featured = false;

		m_states.push_back(s);
		graphic.vertices.push_back(b);

		x += item_length;
		if (x + item_length > columns || i == int32_t(items.size()) - 1) {
			m_states.back().text += '\n';
			x = 0;
			y--;
		}
	}
	Base::bakeTextInternal<ButtonState, int32_t>(m_states);
	Base::update();
}

void Button::changeState(const std::vector<const void *> &ptrs, const hash32_t &state)
{
	Base::changeStateInternal<ButtonState, int32_t>(m_states, ptrs, state);
}

void Button::update()
{
	auto cursor = getCursor(getGesture()->curr().cursor);
	if (!isInside(cursor)) return;

	for (auto &s: m_states) {
		s.is_featured = false;
		if (s.state != e_disabled && s.range.inside(cursor)) {
			s.is_featured = true;
			auto *gesture = getGesture();
			if (!gesture->prev().mouse_L && gesture->curr().mouse_L) {
				*s.value_ptr = *s.value_ptr ? 0 : 1;
				ms_lastUpdateCount = getSeconds().count();
			}
		}
	}

	auto &graphic = getGraphic(e_button);
	for (auto i = 0u; i < m_states.size(); i++) {
		auto &s = m_states[i];
		auto &b = graphic.vertices[i];
		auto state = s.state == e_disabled ? e_disabled : *s.value_ptr ? e_active : e_inactive;
		b.c = getColors()[state];
	}
	Base::update();
}
}  // namespace spu::gs_node::gui
