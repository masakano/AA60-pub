//
// GsObject :
//
#include <gsys/node/dui.h>

#include <algorithm>
#include <cstdio>

// extern bool is_debug;
namespace spu::gs_node::dui {

Popup::Popup(Container *contents, Button *button, Button *popper) : Popup()
{
	// button
	{
		m_button = button;
		m_button->getValue() = e_off;

		auto container_range = getARange();
		auto button_range = m_button->getARange();
		auto from = upperLeftOf(button_range);
		auto to = upperLeftOf(container_range);

		m_button->move(to - from);

		add(m_button);
	}

	// popper
	{
		m_popper = popper;

		auto button_range = m_button->getARange();
		auto popper_range = m_popper->getARange();

		auto from = rightOf(popper_range);
		auto to = rightOf(button_range);

		m_popper->move(to - from);
		m_popper->getValue() = e_off;
		add(m_popper);
	}

	// contents
	{
		m_contents = contents;
		m_contents->setName(m_button->name());
		m_contents->GsObject::setProperty(e_render, 0);

		auto button_range = m_button->getARange();
		auto contents_range = m_contents->getARange();

		auto from = upperLeftOf(contents_range);
		auto to = lowerLeftOf(button_range);

		m_contents->move(to - from);

		add(m_contents, false);  // no expand
	}
	DuiNode::setName(m_button->name());
}

void Popup::update()
{
	if (int32_t(m_contents->getValue()) == e_inactive) {
		m_button->getValue() = e_inactive;
		m_popper->getValue() = e_inactive;
		return;
	}

	auto stat = (ms_gesture.prev().mouse_L << 1) | ms_gesture.curr().mouse_L;
	switch (stat) {
	case 0x01: {  // press
		auto is_button_inside = m_button->insideState().curr;
		auto is_popper_inside = m_popper->insideState().curr;

		if (is_button_inside || is_popper_inside) {
			m_contents->GsObject::setProperty(e_render, 1);
			dynamic_cast<Container *>(getParent())->relink(this);  // rise priority
		}
		break;
	}
	case 0x03: {  // drag
		if (m_contents->getProperty(e_render)) {
			getValue() = m_contents->getValue();
			if (m_button->text()) {
				const auto *hot_node = m_contents->hotNode();
				if (hot_node) {
					auto name = hot_node->name();
					m_button->text()->setText(name);
				}
			}
		}
		break;
	}
	case 0x02: {  // release
		m_contents->GsObject::setProperty(e_render, 0);
		break;
	}
	}

	auto is_visible = m_contents->getProperty(e_render);
	m_popper->getValue() = is_visible ? e_on : e_off;
	m_button->getValue() = is_visible ? e_on : e_off;

	Container::update();
}
}  // namespace spu::gs_node::dui
