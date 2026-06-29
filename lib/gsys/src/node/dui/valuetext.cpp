//
// ValueText :
//
#include <gsys/node/dui.h>

namespace spu::gs_node::dui {

void ValueText::init(const Attrs &attrs)
{
	Text::init(attrs);
	m_format = attrs.get("format", "%.1f");
	m_value = new Text(attrs.select("value."));

	assert(alignH() == e_left && m_value->alignH() == e_right);
	replaceChildren({m_value});
}

void ValueText::doRender()
{
	if (float(getValue()) != m_prevValue) {  // memory compare
		std::stringstream str;
		auto fval = float(getValue());
		if (std::isnan(fval)) {
			setHighlight(e_inactive);
			m_value->setHighlight(e_inactive);
		}
		else {
			setHighlight(e_none);
			m_value->setHighlight(e_none);
			m_value->setText(string_printf(m_format.c_str(), float(getValue())));
		}
		m_prevValue = getValue();
	}

	{
		Text::doRender();
	}
	{
		auto text_range = getARange();
		auto value_range = m_value->getARange();
		auto from = rightOf(value_range);
		auto to = rightOf(text_range);

		m_value->move(to - from);
		m_value->render();
	}
}
}  // namespace spu::gs_node::dui
