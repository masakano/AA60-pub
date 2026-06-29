//
// FTWriter :
//
#pragma once
#include <gsys/painter/ft_text.h>

namespace spu::gs_painter {

class FTWriter : public FTText {
public:
	using FTText::init;

	enum Style {
		e_normal = 0,
		e_reverse,
		e_overline,
		e_underline,
		e_bold,
		e_italic,
		e_max,
	};

	void init(float fontsize)
	{
		FTText::init(Attrs());

		const Vec4f black = {0.0, 0.0, 0.0, 1.0};
		const Vec4f white = {1.0, 1.0, 1.0, 1.0};
		// const Vec4f grey = {0.5, 0.5, 0.5, 1.0};
		const Vec4f none = {1.0, 1.0, 1.0, 0.0};
		const char *f_normal = "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf";
		const char *f_bold = "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf";
		const char *f_italic = "/usr/share/fonts/truetype/noto/NotoSans-Italic.ttf";

		m_markups[e_normal] = {
		        .family = f_normal,
		        .size = fontsize,
		        .bold = 0,
		        .italic = 0,
		        .spacing = 0.0,
		        .gamma = 2.,
		        .foreground_color = white,
		        .background_color = none,
		        .outline = 0,
		        .outline_color = none,
		        .underline = 0,
		        .underline_color = white,
		        .overline = 0,
		        .overline_color = white,
		        .strikethrough = 0,
		        .strikethrough_color = white,
		        .font = 0,
		};

		m_markups[e_reverse] = m_markups[e_normal];
		m_markups[e_reverse].foreground_color = black;
		m_markups[e_reverse].background_color = white;
		m_markups[e_reverse].gamma = 1.0;

		m_markups[e_overline] = m_markups[e_normal];
		m_markups[e_overline].overline = 1;

		m_markups[e_underline] = m_markups[e_normal];
		m_markups[e_underline].underline = 1;

		m_markups[e_bold] = m_markups[e_normal];
		m_markups[e_bold].bold = 1;
		m_markups[e_bold].family = f_bold;

		m_markups[e_italic] = m_markups[e_normal];
		m_markups[e_italic].italic = 1;
		m_markups[e_italic].family = f_italic;
	}

	void addText(const freetype::FTMarkup &markup, const char *text) { FTText::addText(markup, text, 0); }

	void flush(FTText::Align alignment)
	{
		upload();
		align(alignment);
		finishLine(false);  // don't forget
	}

	void render() { FTText::draw(GL_TRIANGLES); }
	freetype::FTMarkup &getMarkup(uint32_t style) { return m_markups[style]; }

private:
	freetype::FTMarkup m_markups[e_max];
};
}  // namespace spu::gs_painter
