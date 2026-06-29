//
// Texture :
//
#include <gsys/painter/ft_text.h>

#include "app.h"

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {

class Texture : public App {
	int32_t width() const override { return 512; }
	int32_t height() const override { return 512; }

	FTText m_text;

	void init() override
	{
		Attrs text_attrs = {
		        {"def_render_mode", FTText::e_render_simple},
		        {"atlas.size", Vec4i(512, 512, 1, 0)},
		};
		m_text.init(text_attrs);

		const char* filename = "assets/Vera.ttf";
		const char* cache
		        = " !\"#$%&'()*+,-./0123456789:;<=>?"
		          "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
		          "`abcdefghijklmnopqrstuvwxyz{|}~";
		size_t minsize = 8, maxsize = 27;
		size_t count = maxsize - minsize;

		for (auto i = minsize; i < maxsize; ++i) {
			auto* font = m_text.getFontFromFile(filename, i);
			font->loadGlyphs(cache);
		}

		auto atlas_size = m_text.atlasSize();

		printf("Matched font               : %s\n", filename);
		printf("Number of fonts            : %ld\n", count);
		printf("Number of glyphs per font  : %ld\n", freetype::utf8_strlen(cache));
		printf("Total number of glyphs     : %ld/%ld\n", freetype::utf8_strlen(cache) * count,
		       freetype::utf8_strlen(cache) * count);
		printf("Texture size               : %dx%d\n", atlas_size.x, atlas_size.y);
		printf("Texture occupancy          : %.2f%%\n",
		       100.0 * m_text.atlasUsed() / (float)(atlas_size.x * atlas_size.y));

		m_text.upload();

		const FTText::Vertex vertices[4] = {
		        {0,   0,   0, 0, 1, 0, 0, 0, 1, 0, 0},
		        {0,   512, 0, 0, 0, 0, 0, 0, 1, 0, 0},
		        {512, 512, 0, 1, 0, 0, 0, 0, 1, 0, 0},
		        {512, 0,   0, 1, 1, 0, 0, 0, 1, 0, 0}
                };
		int32_t indices[6] = {0, 1, 2, 0, 2, 3};
		m_text.addVertices(vertices, 4, indices, 6);
	}

	void doDisplay() override
	{
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		m_text.draw(GL_TRIANGLES);
	}
};
App* createTexture() { return new Texture(); }
}  // namespace spu
