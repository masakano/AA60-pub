//
//$<<Header>>$
//

#pragma once

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <ssys/ssys.h>
#include <external/stb/stb_truetype.h>
#include <istream>
// #include <vector>

namespace spu::oglplus::text {

class Font2DGlyph {
private:
	friend class Font2D;

	const ::stbtt_fontinfo *m_font;
	int32_t m_index;

	Font2DGlyph(const ::stbtt_fontinfo &font) : m_font(&font), m_index(0) {}

	void init_index(const char *code_point)
	{
		const auto cp = *reinterpret_cast<const char32_t *>(code_point);
		m_index = ::stbtt_FindGlyphIndex(m_font, int(cp));
	}

	Font2DGlyph(const ::stbtt_fontinfo &font, const char *code_point) : m_font(&font)
	{
		init_index(code_point);
	}

public:
	void GetHMetrics(int32_t &left_bearing, int32_t &width) const
	{
		::stbtt_GetGlyphHMetrics(m_font, m_index, &width, &left_bearing);
	}

	void GetVMetrics(int32_t &ascent, int32_t &descent, int32_t &line_gap) const
	{
		::stbtt_GetFontVMetrics(m_font, &ascent, &descent, &line_gap);
	}

	void GetBitmapBox(float xscale, float yscale, int32_t &x0, int32_t &y0, int32_t &x1, int32_t &y1) const
	{
		::stbtt_GetGlyphBitmapBox(m_font, m_index, xscale, yscale, &x0, &y0, &x1, &y1);
	}

	void GetBitmapBoxSubpixel(
	        float xscale, float yscale, float xoffs, float yoffs, int32_t &x0, int32_t &y0, int32_t &x1,
	        int32_t &y1) const
	{
		//::stbtt_GetGlyphBitmapBoxSubpixel2(
		::stbtt_GetGlyphBitmapBoxSubpixel(
		        m_font, m_index, xscale, yscale, xoffs, yoffs, &x0, &y0, &x1, &y1);
	}

	int32_t leftBearing() const
	{
		auto result = 0;
		::stbtt_GetGlyphHMetrics(m_font, m_index, nullptr, &result);
		return result;
	}

	int32_t rightBearing() const
	{
		int32_t w = 0;
		auto lb = 0;
		::stbtt_GetGlyphHMetrics(m_font, m_index, &w, &lb);
		return lb + w;
	}

	int32_t width() const
	{
		auto result = 0;
		::stbtt_GetGlyphHMetrics(m_font, m_index, &result, nullptr);
		return result;
	}

	int32_t ascent() const
	{
		auto result = 0;
		::stbtt_GetFontVMetrics(m_font, &result, nullptr, nullptr);
		return result;
	}

	int32_t descent() const
	{
		auto result = 0;
		::stbtt_GetFontVMetrics(m_font, nullptr, &result, nullptr);
		return result;
	}

	int32_t height() const
	{
		auto asc = 0;
		auto dsc = 0;
		::stbtt_GetFontVMetrics(m_font, &asc, &dsc, nullptr);
		return asc - dsc;
	}

	int32_t lineGap() const
	{
		auto result = 0;
		::stbtt_GetFontVMetrics(m_font, nullptr, nullptr, &result);
		return result;
	}

	void render(
	        uint8_t *start, int32_t frame_width, int32_t frame_height, int32_t stride, float scale) const
	{
		::stbtt_MakeGlyphBitmap(
		        m_font, start, frame_width, frame_height, stride, scale, scale, m_index);
	}
};

class Font2D {
private:
	std::vector<uint8_t> m_ttf_data;
	::stbtt_fontinfo m_font;

	void load_font(const uint8_t *ttf_buffer);
	static std::vector<uint8_t> load_ttf(const std::string &path);

public:
	void init(const std::string &path)
	{
		m_ttf_data = load_ttf(path);
		load_font(m_ttf_data.data());
	}

	void init(const std::vector<uint8_t> &ttf_data)
	{
		m_ttf_data = ttf_data;
		load_font(m_ttf_data.data());
	}

	using Glyph = Font2DGlyph;

	Glyph GetGlyph(const char *code_point) const { return Glyph(m_font, code_point); }

	static int32_t KernAdvance(const Glyph &glyph1, const Glyph &glyph2)
	{
		assert(glyph1.m_font == glyph2.m_font);
		return ::stbtt_GetGlyphKernAdvance(glyph1.m_font, glyph1.m_index, glyph2.m_index);
	}

	float ScaleForPixelHeight(float pixels) const { return ::stbtt_ScaleForPixelHeight(&m_font, pixels); }

	using Layout = std::vector<Glyph>;

	Layout MakeLayout(const char *code_points, std::size_t cp_n) const;

	Layout MakeLayout(const std::vector<char> &code_points) const
	{
		return MakeLayout(code_points.data(), code_points.size() / sizeof(char32_t));
	}

	float Baseline(
	        std::size_t size_in_pixels, const Layout & /*layout*/
	) const
	{
		auto result = 0;
		::stbtt_GetFontVMetrics(&m_font, &result, nullptr, nullptr);
		return result * ScaleForPixelHeight(size_in_pixels);
	}

	float Height(
	        std::size_t size_in_pixels, const Layout & /*layout*/
	) const
	{
		auto asc = 0;
		auto dsc = 0;
		::stbtt_GetFontVMetrics(&m_font, &asc, &dsc, nullptr);
		return (asc - dsc) * ScaleForPixelHeight(size_in_pixels);
	}

	float Width(std::size_t size_in_pixels, const Layout &layout) const;

	void render(
	        std::size_t size_in_pixels, const Layout &layout, uint8_t *buffer_start,
	        std::size_t buffer_width, std::size_t buffer_height, int32_t xposition = 0,
	        int32_t yposition = 0) const;
};

}  // namespace spu::oglplus::text
#include <text/font2d.ipp>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
