//
// Button :
//
#include <gsys/node/dui.h>

namespace spu::gs_node::dui {

void Button::init(const Attrs &attrs)
{
	DuiNode::init(attrs);

	m_type = attrs.get<int32_t>("type", e_radio);
	m_enum = attrs.get<int32_t>("enum", 1);
	m_text = attrs.get<Text *>("text", nullptr);
	m_offNode = attrs.get<DuiNode *>("off_node", nullptr);
	m_onNode = attrs.get<DuiNode *>("on_node", nullptr);

	auto &range = getARange();

	auto add_button = [&](DuiNode *element) {
		auto &element_range = element->getARange();
		auto from = leftOf(element_range);
		auto to = leftOf(range);
		if (m_type == e_radio || m_type == e_check) {
			to += Vec2f(c_fontSize * 0.5, 0);
		}
		element_range = element_range + (to - from);
		add(element);
	};
	if (m_offNode) {
		add_button(m_offNode);
	}
	if (m_onNode) {
		add_button(m_onNode);
	}
	if (m_text) {
		setName(m_text->name());
		auto &element_range = m_offNode->getARange();
		auto &text_range = m_text->getARange();

		if (m_offNode == nullptr) {
			auto from = leftOf(text_range);
			auto to = leftOf(range);
			text_range = text_range + (to - from);
		}
		else if (m_type == e_radio || m_type == e_check) {
			auto from = leftOf(text_range);
			auto to = rightOf(element_range);
			text_range = text_range + (to - from);
		}
		else {
			auto from = centerOf(text_range);
			auto to = centerOf(element_range);
			text_range = text_range + (to - from);
		}
		add(m_text);
	}
	squash();
}

void Button::setColor(const Vec4f &color, uint32_t mask)
{
	DuiNode::setColor(color, mask);
	if (m_text) {
		m_text->setColor(color, mask);
	}
}

void Button::update()
{
	DuiNode::update();

	auto stat = (ms_gesture.prev().mouse_L << 1) | ms_gesture.curr().mouse_L;
	switch (stat) {
	case 0x1: {  // press
		break;
	}
	case 0x3: {  // drog
		if (m_type == e_slide) {
			if (insideState().curr) {
				auto parent = dynamic_cast<Container *>(getParent());
				parent->getValue() = m_enum;
				parent->setHotNode(this);
				getValue() = e_on;
			}
			else {
				getValue() = e_off;
			}
		}
		break;
	}
	case 0x2: {  // release
		if (m_type == e_radio || m_type == e_slide) {
			auto *parent = dynamic_cast<DuiNode *>(getParent());
			if (parent && parent->insideState().anchor) {  // not to change hidden button
				if (insideState().curr) {              // use parent link pointer
					parent->getValue() = m_enum;
					getValue() = e_on;
				}
				else {
					getValue() = e_off;
				}
			}
		}
		else if (insideState().anchor) {
			auto ival = int32_t(getValue());
			getValue() = ival == e_on ? e_off : e_on;
		}
		break;
	}
	default:  // nothing {
		if (m_type == e_radio || m_type == e_slide) {
			auto *parent = dynamic_cast<DuiNode *>(getParent());
			if (parent) {
				auto ival = int32_t(parent->getValue());
				if (ival < 0) {
					getValue() = e_inactive;
				}
				else {
					getValue() = ival == m_enum ? e_on : e_off;
				}
			}
		}
		break;
	}
}

void Button::doRender()
{
	auto state = int32_t(getValue());

	// main
	DuiNode::doRender();

	auto highlight = state == e_inactive ? e_inactive : e_active;

	// button
	auto *element = state == e_on ? m_onNode : m_offNode;
	if (element) {
		element->setHighlight(highlight);
		element->render();
	}

	// title
	if (m_text) {
		m_text->setHighlight(highlight);
		m_text->render();
	}
}
}  // namespace spu::gs_node::dui
