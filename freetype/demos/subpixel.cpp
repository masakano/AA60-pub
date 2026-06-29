//
// Subpixel :
//
#include <gsys/painter/ft_text.h>
#include "app.h"

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {
class Subpixel : public App {
public:
	int32_t width() const override { return 260; }
	int32_t height() const override { return 330; }

	// FontAtlas texture_font_atlas;
	FTText m_text;
	FTText m_bounds;

	void init() override
	{
		Attrs text_attrs = {
		        {"path", "shaders/text.us"},
		        {"atlas.size", Vec4i(512, 512, 3, 0)},
		        {"def_render_mode", FTText::e_render_lcd},
		};
		m_text.init(text_attrs);

		Attrs bounds_attrs = {
		        {"path",            "shaders/text.us"    },
		        {"def_render_mode", FTText::e_render_fill},
		};
		m_bounds.init(text_attrs);

		// buffer = new FTText();
		const FTText::Vertex vertices[4 * 2] = {
		        {15,  0,   0, 0, 0, 0, 0, 0, 1, 0, 0},
		        {15,  330, 0, 0, 0, 0, 0, 0, 1, 0, 0},
		        {245, 0,   0, 0, 0, 0, 0, 0, 1, 0, 0},
		        {245, 330, 0, 0, 0, 0, 0, 0, 1, 0, 0}
                };
		int32_t indices[4 * 3] = {0, 1, 2, 3};
		m_bounds.addVertices(vertices, 4, indices, 4);

		Vec4f black = {0.0, 0.0, 0.0, 1.0};

		Vec4f none = {1.0, 1.0, 1.0, 0.0};
		FTMarkup markup;
		markup.family = "assets/Vera.ttf";
		markup.size = 9.0;
		markup.bold = 0;
		markup.italic = 0;
		markup.spacing = 0.0;
		markup.gamma = 1.0;
		markup.foreground_color = black;
		markup.background_color = none;
		markup.underline = 0;
		markup.underline_color = black;
		markup.overline = 0;
		markup.overline_color = black;
		markup.strikethrough = 0;
		markup.strikethrough_color = black;
		markup.font = 0;

		// markup.font = m_text.getFontFromMarkup(markup);

		const char *text = "| A Quick Brown Fox Jumps Over The Lazy Dog\n";

		m_text.getPen() = {20, 320};
		for (auto i = 0; i < 30; ++i) {
			m_text.addText(markup, text, 0);
			m_text.getPen().x += i * 0.1;
		}
		m_text.upload();
	}

	void doDisplay() override
	{
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		m_bounds.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);

		m_text.draw(GL_TRIANGLES);

		m_bounds.getADrawcall().blend_func = {
		        GL_ONE,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_ONE,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
		m_bounds.draw(GL_LINES);
	}
};
App *createSubpixel() { return new Subpixel(); }
}  // namespace spu
