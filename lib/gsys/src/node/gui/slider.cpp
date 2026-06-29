//
// Slider :
//
#include "key_value.h"
#include <gsys/node/gui/slider.h>

namespace spu::gs_node::gui {

namespace {
const Vec4f c_barcolor = {0.25, 0.25, 1.0, 0.5};
}  // namespace

void Slider::save(File &file) const
{
	KeyValue kv(this);

	for (auto lineno = 0u; lineno < m_states.size(); lineno++) {
		auto &s = m_states[lineno];
		kv.add(lineno, "key", s.text);
		kv.add(lineno, "value", *s.value_ptr);
		kv.add(lineno, "min", s.value_minmax.x);
		kv.add(lineno, "max", s.value_minmax.y);
		kv.add(lineno, "power", s.value_power);
	}
	kv.save(file);
}

void Slider::load(File &file)
{
	KeyValue kv(this);
	kv.load(file);

	for (auto lineno = 0u; lineno < m_states.size(); lineno++) {
		std::string line_key;
		kv.get(lineno, "key", line_key);
		for (auto &s: m_states) {
			auto key = peeloff_string(s.text);
			if (key == line_key) {
				kv.get(lineno, "value", *s.value_ptr);
				kv.get(lineno, "min", s.value_minmax.x);
				kv.get(lineno, "max", s.value_minmax.y);
				kv.get(lineno, "power", s.value_power);
			}
		}
	}
}

void Slider::init(const Attrs &attrs)
{
	Base::init(attrs);

	auto *desc = attrs.get<Desc *>("desc", nullptr);
	auto *name = attrs.get<const char *>("name", nullptr);
	auto *parent = attrs.get<Base *>("parent", nullptr);
	auto columns = float(getColumns(name, parent));

	assert(desc);
	auto line_spacing = desc->line_spacing;
	auto bar_offset = 0.0f;
	const auto c_indent = std::string(" ");

	for (auto &item: desc->items) {
		bar_offset = std::max(bar_offset, float((c_indent + item).length()));
		aux_error(
		        columns - bar_offset < 6, "Text '%s' too long. (or title '%s' too short)\n", item,
		        name);
	}
	auto bar_length = columns - bar_offset - 1.5f;
	m_minmax = {
	        bar_offset + 1.0f,
	        bar_offset + bar_length,
	};

	auto &button_graphic = getGraphic(e_button);
	auto &bar_graphic = getGraphic(e_bar);
	auto y = -1.0f;  // skip title
	for (auto &item: desc->items) {
		auto i = &item - &desc->items[0];

		SliderState s;
		gs_painter::Sprite::Vertex button;
		gs_painter::Sprite::Vertex bar;

		s.text = c_indent + item + std::string(line_spacing + 1, '\n');
		s.value_ptr = desc->value_ptrs[i];
		s.value_minmax = desc->value_minmaxs[i];
		s.value_power = desc->value_powers[i];
		assert(s.value_power > 0);

		s.state = e_active;  // start with active
		m_states.push_back(s);

		if (s.value_ptr) {
			auto rate = delerp(*s.value_ptr, s.value_minmax);
			button.p.x = lerp(powf(rate, 1.0f / s.value_power), m_minmax);
			button.p.y = y * c_charheight;
			button.p.z = 0.0f;
		}
		button.t = Vec4f(0, 0, 1, 1);
		button.s = Vec4f(-0.6, -0.6, 0.6, 0.6);
		button.c = getColors()[s.state];
		button_graphic.vertices.push_back(button);

		bar.p.x = bar_offset;
		bar.p.y = y * c_charheight;
		bar.p.z = 0.0f;
		bar.t = Vec4f(0, 0, 1, 1);
		bar.s = Vec4f(0, -0.2, bar_length + 1.0, 0.2);
		bar.c = getColors()[s.state] * c_barcolor;
		bar_graphic.vertices.push_back(bar);

		y -= line_spacing + 1;
	}
	m_states.back().text.pop_back();

	Base::bakeTextInternal<SliderState, float>(m_states);
	Base::update();
}
void Slider::changeState(const std::vector<const void *> &ptrs, const hash32_t &state)
{
	Base::changeStateInternal<SliderState, float>(m_states, ptrs, state);
}

void Slider::update()
{
	auto anchor_cursor = getCursor(getGesture()->anchorR());
	if (!isInside(anchor_cursor)) return;

	auto *gesture = getGesture();
	auto cursor = getCursor(gesture->curr().cursor);
	auto &button_graphic = getGraphic(e_button);
	auto &bar_graphic = getGraphic(e_bar);

	for (auto i = 0u; i < m_states.size(); i++) {
		auto &s = m_states[i];
		auto &b = button_graphic.vertices[i];
		auto range = Range3f(b.p + Vec3f(b.s.x, b.s.y, 0), b.p + Vec3f(b.s.z, b.s.w, 0));

		if (s.state == e_disabled) {
			// do nothing
		}
		else if (s.value_ptr == nullptr) {
			s.state = e_disabled;
		}
		else if (range.inside(cursor)) {
			s.state = e_featured;
		}
		// else if (gesture->L().stat == 0) {
		else if (!gesture->curr().mouse_L) {
			s.state = e_active;
		}

		// if (s.state == e_featured && gesture->L().stat == SpuGesture::e_drag) {
		if (s.state == e_featured && gesture->prev().mouse_L && gesture->curr().mouse_L) {
			auto rate = delerp(cursor.x, m_minmax);
			b.p.x = lerp(rate, m_minmax);  // with clamp
			*s.value_ptr = lerp(powf(rate, s.value_power), s.value_minmax);
			ms_lastUpdateCount = getSeconds().count();
		}
		else if (s.value_ptr) {
			auto rate = delerp(*s.value_ptr, s.value_minmax);
			b.p.x = lerp(powf(rate, 1.0f / s.value_power), m_minmax);  // with clamp
		}
		b.c = getColors()[s.state];
		bar_graphic.vertices[i].c = b.c * c_barcolor;
	}
	Base::update();
}
float Slider::delerp(float v, const Vec2f &minmax)
{
	return clamp((v - minmax.x) / (minmax.y - minmax.x), 0.0f, 1.0f);
}
float Slider::lerp(float r, const Vec2f &minmax)
{
	return minmax.x + clamp(r, 0.0f, 1.0f) * (minmax.y - minmax.x);
}
}  // namespace spu::gs_node::gui
