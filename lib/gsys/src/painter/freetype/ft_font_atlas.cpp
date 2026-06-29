//
// FTFontAtlas :
//
#include "ft_atlas.h"
#include <gsys/painter/ft_text.h>

namespace spu::gs_painter::freetype {

FTFontAtlas::~FTFontAtlas()
{
	for (auto &font: m_fonts) {
		delete font;
	}
}

FTFont *FTFontAtlas::getFontFromFile(const char *filename, const float fontsize)
{
	for (auto &font: m_fonts) {
		if (font->m_filename == filename && font->m_size == fontsize) {
			return font;
		}
	}

	auto font = new FTFont();
	font->initFromFile(*this, filename, fontsize);
	m_fonts.push_back(font);
	return font;
}
FTFont *FTFontAtlas::getFontFromMarkup(const FTMarkup &markup)
{
	return getFontFromFile(markup.family.c_str(), markup.size);
}
}  // namespace spu::gs_painter::freetype
