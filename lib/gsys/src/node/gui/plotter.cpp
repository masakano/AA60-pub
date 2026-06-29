//
// Base :
//
#include <gsys/node/gui/plotter.h>

namespace spu::gs_node::gui {

void Plotter::init(const Attrs &attrs)
{
	Base::init(attrs);

	auto *desc = attrs.get<Desc *>("desc", nullptr);
	auto *name = attrs.get<const char *>("name", nullptr);
	auto *parent = attrs.get<Base *>("parent", nullptr);
	auto columns = getColumns(name, parent);

	assert(desc);
	auto bar_offset = 0.0f;
	// text
	{
		for (auto i = 0u; i < desc->items.size(); i++) {
			PlotterState s;
			s.text = std::string(desc->items[i]) + "\n";
			s.value_ptr = desc->value_ptrs[i];
			m_states.push_back(s);
			bar_offset = std::max(bar_offset, float(s.text.length()));
		}
		bakeTextInternal<PlotterState, float>(m_states);
		m_barLength = columns - bar_offset - 2;

		getARange().p0.y -= 8.0;  // expand : ad-hoc
	}

	// bar
	{
		auto &graphic = newGraphic("plotter");
		auto &vertices = graphic.vertices;
		auto y = -c_charheight;  // skip header
		for (auto iy = 0u; iy < m_states.size(); iy++) {
			auto x = bar_offset;
			for (auto ix = 0; ix < m_barLength; ix++) {
				gs_painter::Sprite::Vertex b;
				b.p.x = x;
				b.p.y = y;
				b.s = {-0.2, -0.90, +0.2, +0.90};
				b.c = Vec4f(1.0f);
				vertices.push_back(b);
				x += c_charwidth;
			}
			y -= c_charheight;
		}
		graphic.drawcall.flags.depth_test = false;
		graphic.drawcall.flags.blend = true;
	}
	Base::update();

	// experimental
	m_line.init(Attrs());

	// queueu
	for (auto &s: m_states) {
		s.queue.resize(128, 0);
	}
}

void Plotter::update()
{
	auto &graphic = getGraphic("plotter");
	auto *b = graphic.vertices.data();
	for (auto &s: m_states) {
		auto value = *s.value_ptr / m_maxValue;
		auto ivalue = int32_t(value * m_barLength + 0.5);
		for (auto ix = 0; ix < m_barLength; ix++) {
			b->c = Vec4f(ix < ivalue ? 0.50 : 0.25);
			b++;
		}
	}
	Base::update();  // experimental
}

void Plotter::doRender()
{
	Base::doRender();

	// experimental
	for (auto &s: m_states) {
		s.queue.push_back(*s.value_ptr);
		s.queue.pop_front();
		auto max_value = *max_element(begin(s.queue), end(s.queue));
		m_maxValue = std::max(m_maxValue * 0.99999f, max_value);
	}

	// experimental
	auto current = GsCanvas::getCurrent();
	auto viewscreen_save = current->viewscreen();
	current->getViewscreen() = Mat4f::texcscreen();
	m_line.begin();
	for (auto &s: m_states) {
		for (auto i = 0u; i < s.queue.size() - 1; i++) {
			Segment3f seg;
			auto x0 = float(i) / s.queue.size();
			auto y0 = *(s.queue.begin() + i);

			auto x1 = float(i + 1) / s.queue.size();
			auto y1 = *(s.queue.begin() + i + 1);

			y0 /= m_maxValue * 2.0f;
			y1 /= m_maxValue * 2.0f;

			seg.p0 = Vec3f(x0, y0, 0);
			seg.p1 = Vec3f(x1, y1, 0);
			m_line.addPrim(seg);
		}
	}
	m_line.end();
	// m_line.renderMode(GL_LINES);
	m_line.draw(GL_LINES);
	current->getViewscreen() = viewscreen_save;
}
}  // namespace spu::gs_node::gui
