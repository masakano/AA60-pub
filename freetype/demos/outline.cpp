//
// Outline :
//
#include <gsys/painter/ft_text.h>
#include "app.h"

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {
class Outline : public App {
public:
	int32_t width() const override { return 600; }
	int32_t height() const override { return 250; }

	FTText m_text;

	void addText(Vec2f* pen, ...)
	{
		FTMarkup* markup;
		char* text;
		va_list args;
		va_start(args, pen);

		do {
			markup = va_arg(args, FTMarkup*);
			if (markup == NULL) {
				break;
			}
			text = va_arg(args, char*);

			size_t i;
			FTFont* font = markup->font;
			float r = markup->foreground_color.r;
			float g = markup->foreground_color.g;
			float b = markup->foreground_color.b;
			float a = markup->foreground_color.a;

			for (i = 0; i < strlen(text); ++i) {
				const auto* glyph = font->loadGlyph(text + i);

				if (glyph != NULL) {
					float kerning = 0.0f;
					if (i > 0) {
						kerning = glyph->getKerning(text + i - 1);
					}
					pen->x += kerning;

					/* Actual glyph */
					float x0 = (pen->x + glyph->offset_x);
					float y0 = (int)(pen->y + glyph->offset_y);
					float x1 = (x0 + glyph->width);
					float y1 = (int)(y0 - glyph->height);
					float s0 = glyph->s0;
					float t0 = glyph->t0;
					float s1 = glyph->s1;
					float t1 = glyph->t1;

					int32_t indices[] = {0, 1, 2, 0, 2, 3};

					FTText::Vertex vertices[] = {
					        {x0, y0, 0, s0, t0, r, g, b, a, 0, 1},
					        {x0, y1, 0, s0, t1, r, g, b, a, 0, 1},
					        {x1, y1, 0, s1, t1, r, g, b, a, 0, 1},
					        {x1, y0, 0, s1, t0, r, g, b, a, 0, 1}
                                        };
					m_text.addVertices(vertices, 4, indices, 6);
					pen->x += glyph->advance_x;
				}
			}
		} while (markup != 0);
		va_end(args);
	}

	void init() override
	{
		Attrs text_attrs = {
		        {"def_render_mode", FTText::e_render_simple},
		        {"atlas.size", Vec4i(512, 512, 1, 0)},
		};
		m_text.init(text_attrs);

		Vec4f white = {1.0, 1.0, 1.0, 1.0};
		Vec4f none = {1.0, 1.0, 1.0, 0.0};

		FTFont font;
		FTMarkup markup;
		markup.family = "assets/Vera.ttf";
		markup.size = 80.0;
		markup.bold = 0;
		markup.italic = 0;
		markup.spacing = 0.0;
		markup.gamma = 1.5;
		markup.foreground_color = white;
		markup.background_color = none;
		markup.underline = 0;
		markup.underline_color = white;
		markup.overline = 0;
		markup.overline_color = white;
		markup.strikethrough = 0;
		markup.strikethrough_color = white;
		markup.font = 0;

		markup.font = &font;
		markup.font = m_text.getFontFromFile("assets/Vera.ttf", markup.size);
		markup.font->m_glyphMode = FTGlyph::e_outline_edge;

		Vec2f pen;
		pen.x = 40;
		pen.y = 190;
		for (auto i = 0; i < 10; ++i) {
			markup.font->m_outlineThickness = 2 * ((i + 1) / 10.0);
			addText(&pen, &markup, "g", NULL);
		}

		pen.x = 40;
		pen.y = 110;
		markup.font->m_glyphMode = FTGlyph::e_outline_positive;
		for (auto i = 0; i < 10; ++i) {
			markup.font->m_outlineThickness = 2 * ((i + 1) / 10.0);
			addText(&pen, &markup, "g", NULL);
		}

		pen.x = 40;
		pen.y = 30;
		markup.font->m_glyphMode = FTGlyph::e_outline_negative;
		for (auto i = 0; i < 10; ++i) {
			markup.font->m_outlineThickness = 1 * ((i + 1) / 10.0);
			addText(&pen, &markup, "g", NULL);
		}
		m_text.upload();

		m_bgcolor0 = {0.40, 0.40, 0.45, 1.00};
	}

	void doDisplay() override
	{
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		m_text.draw(GL_TRIANGLES);
	}
};
App* createOutline() { return new Outline(); }
}  // namespace spu
