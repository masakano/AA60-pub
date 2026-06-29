//
// DistanceField :
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
class DistanceField : public App {
public:
	int32_t width() const override { return 512; }
	int32_t height() const override { return 512; }

	FTText m_text;
	Vec4f m_color = {1.0, 1.0, 1.0, 1.0};

	Vec3f m_center = {0.0, 0.0, 0.0};
	float m_scale = 1.0;
	Mat4f m_localtext;

	SpuPad* m_curr = nullptr;
	SpuPad m_prev;

	void init() override
	{
		const char* filename = "assets/Vera.ttf";
		const int fontsize = 72;
		const FT_Fixed fontweight = 400;
		const int texsize = 512;
		const char* cache
		        = " !\"#$%&'()*+,-./0123456789:;<=>?"
		          "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
		          "`abcdefghijklmnopqrstuvwxyz{|}~";

		Attrs text_attrs = {
		        {"path", "shaders/distance_field.us"},
		        {"atlas.size", Vec4i(texsize, texsize, 1, 0)},
		};
		m_text.init(text_attrs);

		// FTFont ft_font;
		auto ft_font = m_text.getFontFromFile(filename, fontsize);
		ft_font->m_glyphMode = FTGlyph::e_signed_distance_field;
		if (ft_font->isVariable()) ft_font->setWeight(fontweight << 16);
		ft_font->loadGlyphs(cache);
		m_text.upload();

		int32_t indices[6] = {0, 1, 2, 0, 2, 3};
		auto w = float(width());
		auto h = float(height());
		FTText::Vertex vertices[4] = {
		        {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0},
		        {0, h, 0, 0, 0, 1, 1, 1, 1, 0, 0},
		        {w, h, 0, 1, 0, 1, 1, 1, 1, 0, 0},
		        {w, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0}
                };
		// buffer = new FTText();
		m_text.addVertices(vertices, 4, indices, 6);

		m_bgcolor0 = {0.5, 0.5, 0.5, 1.00};

		spu_graphics_get("pad", &m_curr);
	}

	void doDisplay() override
	{
		if (m_curr->wheel > 0 && m_prev.wheel == 0) {
			auto center_text = Vec4f(
			        m_curr->cursor[0] - m_viewport1.ox, m_curr->cursor[1] - m_viewport1.oy, 0, 1);
			m_center = m_localtext.unitary_inverse() * center_text;
		}
		if (m_curr->wheel != m_prev.wheel) {
			m_scale *= m_curr->wheel > m_prev.wheel ? 1.05 : 1.0 / 1.05;
			const auto u = Mat4f();
			m_localtext = u.trans(-m_center).scale(m_scale).trans(m_center);
		}
		m_prev = *m_curr;
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false) * m_localtext;
		m_text.draw(GL_TRIANGLES);
	}
};
App* createDistanceField() { return new DistanceField(); }
}  // namespace spu
