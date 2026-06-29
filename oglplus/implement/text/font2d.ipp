
#include <stdexcept>
#define STB_TRUETYPE_IMPLEMENTATION
#include <external/stb/stb_truetype.h>

//
// patch
//
void stbtt_MakeGlyphBitmapSubpixel2(
        const stbtt_fontinfo *info, unsigned char *output, int32_t out_w, int32_t out_h, int32_t out_stride,
        float scale_x, float scale_y, float shift_x, float shift_y, int32_t glyph)
{
	int32_t ix0 = 0;
	int32_t iy0 = 0;
	stbtt_vertex *vertices;
	int32_t num_verts = stbtt_GetGlyphShape(info, glyph, &vertices);
	stbtt__bitmap gbm;

	// patch
	{
		int32_t x1 = 0;
		int32_t y1 = 0;
		stbtt_GetGlyphBox(info, glyph, &ix0, &iy0, &x1, &y1);

		ix0 = STBTT_ifloor(ix0 * scale_x + shift_x);
		iy0 = -STBTT_iceil(y1 * scale_y + shift_y);
		// iy0 = STBTT_ifloor(-y1 * scale_y + shift_y); // original

		int32_t vsubsample = out_h < 8 ? 15 : 5;
		shift_y /= vsubsample;  // patch
	}

	gbm.pixels = output;
	gbm.w = out_w;
	gbm.h = out_h;
	gbm.stride = out_stride;

	if (gbm.w && gbm.h) {
		stbtt_Rasterize(
		        &gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1,
		        info->userdata);
	}

	STBTT_free(vertices, info->userdata);
}

namespace spu::oglplus::text {

inline void Font2D::load_font(const uint8_t *ttf_buffer)
{
	if (::stbtt_InitFont(&m_font, ttf_buffer, ::stbtt_GetFontOffsetForIndex(ttf_buffer, 0)) == 0) {
		throw std::runtime_error("Error loading true type font");
	}
}

inline std::vector<uint8_t> Font2D::load_ttf(const std::string &path)
{
	return read_from_file<std::vector<uint8_t>>(path);
}

inline Font2D::Layout Font2D::MakeLayout(const char *code_points, const std::size_t cp_n) const
{
	Layout layout(cp_n, Glyph(this->m_font));
	for (std::size_t cp = 0; cp != cp_n; ++cp) {
		layout[cp].init_index(code_points + cp * sizeof(char32_t));
	}
	return layout;
}

inline float Font2D::Width(std::size_t size_in_pixels, const Layout &layout) const
{
	float scale = ScaleForPixelHeight(float(size_in_pixels));
	float width = 0.0F;
	for (auto i = layout.begin(), p = i, e = layout.end(); i != e; p = i, ++i) {
		if (p != i) {
			width += KernAdvance(*p, *i);
		}
		width += i->width();
	}
	return width * scale;
}

inline void Font2D::render(
        std::size_t size_in_pixels, const Font2D::Layout &layout, uint8_t *buffer_start,
        const std::size_t buffer_width, const std::size_t buffer_height, const int32_t xposition,
        const int32_t yposition) const
{
	if (int(buffer_height) < yposition) {
		return;
	}
	if (int(size_in_pixels) <= -yposition) {
		return;
	}

	std::vector<uint8_t> tmp_buffer;
	auto tmp_height = int(size_in_pixels);
	int32_t tmp_width = 0;

	float scale = ScaleForPixelHeight(float(size_in_pixels));

	float xoffset = 0.0F;
	for (auto i = layout.begin(), p = i, e = layout.end(); i != e; ++i) {
		const int32_t xo = int(std::floor(xoffset)) + xposition;
		const float advance = i->width() * scale;

		auto width_in_pixels = int(std::ceil(advance));

		if (xo >= int(buffer_width)) {
			break;
		}
		if (xo + width_in_pixels < 0) {
			xoffset += advance;
			continue;
		}

		if (tmp_width < width_in_pixels) {
			tmp_width = width_in_pixels;
			tmp_buffer.resize(tmp_width * tmp_height);
		}
		std::fill(tmp_buffer.begin(), tmp_buffer.end(), 0x00);

		if (p != i) {
			xoffset += KernAdvance(*p, *i) * scale;
		}
		const float xshift = xoffset - std::floor(xoffset);
		int32_t x0;
		int32_t y0;
		int32_t x1;
		int32_t y1;
		i->GetBitmapBoxSubpixel(scale, scale, xshift, 0, x0, y0, x1, y1);
		const float yshift = std::floor((i->ascent() * scale + y0));

		::stbtt_MakeGlyphBitmapSubpixel2(
		        &m_font, tmp_buffer.data(), tmp_width, tmp_height, tmp_width, scale, scale, xshift,
		        yshift, i->m_index);

		const int32_t yo = yposition;

		int32_t gb = xo < 0 ? -xo : 0;
		auto gw = int(gb + 1 + (x1 - x0));
		if (gw > tmp_width) {
			gw = tmp_width;
		}
		if (gw > int(buffer_width - xo)) {
			gw = int(buffer_width - xo);
		}
		int32_t gy = (std::floor(yshift));
		if (gy < -yo) {
			gy = -yo;
		}
		int32_t gh = tmp_height;
		if (gh > int(buffer_height - yo)) {
			gh = int(buffer_height - yo);
		}

		while (gy < gh) {
			int32_t gx = gb;
			while (gx < gw) {
				int32_t si = gy * tmp_width + gx;
				uint32_t src = tmp_buffer[si];
				if (src != 0) {
					int32_t di = (gy + yo) * buffer_width + gx + xo + x0;
					uint32_t dst = buffer_start[di] + src;

					if (dst > 0xFF) {
						dst = 0xFF;
					}
					buffer_start[di] = dst & 0xFF;
				}
				++gx;
			}
			++gy;
		}

		p = i;
		xoffset += advance;
	}
}

}  // namespace spu::oglplus::text
