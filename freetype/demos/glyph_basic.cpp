//
// FTGlyphBasic :
//
#include <gsys/painter/ft_text.h>
#include "app.h"

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {
class FTGlyphBasic : public App {
public:
	int32_t width() const override { return 600; }
	int32_t height() const override { return 600; }

	FTText m_text;
	FTText m_lines;
	FTText m_points;

	FTFont* m_big = nullptr;    // not owner
	FTFont* m_small = nullptr;  // not owner
	FTFont* m_title = nullptr;  // not owner

	void addText(FTFont& font, const char* text, const Vec4f& color, Vec2f& pen)
	{
		for (auto i = 0u; i < strlen(text); ++i) {
			const auto* glyph = font.loadGlyph(text + i);
			if (glyph != NULL) {
				float kerning = 0.0f;
				if (i > 0) {
					kerning = glyph->getKerning(text + i - 1);
				}
				pen.x += kerning;
				float x0 = (int)(pen.x + glyph->offset_x);
				float y0 = (int)(pen.y + glyph->offset_y);
				float x1 = (int)(x0 + glyph->width);
				float y1 = (int)(y0 - glyph->height);
				float s0 = glyph->s0;
				float t0 = glyph->t0;
				float s1 = glyph->s1;
				float t1 = glyph->t1;
				float r = color.r;
				float g = color.g;
				float b = color.b;
				float a = color.a;

				int32_t indices[] = {0, 1, 2, 0, 2, 3};
				FTText::Vertex vertices[] = {
				        {x0, y0, 0, s0, t0, r, g, b, a, 0, 0},
				        {x0, y1, 0, s0, t1, r, g, b, a, 0, 0},
				        {x1, y1, 0, s1, t1, r, g, b, a, 0, 0},
				        {x1, y0, 0, s1, t0, r, g, b, a, 0, 0},
				};
				m_text.addVertices(vertices, 4, indices, 6);
				pen.x += glyph->advance_x;
			}
		}
	}

	FTText::Vertex set(float x, float y, const Vec4f& c)
	{
		return FTText::Vertex(x, y, 0, 0, 0, c.r, c.g, c.b, c.a, 0, 0);
	}

	void init() override
	{
		Vec4f blue = {0, 0, 1, 1};
		Vec4f black = {0, 0, 0, 1};
		Vec2f pen, origin;

		Attrs text_attrs = {
		        {"def_render_mode", FTText::e_render_simple},
		        {"atlas.size", Vec4i(512, 512, 1, 0)},
		};
		m_text.init(text_attrs);

		Attrs lines_attrs = {
		        {"path",            "shaders/text.us"    },
		        {"def_render_mode", FTText::e_render_fill},
		};
		m_lines.init(lines_attrs);

		Attrs points_attrs = {
		        {"path",            "shaders/text.us"    },
		        {"def_render_mode", FTText::e_render_fill},
		};
		m_points.init(points_attrs);

		m_big = m_text.getFontFromFile("assets/Vera.ttf", 400);
		m_small = m_text.getFontFromFile("assets/Vera.ttf", 18);
		m_title = m_text.getFontFromFile("assets/Vera.ttf", 32);

		const auto* glyph = m_big->loadGlyph("g");
		origin.x = width() / 2 - glyph->offset_x - glyph->width / 2;
		origin.y = height() / 2 - glyph->offset_y + glyph->height / 2;
		addText(*m_big, "g", black, origin);

		// title
		pen.x = 50;
		pen.y = 560;
		addText(*m_title, "FTGlyph metrics", black, pen);

		FTText::Vertex vertices[] = {
		        // Baseline
		        set(0.1f * width(), origin.y, black),
		        set(0.9f * width(), origin.y, black),

		        // Top line
		        set(0.1f * width(), origin.y + glyph->offset_y, black),
		        set(0.9f * width(), origin.y + glyph->offset_y, black),

		        // Bottom line
		        set(0.1f * width(), origin.y + glyph->offset_y - glyph->height, black),
		        set(0.9f * width(), origin.y + glyph->offset_y - glyph->height, black),

		        // Left line at origin
		        set(float(width() / 2) - glyph->offset_x - glyph->width / 2, 0.1f * height(), black),
		        set(float(width() / 2) - glyph->offset_x - glyph->width / 2, 0.9f * height(), black),

		        // Left line
		        set(float(width() / 2) - glyph->width / 2, 0.3f * height(), black),
		        set(float(width() / 2) - glyph->width / 2, 0.9f * height(), black),

		        // Right line
		        set(float(width() / 2) + glyph->width / 2, 0.3f * height(), black),
		        set(float(width() / 2) + glyph->width / 2, 0.9f * height(), black),

		        // Right line at origin
		        set(float(width() / 2) - glyph->offset_x - glyph->width / 2 + glyph->advance_x,
		            0.1f * height(), black),
		        set(float(width() / 2) - glyph->offset_x - glyph->width / 2 + glyph->advance_x,
		            0.7f * height(), black),

		        // Width
		        set(float(width() / 2) - glyph->width / 2, 0.8f * height(), blue),
		        set(float(width() / 2) + glyph->width / 2, 0.8f * height(), blue),

		        // Advance_x
		        set(float(width() / 2) - glyph->width / 2 - glyph->offset_x, 0.2f * height(), blue),
		        set(float(width() / 2) - glyph->width / 2 - glyph->offset_x + glyph->advance_x,
		            0.2f * height(), blue),

		        // Offset_x
		        set(float(width() / 2) - glyph->width / 2 - glyph->offset_x, 0.85f * height(), blue),
		        set(float(width() / 2) - glyph->width / 2, 0.85f * height(), blue),

		        // Height
		        set(0.3f * width() / 2, origin.y + glyph->offset_y - glyph->height, blue),
		        set(0.3f * width() / 2, origin.y + glyph->offset_y, blue),

		        // Offset y
		        set(0.8f * width(), origin.y + glyph->offset_y, blue),
		        set(0.8f * width(), origin.y, blue),
		};

		int32_t indices[] = {
		        0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
		        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
		};

		m_lines.addVertices(vertices, 26, indices, 26);

		pen.x = width() / 2 - 48;
		pen.y = .2 * height() - 18;
		addText(*m_small, "advance_x", blue, pen);

		pen.x = width() / 2 - 20;
		pen.y = .8 * height() + 3;
		addText(*m_small, "width", blue, pen);

		pen.x = width() / 2 - glyph->width / 2 + 5;
		pen.y = .85 * height() - 8;
		addText(*m_small, "offset_x", blue, pen);

		pen.x = 0.2 * width() / 2 - 30;
		pen.y = origin.y + glyph->offset_y - glyph->height / 2;
		addText(*m_small, "height", blue, pen);

		pen.x = 0.8 * width() + 3;
		pen.y = origin.y + glyph->offset_y / 2 - 6;
		addText(*m_small, "offset_y", blue, pen);

		pen.x = width() / 2 - glyph->offset_x - glyph->width / 2 - 58;
		pen.y = height() / 2 - glyph->offset_y + glyph->height / 2 - 20;
		addText(*m_small, "Origin", black, pen);

		m_text.upload();
		int32_t i = 0;

		FTText::Vertex p
		        = set(width() / 2 - glyph->offset_x - glyph->width / 2,
		              height() / 2 - glyph->offset_y + glyph->height / 2, black);

		// Origin point
		m_points.addVertices(&p, 1, &i, 1);

		// Advance point
		p.x = width() / 2 - glyph->offset_x - glyph->width / 2 + glyph->advance_x;
		p.y = height() / 2 - glyph->offset_y + glyph->height / 2;
		m_points.addVertices(&p, 1, &i, 1);
	}

	void doDisplay() override
	{
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		m_lines.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		m_points.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);

#if 0		
		SpuScopedRenderstate renderstate(true);
		renderstate.flags.program_point_size = false;
		renderstate.flags.point_smooth = true;
		renderstate.point_size = 10.0;
		renderstate.use();
#endif
		m_text.draw(GL_TRIANGLES);
		m_lines.draw(GL_LINES);
		m_points.draw(GL_POINTS);
	}
};
App* createGlyphBasic() { return new FTGlyphBasic(); }
}  // namespace spu
