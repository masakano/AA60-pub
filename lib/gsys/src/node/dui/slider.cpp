//
// Slider :
//
#include <gsys/node/dui.h>

namespace spu::gs_node::dui {

void Slider::init(const Attrs &attrs)
{
	assert(m_emptybar == nullptr && m_fullbar == nullptr && m_thumb == nullptr);

	auto min_value = attrs.get<float>("min", 0.0);
	auto max_value = attrs.get<float>("max", 100.0);

	m_emptybar = attrs.get<DuiNode *>("emptybar", nullptr);
	m_fullbar = attrs.get<DuiNode *>("fullbar", nullptr);
	m_thumb = attrs.get<DuiNode *>("thumb", nullptr);

	assert(m_emptybar);
	assert(m_fullbar);
	assert(m_thumb);

	DuiNode::init(attrs);
	m_valueRange = {min_value, max_value};

	// setValueRange({min_value, max_value});
	add(m_thumb);
	add(m_emptybar);
	add(m_fullbar);
	squash();
}

void Slider::updateBar(float value)
{
	if (!std::isnan(value)) {
		const auto rate = std::max(epsilon(), (value - m_valueRange.p0) / m_valueRange.span());
		const auto &range = getARange();
		const auto target = lerp(range.p0.x, range.p1.x, rate);
		auto &fullbar_range = m_fullbar->getARange();

		fullbar_range.p1.x = target;

		const auto &thumb_range = m_thumb->getARange();
		const auto to = Vec2f(target, range.center().y);
		const auto from = Vec2f(thumb_range.center());
		m_thumb->move(to - from);
	}
}

void Slider::setColor(const Vec4f &color, uint32_t mask)
{
	DuiNode::setColor(color, mask);
	m_fullbar->setColor(color, mask & 0xf000);  // alpha only
	m_emptybar->setColor(color, mask & 0xf000);
	m_thumb->setColor(color, mask);
}

void Slider::setValue(float value)
{
	m_curr = std::clamp(value, m_valueRange.p0, m_valueRange.p1);
	getValue() = m_curr;
}

void Slider::update()
{
	DuiNode::update();

	if (!std::isnan(float(getValue()))) {
		// if (ms_gesture.L().stat == SpuGesture::e_drag) {
		if (ms_gesture.prev().mouse_L || ms_gesture.curr().mouse_L) {
			if (m_thumb->insideState().anchor) {
				auto range = getARange();
				auto span = range.span();
				auto rate = (ms_gesture.curr().cursor[0] - range.p0.x) / span.x;

				setValue(lerp(m_valueRange.p0, m_valueRange.p1, rate));
			}
		}
	}
}

void Slider::doRender()
{
	// if (ms_gesture.L().stat != SpuGesture::e_drag) {
	if (!(ms_gesture.prev().mouse_L && ms_gesture.curr().mouse_L)) {  // not drag
		m_curr = getValue();
	}

	auto is_active = !std::isnan(m_curr);

	if (is_active) {
		m_emptybar->setHighlight(e_none);
		m_emptybar->setHighlight(e_none);
		m_thumb->setHighlight(e_active);
		if (m_prev != m_curr) {
			updateBar(m_curr);
			m_prev = m_curr;
		}
	}
	else {
		m_emptybar->setHighlight(e_inactive);
		m_emptybar->setHighlight(e_inactive);
		m_thumb->setHighlight(e_inactive);
	}

	m_emptybar->render();
	m_fullbar->render();
	m_thumb->render();
}
}  // namespace spu::gs_node::dui
