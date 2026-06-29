//
// Text :
//
#include <gsys/node/dui.h>
#include <gsys/painter/ft_text.h>
#include "writer.h"

namespace spu::gs_node::dui {

Text::Text(const char *name) : DuiNode(name)
{
	m_ftwriter = new gs_painter::FTWriter();
	// m_ftwriter->init(ms_ftatlas, c_fontSize);
	m_ftwriter->init(c_fontSize);
}

Text::~Text() { delete m_ftwriter; }

void Text::init(const Attrs &attrs)
{
	attrs.peek("title", "use 'text' instead");
	auto *text = attrs.get("text", "");
	DuiNode::init(attrs);

	if (*text) {
		const auto c_white = Vec4f(0.9, 0.9, 0.9, 1.0);
		const auto text_color = attrs.get<vec4f_t>("text_color", c_white);
		const auto align_h = attrs.get<int32_t>("align_h", e_left);
		const auto align_v = attrs.get<int32_t>("align_v", e_top);

		setTextColor(text_color);
		setAlignH(align_h);
		setAlignV(align_v);
		setText(text);

		bake();
		alignText();
	}
}

void Text::setTextColor(const Vec4f &color, uint32_t mask)
{
	if (((m_textColor == color).pack() & mask) != mask) {
		m_textColor = color;
		m_isCached = false;
	}
}

void Text::setText(const std::string &text)
{
	setName(text);
	if (m_text != text) {
		m_text = text;
		m_isCached = false;
	}
}

void Text::setAlignH(int32_t align_h) { m_alignH = align_h; }

void Text::setAlignV(int32_t align_v) { m_alignV = align_v; }

void Text::bake()
{
	using FTWriter = gs_painter::FTWriter;
	using FTMarkup = gs_painter::freetype::FTMarkup;

	if (m_isCached) {
		return;
	}
	m_isCached = true;

	struct SubText {
		FTWriter *writer;
		FTMarkup markup;
		Vec4f color;
		std::string line;

		SubText(FTWriter *writer) : writer(writer) {}

		void append()
		{
			markup.foreground_color = color;
			if (!line.empty()) {
				writer->addText(markup, line.c_str());
				line.clear();
			}
		}
		void append(const Vec4f &color)
		{
			append();
			this->color = color;
		}
		void append(const FTWriter::Style &style)
		{
			append();
			this->markup = writer->getMarkup(style);
		}
	} subtext(m_ftwriter);

	m_ftwriter->clear();
	m_ftwriter->getPen().x = 0;  // start with lef upper corner
	m_ftwriter->getPen().y = 0;

	subtext.append(FTWriter::e_normal);
	subtext.append(m_textColor);

	for (auto code: m_text) {
		switch (code) {
		case e_white: subtext.append(Vec4f(1.0, 1.0, 1.0, 1.0)); break;
		case e_gray: subtext.append(Vec4f(0.6, 0.6, 0.6, 1.0)); break;
		case e_black: subtext.append(Vec4f(0.0, 0.0, 0.0, 1.0)); break;
		case e_red: subtext.append(Vec4f(1.0, 0.2, 0.2, 1.0)); break;
		case e_green: subtext.append(Vec4f(0.1, 1.0, 0.1, 1.0)); break;
		case e_blue: subtext.append(Vec4f(0.2, 0.2, 1.0, 1.0)); break;
		case e_preset: subtext.append(m_textColor); break;
		case e_normal: subtext.append(FTWriter::e_normal); break;
		case e_bold: subtext.append(FTWriter::e_bold); break;
#if 0			
		case '\n':
		case '\r':
			subtext.append();
			break;
#endif
		default: subtext.line += code; break;
		}
	}
	subtext.append();
	m_ftwriter->flush(FTWriter::e_align_left);
	m_textRange = m_ftwriter->getRange();
}

Range2f Text::alignText()
{
	auto &range = getARange();
	auto text_range = m_textRange;

	// expand to lower-left
	{
		auto from = upperLeftOf(text_range);
		auto to = upperLeftOf(range);
		text_range = text_range + (to - from);
		range.expand(text_range);
	}

	// align
	{
		auto from = Vec2f(0);
		auto to = Vec2f(0);

		switch (m_alignH) {
		case e_left:
			from.x = text_range.p0.x;
			to.x = range.p0.x;
			break;
		case e_center:
			from.x = text_range.center().x;
			to.x = range.center().x;
			break;
		case e_right:
			from.x = text_range.p1.x;
			to.x = range.p1.x;
			break;
		}

		switch (m_alignV) {
		case e_top:
			from.y = text_range.p1.y;
			to.y = range.p1.y;
			break;
		case e_center:
			from.y = text_range.center().y;
			to.y = range.center().y;
			break;
		case e_bottom:
			from.y = text_range.p0.y;
			to.y = range.p0.y;
			break;
		}
		text_range = text_range + (to - from);
	}
	return text_range;
}

void Text::doRender()
{
	DuiNode::doRender();

	if (m_text.empty()) {
		return;
	}

	if (!m_isCached) {
		bake();
	}

	alignText();
	auto range = getARange();

	Mat4f trans = Mat4f().trans({range.p0.x, range.p1.y, 0});
	m_ftwriter->u_color = Vec4f(highlightRate(2.0));

	m_ftwriter->u_textscreen = GsCanvas::getCurrent()->viewscreen() * trans;
	m_ftwriter->render();
}

}  // namespace spu::gs_node::dui
