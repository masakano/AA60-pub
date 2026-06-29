//
// FTWriter :
//
#pragma once
#include <gsys/painter/ft_text.h>

namespace spu::gs_painter {

class FTWriter : public FTText {
public:
	enum {
		e_normal = 0,
		e_highlight,
		e_reverse,
		e_overline,
		e_underline,
		e_small,
		e_big,
		e_bold,
		e_italic,
		e_max,
	};

	void init()
	{
		Attrs painter_attrs = {
		        {"atlas.size", Vec4i(512, 512, 1, 0)},
		};
		FTText::init(painter_attrs);

		const Vec4f black = {0.0, 0.0, 0.0, 1.0};
		const Vec4f white = {1.0, 1.0, 1.0, 1.0};
		const Vec4f yellow = {1.0, 1.0, 0.0, 1.0};
		const Vec4f grey = {0.5, 0.5, 0.5, 1.0};
		const Vec4f none = {1.0, 1.0, 1.0, 0.0};

		const char *f_normal = "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf";
		const char *f_bold = "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf";
		const char *f_italic = "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Italic.ttf";

		m_markups[e_normal] = {
		        .family = f_normal,
		        .size = 24.0,
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
		m_markups[e_highlight] = m_markups[e_normal];
		m_markups[e_highlight].background_color = grey;

		m_markups[e_reverse] = m_markups[e_normal];
		m_markups[e_reverse].foreground_color = black;
		m_markups[e_reverse].background_color = white;
		m_markups[e_reverse].gamma = 1.0;

		m_markups[e_overline] = m_markups[e_normal];
		m_markups[e_overline].overline = 1;

		m_markups[e_underline] = m_markups[e_normal];
		m_markups[e_underline].underline = 1;

		m_markups[e_small] = m_markups[e_normal];
		m_markups[e_small].size = 12.0;

		m_markups[e_big] = m_markups[e_normal];
		m_markups[e_big].size = 48.0;
		m_markups[e_big].italic = 1;
		m_markups[e_big].foreground_color = yellow;

		m_markups[e_bold] = m_markups[e_normal];
		m_markups[e_bold].bold = 1;
		m_markups[e_bold].family = f_bold;

		m_markups[e_italic] = m_markups[e_normal];
		m_markups[e_italic].italic = 1;
		m_markups[e_italic].family = f_italic;
	}

	void addText(uint32_t style, const char *text) { FTText::addText(m_markups[style], text, 0); }

	void flush(FTText::Align alignment)
	{
		upload();
		align(alignment);
	}

	void render()
	{
		SpuScopedRenderstate renderstate(true);
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
		renderstate.use();
		FTText::draw(GL_TRIANGLES);
	}

	freetype::FTMarkup &getMarkup(uint32_t style) { return m_markups[style]; }

private:
	SpuShader m_shader;
	freetype::FTMarkup m_markups[e_max];
};
}  // namespace spu::gs_painter
