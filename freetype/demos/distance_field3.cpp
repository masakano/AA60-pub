//
// DistanceField3 :
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

#include <ft2build.h>
#include FT_FREETYPE_H

class DistanceField3 : public App {
public:
	int32_t width() const override { return 512; }
	int32_t height() const override { return 512; }

	double m_totalTime = 0.0;
	float m_angle = 0.0;

	FTText m_text;
	FTFont *m_font = nullptr;  // not owner

	// Mitchell Netravali reconstruction filter
	float mitchellNetravali(float x)
	{
		const float b = 1 / 3.0, c = 1 / 3.0;  // Recommended
		// const float B =   1.0, C =   0.0; // Cubic B-spline (smoother results)
		// const float B =   0.0, C = 1/2.0; // Catmull-Rom spline (sharper results)
		x = fabs(x);
		if (x < 1) {
			return ((12 - 9 * b - 6 * c) * x * x * x + (-18 + 12 * b + 6 * c) * x * x + (6 - 2 * b))
			     / 6;
		}
		else if (x < 2) {
			return ((-b - 6 * c) * x * x * x + (6 * b + 30 * c) * x * x + (-12 * b - 48 * c) * x
			        + (8 * b + 24 * c))
			     / 6;
		}
		else {
			return 0;
		}
	}

	float interpolate(float x, float y0, float y1, float y2, float y3)
	{
		auto c0 = mitchellNetravali(x - 1);
		auto c1 = mitchellNetravali(x);
		auto c2 = mitchellNetravali(x + 1);
		auto c3 = mitchellNetravali(x + 2);
		auto r = c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3;
		return min(max(r, 0.0), 1.0);
	}

	int resize(
	        double *src_data, size_t src_width, size_t src_height, double *dst_data, size_t dst_width,
	        size_t dst_height)
	{
		if ((src_width == dst_width) && (src_height == dst_height)) {
			memcpy(dst_data, src_data, src_width * src_height * sizeof(double));
			return 0;
		}
		size_t i, j;
		auto xscale = src_width / (float)dst_width;
		auto yscale = src_height / (float)dst_height;
		for (j = 0; j < dst_height; ++j) {
			for (i = 0; i < dst_width; ++i) {
				auto src_i = (int)floor(i * xscale);
				auto src_j = (int)floor(j * yscale);
				auto i0 = static_cast<size_t>(min(max(0, src_i - 1), int32_t(src_width - 1)));
				auto i1 = static_cast<size_t>(min(max(0, src_i), int32_t(src_width - 1)));
				auto i2 = static_cast<size_t>(min(max(0, src_i + 1), int32_t(src_width - 1)));
				auto i3 = static_cast<size_t>(min(max(0, src_i + 2), int32_t(src_width - 1)));
				auto j0 = static_cast<size_t>(min(max(0, src_j - 1), int32_t(src_height - 1)));
				auto j1 = static_cast<size_t>(min(max(0, src_j), int32_t(src_height - 1)));
				auto j2 = static_cast<size_t>(min(max(0, src_j + 1), int32_t(src_height - 1)));
				auto j3 = static_cast<size_t>(min(max(0, src_j + 2), int32_t(src_height - 1)));
				auto t0 = interpolate(
				        i / (float)dst_width, src_data[j0 * src_width + i0],
				        src_data[j0 * src_width + i1], src_data[j0 * src_width + i2],
				        src_data[j0 * src_width + i3]);
				auto t1 = interpolate(
				        i / (float)dst_width, src_data[j1 * src_width + i0],
				        src_data[j1 * src_width + i1], src_data[j1 * src_width + i2],
				        src_data[j1 * src_width + i3]);
				auto t2 = interpolate(
				        i / (float)dst_width, src_data[j2 * src_width + i0],
				        src_data[j2 * src_width + i1], src_data[j2 * src_width + i2],
				        src_data[j2 * src_width + i3]);
				auto t3 = interpolate(
				        i / (float)dst_width, src_data[j3 * src_width + i0],
				        src_data[j3 * src_width + i1], src_data[j3 * src_width + i2],
				        src_data[j3 * src_width + i3]);
				auto y = interpolate(j / (float)dst_height, t0, t1, t2, t3);
				dst_data[j * dst_width + i] = y;
			}
		}
		return 0;
	}

	FTGlyph *loadGlyph(
	        const char *filename, const char *codepoint, const float highres_size, const float lowres_size,
	        const float padding)
	{
		size_t i, j;
		FT_Library library;
		FT_Face face;

		std::string full_path = File::searchPath(filename);

		FT_Init_FreeType(&library);
		FT_New_Face(library, full_path.c_str(), 0, &face);
		FT_Select_Charmap(face, FT_ENCODING_UNICODE);
		FT_UInt glyph_index = FT_Get_Char_Index(face, freetype::utf8_to_utf32(codepoint));

		// Render glyph at high resolution (highres_size points)
		FT_Set_Char_Size(face, highres_size * 64, 0, 72, 72);
		FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT);
		FT_GlyphSlot slot = face->glyph;
		FT_Bitmap bitmap = slot->bitmap;

		// Allocate high resolution buffer
		size_t highres_width = bitmap.width + 2 * padding * highres_size;
		size_t highres_height = bitmap.rows + 2 * padding * highres_size;
		std::vector<double> highres_data(highres_width * highres_height, 0.0);

		// Copy high resolution bitmap with padding and normalize values
		for (j = 0; j < bitmap.rows; ++j) {
			for (i = 0; i < bitmap.width; ++i) {
				auto x = i + padding;
				auto y = j + padding;
				highres_data[y * highres_width + x]
				        = bitmap.buffer[j * bitmap.width + i] / 255.0;
			}
		}

		// Compute distance map
		auto usec = get_microsec();
		highres_data = freetype::make_distance_mapd(highres_data, highres_width, highres_height);
		auto dusec = get_microsec() - usec;
		m_totalTime += double(dusec) / 1000000;

		size_t lowres_width = round(highres_width * lowres_size / highres_size);
		size_t lowres_height = round(highres_height * lowres_width / (float)highres_width);
		std::vector<double> lowres_data(lowres_width * lowres_height, 0.0);

		// Scale down highres buffer into lowres buffer
		resize(highres_data.data(), highres_width, highres_height, lowres_data.data(), lowres_width,
		       lowres_height);

		// Convert the (double *) lowres buffer into a (uint8_t *) buffer and
		// rescale values between 0 and 255.
		std::vector<uint8_t> data(lowres_width * lowres_height);
		for (j = 0; j < lowres_height; ++j) {
			for (i = 0; i < lowres_width; ++i) {
				double v = lowres_data[j * lowres_width + i];
				data[j * lowres_width + i] = (int)(255 * (1 - v));
			}
		}

		// Compute new glyph information from highres value
		auto ratio = lowres_size / highres_size;
		size_t pitch = lowres_width * sizeof(uint8_t);

		// Create glyph
		FTGlyph *glyph = new FTGlyph();
		glyph->offset_x = (slot->bitmap_left + padding * highres_width) * ratio;
		glyph->offset_y = (slot->bitmap_top + padding * highres_height) * ratio;
		glyph->width = lowres_width;
		glyph->height = lowres_height;
		glyph->codepoint = freetype::utf8_to_utf32(codepoint);

		Recti region = m_text.getAtlasRegion(glyph->width, glyph->height);

		m_text.setAtlasRegion(
		        Recti(region.ox, region.oy, glyph->width, glyph->height), data.data(), pitch);
		auto atlas_size = m_text.atlasSize();
		glyph->s0 = region.ox / (float)atlas_size.x;
		glyph->t0 = region.oy / (float)atlas_size.y;
		glyph->s1 = (region.ox + glyph->width) / (float)atlas_size.x;
		glyph->t1 = (region.oy + glyph->height) / (float)atlas_size.y;

		FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT);
		glyph->advance_x = ratio * face->glyph->advance.x / 64.0;
		glyph->advance_y = ratio * face->glyph->advance.y / 64.0;

		return glyph;
	}

	void init() override
	{
		Attrs text_attrs = {
		        {"def_render_mode", FTText::e_render_distance_field},
		        {"atlas.size", Vec4i(512, 512, 1, 0)},
		};
		m_text.init(text_attrs);
		m_text.u_color = {1.0, 1.0, 1.0, 1.0};

		m_font = m_text.getFontFromFile("assets/Vera.ttf", 32);

		// Generate the glyp at 512 points, compute distance field and scale it
		// back to 32 points
		// Just load another glyph if you want to see difference (draw render a '@')
		const auto *glyph = loadGlyph("assets/Vera.ttf", "@", 512, 64, 0.1);
		// m_font->indexGlyph(*glyph, '@');
		m_font->m_glyphs['@'] = *glyph;

		glyph = m_font->loadGlyph("@");

		int32_t indices[6] = {0, 1, 2, 0, 2, 3};
		FTText::Vertex vertices[4] = {
		        {-.5, -.5, 0, glyph->s0, glyph->t1, 0, 0, 0, 1, 0, 0},
		        {-.5, .5,  0, glyph->s0, glyph->t0, 0, 0, 0, 1, 0, 0},
		        {.5,  .5,  0, glyph->s1, glyph->t0, 0, 0, 0, 1, 0, 0},
		        {.5,  -.5, 0, glyph->s1, glyph->t1, 0, 0, 0, 1, 0, 0}
                };
		// buffer = new FTText();
		m_text.addVertices(vertices, 4, indices, 6);
		m_text.upload();
	}

	void doDisplay() override
	{
		m_angle += 30 * 0.02;

		auto s = .025f + .975f * (1 + cosf(m_angle / 100.0f)) / 2.f;

		auto nodetext = Mat4f().scale({width() * s, width() * s, 1})
		                        .rot("Z", m_angle)
		                        .trans({width() / 2.f, height() / 2.f, 0.f});

		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false) * nodetext;
		m_text.draw(GL_TRIANGLES);
	}
};
App *createDistanceField3() { return new DistanceField3(); }
}  // namespace spu
