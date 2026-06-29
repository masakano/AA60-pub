//
// DistanceField2 :
//
#include <gsys/painter/ft_text.h>
#include "app.h"

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {
class DistanceField2 : public App {
public:
	int32_t width() const override { return 800; }
	int32_t height() const override { return 600; }

	double m_totalTime = 0.0;
	FTText m_text;
	// Vec4f m_color = {0.067, 0.333, 0.486, 1.0};

	Vec4f addText(FTFont &font, const char *text, Vec4f *color, Vec2f *pen)
	{
		Vec4f bbox = {0, 0, 0, 0};
		size_t i;
		float r = color->r, g = color->g, b = color->b, a = color->a;
		for (i = 0; i < strlen(text); ++i) {
			// double start_time = glfwGetTime();
			double start_time = double(get_microsec()) / 1000000;
			const auto *glyph = font.loadGlyph(text + i);

			m_totalTime += double(get_microsec()) / 1000000 - start_time;
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

				if (x0 < bbox.x) bbox.x = x0;
				if (y1 < bbox.y) bbox.y = y1;
				if ((x1 - bbox.x) > bbox.sx) bbox.sx = x1 - bbox.x;
				if ((y0 - bbox.y) > bbox.sy) bbox.sy = y0 - bbox.y;
			}
		}
		return bbox;
	}

	void init() override
	{
		Attrs text_attrs = {
		        {"def_render_mode", FTText::e_render_distance_field},
		        {"atlas.size", Vec4i(512, 512, 1, 0)},
		};
		m_text.init(text_attrs);
		m_text.u_color = {0.067, 0.333, 0.486, 1.0};

		const char *filename = "assets/Vera.ttf";
		const char *text = "A Quick Brown Fox Jumps Over The Lazy Dog 0123456789";

		Vec2f pen = {0, 0};
		Vec4f black = {1, 1, 1, 1};

		auto *ft_font = m_text.getFontFromFile(filename, 48);

		ft_font->m_glyphMode = FTGlyph::e_signed_distance_field;
		Vec4f bbox = addText(*ft_font, text, &black, &pen);

		Vec3f t = {-(bbox.x + bbox.sx / 2), -(bbox.y + bbox.sy / 2), 0};
		m_text.transform(Mat4f().trans(t));
		m_text.upload();
	}

	void doDisplay() override
	{
		srand(4);
		for (auto i = 0; i < 40; ++i) {
			float scale = .25 + 4.75 * pow(rand() / (float)(RAND_MAX), 2);
			float angle = 90 * (rand() % 2);
			float x = (.05 + .9 * (rand() / (float)(RAND_MAX))) * width();
			float y = (-.05 + .9 * (rand() / (float)(RAND_MAX))) * height();
			float a = 0.1 + .8 * (pow((1.0 - scale / 5), 2));

			m_text.u_color.a = a;

			auto nodetext = Mat4f().rot("Z", angle).scale(scale).trans({x, y, 0});
			m_text.u_textscreen
			        = Mat4f::projection(0, width(), 0, height(), -1, 1, false) * nodetext;
			m_text.draw(GL_TRIANGLES);
		}
	}
};
App *createDistanceField2() { return new DistanceField2(); }
}  // namespace spu
