//
// Gamma :
//
#include <gsys/painter/ft_text.h>
#include "app.h"

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {

class Gamma : public App {
public:
	int32_t width() const override { return 512; }
	int32_t height() const override { return 512; }

	// FontAtlas texture_font_atlas;
	FTText m_text;
	FTText m_background;
	FTMarkup m_markup;

	void init() override
	{
		Attrs text_attrs = {
		        {"path", "shaders/text.us"},
		        {"atlas.size", Vec4i(512, 512, 1, 0)},
		};
		m_text.init(text_attrs);

		Attrs background_attrs = {
		        {"path",            "shaders/text.us"    },
		        {"def_render_mode", FTText::e_render_fill},
		};
		m_background.init(background_attrs);

		Vec4f white = {1.0, 1.0, 1.0, 1.0};
		Vec4f black = {0.0, 0.0, 0.0, 1.0};
		Vec4f none = {1.0, 1.0, 1.0, 0.0};

		m_markup.family = "assets/Vera.ttf";
		m_markup.size = 15.0;
		m_markup.bold = 0;
		m_markup.italic = 0;
		m_markup.spacing = 0.0;
		m_markup.gamma = 1.0;
		m_markup.foreground_color = white;
		m_markup.background_color = none;
		m_markup.underline = 0;
		m_markup.underline_color = none;
		m_markup.overline = 0;
		m_markup.overline_color = none;
		m_markup.strikethrough = 0;
		m_markup.strikethrough_color = none;
		// m_markup.font = m_text.getFontFromMarkup(m_markup);

		const char *text = "A Quick Brown Fox Jumps Over The Lazy Dog 0123456789\n";
		m_text.getPen() = {32, 508};
		for (auto i = 0; i < 14; ++i) {
			m_markup.gamma = 0.75 + 1.5 * i * (1.0 / 14);
			m_text.addText(m_markup, text, 0);
		}

		m_text.getPen() = {32, 252};
		m_markup.foreground_color = black;
		for (auto i = 0; i < 14; ++i) {
			m_markup.gamma = 0.75 + 1.5 * i * (1.0 / 14);
			m_text.addText(m_markup, text, 0);
		}
		m_text.upload();

		FTText::Vertex vertices[4 * 2] = {
		        {0,   0,   0, 0, 0, 1, 1, 1, 1, 0, 0},
                        {0,   256, 0, 0, 0, 1, 1, 1, 1, 0, 0},
		        {512, 256, 0, 0, 0, 1, 1, 1, 1, 0, 0},
                        {512, 0,   0, 0, 0, 1, 1, 1, 1, 0, 0},
		        {0,   256, 0, 0, 0, 0, 0, 0, 1, 0, 0},
                        {0,   512, 0, 0, 0, 0, 0, 0, 1, 0, 0},
		        {512, 512, 0, 0, 0, 0, 0, 0, 1, 0, 0},
                        {512, 256, 0, 0, 0, 0, 0, 0, 1, 0, 0}
                };
		int32_t indices[4 * 3] = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};
		m_background.addVertices(vertices, 8, indices, 12);
	}

	void doDisplay() override
	{
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		m_background.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);

		m_background.draw(GL_TRIANGLES);
		m_text.draw(GL_TRIANGLES);
	}
};
App *createGamma() { return new Gamma(); }
}  // namespace spu
