//
// FontBasic :
//
#include <gsys/painter/ft_text.h>
#include "app.h"

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {
class FontBasic : public App {
public:
	int32_t width() const override { return 800; }
	int32_t height() const override { return 500; }

	FTText m_text;

	void addText(FTFont &font, const char *text, Vec4f *color, Vec2f *pen)
	{
		size_t i;
		float r = color->r, g = color->g, b = color->b, a = color->a;
		for (i = 0; i < strlen(text); ++i) {
			const auto *glyph = font.loadGlyph(text + i);
			if (glyph != NULL) {
				float kerning = 0.0f;
				if (i > 0) {
					kerning = glyph->getKerning(text + i - 1);
				}
				pen->x += kerning;
				float x0 = (int)(pen->x + glyph->offset_x);
				float y0 = (int)(pen->y + glyph->offset_y);
				float x1 = (int)(x0 + glyph->width);
				float y1 = (int)(y0 - glyph->height);
				float s0 = glyph->s0;
				float t0 = glyph->t0;
				float s1 = glyph->s1;
				float t1 = glyph->t1;
				int32_t indices[6] = {0, 1, 2, 0, 2, 3};
				FTText::Vertex vertices[4] = {
				        {x0, y0, 0, s0, t0, r, g, b, a, 0, 0},
				        {x0, y1, 0, s0, t1, r, g, b, a, 0, 0},
				        {x1, y1, 0, s1, t1, r, g, b, a, 0, 0},
				        {x1, y0, 0, s1, t0, r, g, b, a, 0, 0}
                                };
				m_text.addVertices(vertices, 4, indices, 6);
				pen->x += glyph->advance_x;
			}
		}
	}

	void init() override
	{
		Attrs text_attrs = {
		        {"def_render_mode", FTText::e_render_simple},
		        {"atlas.size", Vec4i(512, 512, 1, 0)},
		};
		m_text.init(text_attrs);

		const char *filename = "assets/Vera.ttf";
		const char *text = "A Quick Brown Fox Jumps Over The Lazy Dog 0123456789";

		Vec2f pen = {5, 400};
		Vec4f black = {0, 0, 0, 1};
		for (auto i = 7; i < 27; ++i) {
			auto *font = m_text.getFontFromFile(filename, i);
			pen.x = 5;
			pen.y -= font->m_height;
			font->loadGlyphs(text);
			addText(*font, text, &black, &pen);
		}
		m_text.upload();
	}

	void doDisplay() override
	{
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		m_text.draw(GL_TRIANGLES);
	}
};
App *createFontBasic() { return new FontBasic(); }
}  // namespace spu
