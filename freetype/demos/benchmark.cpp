//
// Benchmark :
//
#include <gsys/painter/ft_text.h>
#include "app.h"

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {
class Benchmark : public App {
public:
	int32_t width() const override { return 800; }
	int32_t height() const override { return 600; }

	FTFont *m_font = nullptr;  // not owner
	FTText m_text;

	int32_t m_lineCount = 42;

	const char *c_text
	        = "A Quick Brown Fox Jumps Over The Lazy Dog 0123456789 "
	          "A Quick Brown Fox Jumps Over The Lazy Dog 0123456789 ";

	void addText(const char *text, Vec4f *color, Vec2f *pen)
	{
		float r = color->r, g = color->g, b = color->b, a = color->a;
		for (auto i = 0u; i < strlen(text); ++i) {
			const auto *glyph = m_font->loadGlyph(text + i);
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

				int32_t indices[] = {0, 1, 2, 0, 2, 3};
				FTText::Vertex vertices[] = {
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
		Vec2f pen = {0, 0};
		Vec4f color = {0, 0, 0, 1};

		Attrs text_attrs = {
		        {"def_render_mode", FTText::e_render_simple},
		        {"atlas.size", Vec4i(512, 512, 1, 0)},
		};
		m_text.init(text_attrs);
		m_font = m_text.getFontFromFile("assets/VeraMono.ttf", 12);

		pen.y = -m_font->m_descender;
		for (auto i = 0u; i < size_t(m_lineCount); ++i) {
			pen.x = 10.0;
			addText(c_text, &color, &pen);
			pen.y += m_font->m_height - m_font->m_linegap;
		}
		m_text.upload();
	}

	void doDisplay() override
	{
		static int frame = 0;
		static int count = 0;
		static double time;

		if (count == 0 && frame == 0) {
			printf("Computing FPS with text generation and rendering at each frame...\n");
			printf("Number of glyphs: %ld\n", strlen(c_text) * m_lineCount);
		}

		frame++;
		time = double(get_microsec()) / 1000000;

		if (time > 2.5) {
			printf("FPS : %.2f (%d frames in %.2f second, %.1f glyph/second)\n", frame / time,
			       frame, time, frame / time * strlen(c_text) * m_lineCount);
			frame = 0;
			++count;
			if (count == 5) {
				printf("\nComputing FPS with text rendering at each frame...\n");
				printf("Number of glyphs: %ld\n", strlen(c_text) * m_lineCount);
			}
		}
		if (count < 5) {
			Vec4f color = {0, 0, 0, 1};
			Vec2f pen = {0, 0};
			m_text.clear();

			pen.y = -m_font->m_descender;
			for (auto i = 0u; i < size_t(m_lineCount); ++i) {
				pen.x = 10.0;
				addText(c_text, &color, &pen);
				pen.y += m_font->m_height - m_font->m_linegap;
			}
		}
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		m_text.draw(GL_TRIANGLES);
	}
};
App *createBenchmark() { return new Benchmark(); }
}  // namespace spu
