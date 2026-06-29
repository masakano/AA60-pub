//
// Cartoon :
//
#include <gsys/painter/ft_text.h>
#include "app.h"

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {
class Cartoon : public App {
public:
	int32_t width() const override { return 850; }
	int32_t height() const override { return 200; }

	FTFont* m_font = nullptr;  // not owner
	FTText m_text;

	void addText(const char* text, Vec2f pen, const Vec4f& fg_color_1, const Vec4f& fg_color_2)
	{
		for (auto i = 0u; i < strlen(text); ++i) {
			const auto* glyph = m_font->loadGlyph(text + i);
			auto kerning = 0.0f;
			if (i > 0) {
				kerning = glyph->getKerning(text + i - 1);
			}
			pen.x += kerning;

			/* Actual glyph */
			float x0 = (pen.x + glyph->offset_x);
			float y0 = (int)(pen.y + glyph->offset_y);
			float x1 = (x0 + glyph->width);
			float y1 = (int)(y0 - glyph->height);
			float s0 = glyph->s0;
			float t0 = glyph->t0;
			float s1 = glyph->s1;
			float t1 = glyph->t1;

			int32_t indices[] = {0, 1, 2, 0, 2, 3};

			FTText::Vertex vertices[] = {
			        {x0, y0, 0, s0, t0, fg_color_1.r, fg_color_1.g, fg_color_1.b, fg_color_1.a, 0,
			         0},
			        {x0, y1, 0, s0, t1, fg_color_2.r, fg_color_2.g, fg_color_2.b, fg_color_2.a, 0,
			         0},
			        {x1, y1, 0, s1, t1, fg_color_2.r, fg_color_2.g, fg_color_2.b, fg_color_2.a, 0,
			         0},
			        {x1, y0, 0, s1, t0, fg_color_1.r, fg_color_1.g, fg_color_1.b, fg_color_1.a, 0,
			         0},
			};
			m_text.addVertices(vertices, 4, indices, 6);
			pen.x += glyph->advance_x;
		}
	}

	void init() override
	{
		Attrs text_attrs = {
		        {"def_render_mode", FTText::e_render_simple},
		        {"atlas.size", Vec4i(1024, 1024, 1, 0)},
		};
		m_text.init(text_attrs);
		m_font = m_text.getFontFromFile("assets/LuckiestGuy.ttf", 128);

		Vec2f pen = {50, 50};
		Vec4f black = {0.0, 0.0, 0.0, 1.0};
		Vec4f yellow = {1.0, 1.0, 0.0, 1.0};
		Vec4f orange1 = {1.0, 0.9, 0.0, 1.0};
		Vec4f orange2 = {1.0, 0.6, 0.0, 1.0};

		m_font->m_glyphMode = FTGlyph::e_outline_positive;
		m_font->m_outlineThickness = 7;
		addText("Freetype GL", pen, black, black);

		m_font->m_glyphMode = FTGlyph::e_outline_positive;
		m_font->m_outlineThickness = 5;
		addText("Freetype GL", pen, yellow, yellow);

		m_font->m_glyphMode = FTGlyph::e_outline_edge;
		m_font->m_outlineThickness = 3;
		addText("Freetype GL", pen, black, black);

		m_font->m_glyphMode = FTGlyph::e_normal;
		m_font->m_outlineThickness = 0;
		addText("Freetype GL", pen, orange1, orange2);
		m_text.upload();
	}

	void doDisplay() override
	{
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		m_text.draw(GL_TRIANGLES);
	}
};
App* createCartoon() { return new Cartoon(); }
}  // namespace spu
