//
// FTFont :
//
#include <gsys/painter/ft_text.h>
#include "ft_atlas.h"

#define HRES 64
#define HRESf 64.f
#define DPI 72

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H
#include FT_STROKER_H
#include FT_LCD_FILTER_H
#include FT_TRUETYPE_TABLES_H
#include FT_MULTIPLE_MASTERS_H

namespace spu::gs_painter::freetype {

namespace {
float convert_F26Dot6_to_float(FT_F26Dot6 value) { return ((float)value) / 64.0; }
FT_F26Dot6 convert_float_to_F26Dot6(float value) { return (FT_F26Dot6)(value * 64.0); }
uint32_t rol(uint32_t in, uint32_t x) { return (in >> (32 - x)) | (in << x); }
}  // namespace

float FTGlyph::getKerning(const char *codepoint) const
{
	auto ucodepoint = utf8_to_utf32(codepoint);
	auto it = kernings.find(ucodepoint);
	return it != std::end(kernings) ? it->second : 0.0f;
}

void FTFont::generateKerning()
{
	std::vector<FTGlyph *> glyph_ptrs;
	for (auto &pair: m_glyphs) {
		glyph_ptrs.push_back(&pair.second);
	}

	for (auto &glyph: glyph_ptrs) {
		auto glyph_index = FT_Get_Char_Index(m_ftFace, glyph->codepoint);
		glyph->kernings.clear();

		for (auto &prev_glyph: glyph_ptrs) {
			auto prev_index = FT_Get_Char_Index(m_ftFace, prev_glyph->codepoint);
			FT_Vector kerning;
			FT_Get_Kerning(m_ftFace, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning);
			if (kerning.x) {
				glyph->kernings[prev_glyph->codepoint]
				        = convert_F26Dot6_to_float(kerning.x) / HRESf;
			}
			FT_Get_Kerning(m_ftFace, glyph_index, prev_index, FT_KERNING_UNFITTED, &kerning);
			if (kerning.x) {
				glyph->kernings[prev_glyph->codepoint] = kerning.x / (float)(HRESf * HRESf);
			}
		}
	}
}

bool FTFont::isColorFont() const
{
	static const uint32_t tag = FT_MAKE_TAG('C', 'B', 'D', 'T');
	unsigned long length = 0;
	FT_Load_Sfnt_Table(m_ftFace, tag, 0, nullptr, &length);
	return length != 0;
}

void FTFont::setSize(float size)
{
	FT_Error error = 0;
	FT_Matrix matrix
	        = {(int)((1.0 / HRES) * 0x10000L), (int)((0.0) * 0x10000L), (int)((0.0) * 0x10000L),
	           (int)((1.0) * 0x10000L)};

	if (FT_HAS_FIXED_SIZES(m_ftFace)) {
		/* Select best size */
		if (m_ftFace->num_fixed_sizes == 0) {
			aux_error(true, "no fixed size in color font\n");
		}

		int best_match = 0;
		float diff = 1e20;

		for (auto i = 0; i < m_ftFace->num_fixed_sizes; ++i) {
			auto new_size = convert_F26Dot6_to_float(m_ftFace->available_sizes[i].size);
			auto ndiff = size > new_size ? size / new_size : new_size / size;

			aux_message(
			        1, "candiate: size[%i]=%f %d*%d\n", i, new_size,
			        m_ftFace->available_sizes[i].width, m_ftFace->available_sizes[i].height);

			if (ndiff < diff) {
				best_match = i;
				diff = ndiff;
			}
		}
		aux_message(1, "selected: size[%i] for %f\n", best_match, size);
		error = FT_Select_Size(m_ftFace, best_match);
		aux_error(error, "FT_Select_Size: error %d\n", error);

		m_scale = m_size / convert_F26Dot6_to_float(m_ftFace->available_sizes[best_match].size);
	}
	else {
		/* Set char size */
		error = FT_Set_Char_Size(m_ftFace, convert_float_to_F26Dot6(size), 0, DPI * HRES, DPI);
		aux_error(error, "FT_Set_Char_Size: error %d\n", error);
	}
	/* Set transform matrix */
	FT_Set_Transform(m_ftFace, &matrix, nullptr);
}

void FTFont::initSize()
{
	FT_Size_Metrics metrics;

	m_underlinePosition = m_ftFace->underline_position / (float)(HRESf * HRESf) * m_size;
	m_underlinePosition = roundf(m_underlinePosition);
	if (m_underlinePosition > -2) {
		m_underlinePosition = -2.0;
	}

	m_underlineThickness = m_ftFace->underline_thickness / (float)(HRESf * HRESf) * m_size;
	m_underlineThickness = roundf(m_underlineThickness);
	if (m_underlineThickness < 1) {
		m_underlineThickness = 1.0;
	}

	metrics = m_ftFace->size->metrics;
	m_ascender = metrics.ascender >> 6;
	m_descender = metrics.descender >> 6;
	m_height = metrics.height >> 6;
	m_linegap = m_height - m_ascender + m_descender;
}

void FTFont::initInternal()
{
	assert(m_atlas);
	assert(m_size > 0);
	assert((m_location == e_file && !m_filename.empty())
	       || (m_location == e_memory && m_memory.base && m_memory.size));

	m_height = 0;
	m_ascender = 0;
	m_descender = 0;
	m_linegap = 0;
	m_glyphMode = FTGlyph::e_normal;
	m_outlineThickness = 0.0;
	m_hinting = 1;
	m_kerning = 1;
	m_filtering = 1;
	m_scaletex = 1;
	m_scale = 1.0;

	m_lcdWeights[0] = 0x10;
	m_lcdWeights[1] = 0x40;
	m_lcdWeights[2] = 0x70;
	m_lcdWeights[3] = 0x40;
	m_lcdWeights[4] = 0x10;

	loadFace(m_size);
	initSize();
	setSize(m_size);
	setSpecial();
}

void FTFont::initFromFile(FTAtlas &atlas, const char *filename, const float fontsize)
{
	m_atlas = &atlas;
	m_size = fontsize;
	m_location = e_file;
	m_filename = File::searchPath(filename);
	initInternal();
}

FTFont::~FTFont()
{
	if (m_ftSize) {
		auto error = FT_Done_Size(m_ftSize);
		aux_error(error, "FT_Done_Size: error %d\n", error);
	}
	if (m_ftFace) {
		auto error = FT_Done_Face(m_ftFace);
		aux_error(error, "FT_Done_Face: error %d\n", error);
	}
}
#if 0
FTFont *FTFont::clone(float pt_size)
{
	FT_Error error = 0;
	float native_size = m_size / m_scale;  // unscale fonts

	FTFont *newone = new FTFont();

	*newone = *this;
	newone->m_size = pt_size;

	error = FT_New_Size(newone->m_ftFace, &newone->m_ftSize);
	aux_error(error, "FT_New_Size: error %d\n", error);

	error = FT_Activate_Size(newone->m_ftSize);
	aux_message(error, "FT_Activate_Size: error %d\n", error);

	newone->setSize(pt_size);
	newone->initSize();

	if (newone->m_size / newone->m_scale != native_size) newone->m_glyphs.clear();
	return newone;
}
#endif

void FTFont::shutdown()
{
	printf("shutdown...\n");
	FT_Done_FreeType(ms_ftLibrary);
	ms_ftLibrary = nullptr;
	printf("shutdown done...\n");
}

void FTFont::loadFace(float size)
{
	FT_Error error;

	if (!ms_ftLibrary) {
		// printf("FT_Init_FreeType...\n");
		error = FT_Init_FreeType(&ms_ftLibrary);
		aux_error(error, "FT_Init_FreeType: error %d\n", error);
	}

	if (!m_ftFace) {
		switch (m_location) {
		case e_file:
			error = FT_New_Face(ms_ftLibrary, m_filename.c_str(), 0, &m_ftFace);
			aux_error(error, "FT_New_Face: error %d\n", error);
			break;

		case e_memory:
			error = FT_New_Memory_Face(
			        ms_ftLibrary, (const FT_Byte *)m_memory.base, m_memory.size, 0, &m_ftFace);

			aux_error(error, "FT_New_Memory_Face: error %d\n", error);
			break;
		}

		/* Select charmap */
		error = FT_Select_Charmap(m_ftFace, FT_ENCODING_UNICODE);
		aux_error(error, "FT_Select_Charmap: error %d\n", error);

		error = FT_New_Size(m_ftFace, &m_ftSize);
		aux_error(error, "FT_New_Face: error %d\n", error);

		error = FT_Activate_Size(m_ftSize);
		aux_error(error, "FT_Activate_Size: error %d\n", error);

		setSize(size);
	}
}

int FTFont::isVariable()
{
	int result = 0;

	if (m_ftFace) result = m_ftFace->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS;

	return result == FT_FACE_FLAG_MULTIPLE_MASTERS;
}

void FTFont::getWeight(FT_Fixed *def, FT_Fixed *min, FT_Fixed *max)
{
	int result = 0;

	if (def && min && max) {
		*def = 0;
		*min = 0;
		*max = 0;

		if (ms_ftLibrary && m_ftFace) {
			FT_MM_Var *master;

			if (FT_Get_MM_Var(m_ftFace, &master) == 0) {
				const FT_Tag tag = FT_MAKE_TAG('w', 'g', 'h', 't');
				const char *name = "Weight";

				for (uint32_t i = 0; i < 16 && i < master->num_axis; i++) {
					if (tag == master->axis[i].tag
					    || strcmp(name, master->axis[i].name) == 0) {
						*def = master->axis[i].def;
						*min = master->axis[i].minimum;
						*max = master->axis[i].maximum;
						// result = 1;
						break;
					}
				}
				FT_Done_MM_Var(ms_ftLibrary, master);
			}
		}
	}
	// return result;
}

void FTFont::setWeight(FT_Fixed wght)
{
	if (ms_ftLibrary && m_ftFace) {
		FT_MM_Var *master;

		if (FT_Get_MM_Var(m_ftFace, &master) == 0) {
			const FT_Tag tag = FT_MAKE_TAG('w', 'g', 'h', 't');
			const char *name = "Weight";

			for (auto i = 0; i < 16 && i < master->num_axis; i++) {
				if (tag == master->axis[i].tag || strcmp(name, master->axis[i].name) == 0) {
					const FT_Fixed min = master->axis[i].minimum;
					const FT_Fixed max = master->axis[i].maximum;

					if (wght >= min && wght <= max) {
						const auto n = i + 1;
						FT_Fixed coords[16];

						if (FT_Get_Var_Design_Coordinates(m_ftFace, n, coords) == 0) {
							coords[i] = wght;

							if (FT_Set_Var_Design_Coordinates(m_ftFace, n, coords)
							    != 0) {
								aux_message(
								        0,
								        "variable font weight not available\n");
							}
						}
					}
					else {
						aux_message(0, "variable font weight out of range\n");
						// result = -1;
					}
					break;
				}
			}
			FT_Done_MM_Var(ms_ftLibrary, master);
		}
	}
}

const FTGlyph *FTFont::findGlyphInternal(uint32_t ucodepoint)
{
	auto it = m_glyphs.find(ucodepoint);
	if (it != std::end(m_glyphs)) {
		auto &glyph = it->second;
		if (glyph.mode == m_glyphMode && glyph.outline_thickness == m_outlineThickness) {
			return &glyph;
		}
	}
	return nullptr;
}

const FTGlyph *FTFont::loadGlyph(const char *codepoint)
{
	if (codepoint == nullptr) {
		return &m_special;
	}

	uint32_t ucodepoint = utf8_to_utf32(codepoint);
	uint32_t glyph_index = FT_Get_Char_Index(m_ftFace, ucodepoint);

	FT_Error error;
	FT_Glyph ft_glyph = nullptr;
	FT_GlyphSlot slot;
	FT_Bitmap ft_bitmap;
	FT_Int32 flags = 0;
	int32_t ft_glyphop = 0;
	int32_t ft_glyph_left = 0;

	{
		const auto *glyph = findGlyphInternal(ucodepoint);
		if (glyph) return glyph;
	}

	loadFace(m_size);
	if (glyph_index == 0) {
		const auto glyph = findGlyphInternal(utf8_to_utf32("\0"));
		if (glyph) {
			m_glyphs[ucodepoint] = *glyph;
			return &m_glyphs.at(ucodepoint);
		}
	}

	// WARNING: We use texture-atlas depth to guess if user wants
	//          LCD subpixel rendering
	if (m_glyphMode != FTGlyph::e_normal && m_glyphMode != FTGlyph::e_signed_distance_field) {
		flags |= FT_LOAD_NO_BITMAP;
	}
	else {
		flags |= FT_LOAD_RENDER;
	}

	if (!m_hinting) {
		flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
	}
	else {
		flags |= FT_LOAD_FORCE_AUTOHINT;
	}

	if (m_atlas->size().z == 3) {
		FT_Library_SetLcdFilter(ms_ftLibrary, FT_LCD_FILTER_LIGHT);
		flags |= FT_LOAD_TARGET_LCD;

		if (m_filtering) {
			FT_Library_SetLcdFilterWeights(ms_ftLibrary, m_lcdWeights);
		}
	}
	else if (HRES == 1) {
		/* “FT_LOAD_TARGET_LIGHT
		 *  A lighter hinting algorithm for gray-level modes. Many generated
		 *  glyphs are fuzzier but better resemble their original shape.
		 *  This is achieved by snapping glyphs to the pixel grid
		 *  only vertically (Y-axis), as is done by FreeType's new CFF engine
		 *  or Microsoft's ClearType font renderer.”
		 * https://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#ft_load_target_xxx
		 */
		flags |= FT_LOAD_TARGET_LIGHT;
	}

	if (m_atlas->size().z == 4) {
		flags |= FT_LOAD_COLOR;
	}

	error = FT_Activate_Size(m_ftSize);
	aux_error(error, "FT_Activate_Size error = %d\n", error);

	error = FT_Load_Glyph(m_ftFace, glyph_index, flags);
	aux_error(error, "FT_Load_Glyph error = %d\n", error);

	if (m_glyphMode == FTGlyph::e_normal || m_glyphMode == FTGlyph::e_signed_distance_field) {
		slot = m_ftFace->glyph;
		ft_bitmap = slot->bitmap;
		ft_glyphop = slot->bitmap_top;
		ft_glyph_left = slot->bitmap_left;
	}
	else {
		FT_Stroker stroker;
		FT_BitmapGlyph ft_bitmap_glyph;

		error = FT_Stroker_New(ms_ftLibrary, &stroker);
		aux_error(error, "FT_Stroker_New: error = %d\n", error);

		FT_Stroker_Set(
		        stroker, (int)(m_outlineThickness * HRES), FT_STROKER_LINECAP_ROUND,
		        FT_STROKER_LINEJOIN_ROUND, 0);

		error = FT_Get_Glyph(m_ftFace->glyph, &ft_glyph);
		aux_error(error, "FT_Get_Glyph: error = %d\n", error);

		if (m_glyphMode == FTGlyph::e_outline_edge) {
			error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
		}
		else if (m_glyphMode == FTGlyph::e_outline_positive) {
			error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 0, 1);
		}
		else if (m_glyphMode == FTGlyph::e_outline_negative) {
			error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 1, 1);
		}
		aux_error(error, "FT_Get_StrokeBorder: error %d\n", error);

		switch (m_atlas->size().z) {
		case 1: error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1); break;
		case 3: error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_LCD, 0, 1); break;
		case 4: error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1); break;
		}
		aux_error(error, "FT_Glhyp_To_Bitmap: error %d\n", error);

		ft_bitmap_glyph = (FT_BitmapGlyph)ft_glyph;
		ft_bitmap = ft_bitmap_glyph->bitmap;
		ft_glyphop = ft_bitmap_glyph->top;
		ft_glyph_left = ft_bitmap_glyph->left;
	}

	struct {
		int left;
		int top;
		int right;
		int bottom;
	} padding = {0, 0, 1, 1};

	if (m_glyphMode == FTGlyph::e_signed_distance_field) {
		padding.top = 1;
		padding.left = 1;
	}

	if (m_padding != 0) {
		padding.top += m_padding;
		padding.left += m_padding;
		padding.right += m_padding;
		padding.bottom += m_padding;
	}

	auto src_w = m_atlas->size().z == 3 ? ft_bitmap.width / 3 : ft_bitmap.width;
	auto src_h = ft_bitmap.rows;

	auto tgt_w = src_w + padding.left + padding.right;
	auto tgt_h = src_h + padding.top + padding.bottom;
	auto region = m_atlas->getRegion(tgt_w, tgt_h);

	if (region.ox < 0) {
		aux_message(0, "texture atlas full\n");
		return &m_special;
	}
	auto x = region.ox;
	auto y = region.oy;

	// Copy pixel data over
	std::vector<uint8_t> buffer(tgt_w * tgt_h * m_atlas->size().z);

	auto *dst_ptr = buffer.data() + (padding.top * tgt_w + padding.left) * m_atlas->size().z;
	auto *src_ptr = ft_bitmap.buffer;
	if (ft_bitmap.pixel_mode == FT_PIXEL_MODE_BGRA && m_atlas->size().z == 4) {
		// BGRA in, RGBA out
		for (auto i = 0; i < src_h; i++) {
			for (auto j = 0; j < ft_bitmap.width; j++) {
				uint32_t bgra, rgba;
				bgra = ((uint32_t *)src_ptr)[j];
				rgba = rol(__builtin_bswap32(bgra), 24);  // little endian
				((uint32_t *)dst_ptr)[j] = rgba;
			}
			dst_ptr += tgt_w * m_atlas->size().z;
			src_ptr += ft_bitmap.pitch;
		}
	}
	else if (ft_bitmap.pixel_mode == FT_PIXEL_MODE_BGRA && m_atlas->size().z == 1) {
		// BGRA in, grey out: Use weighted sum for luminosity, and multiply by alpha
		struct src_pixel_t {
			uint8_t b;
			uint8_t g;
			uint8_t r;
			uint8_t a;
		} *src = (struct src_pixel_t *)ft_bitmap.buffer;
		for (auto row = 0; row < src_h; row++, dst_ptr += tgt_w * m_atlas->size().z) {
			for (int col = 0; col < src_w; col++, src++) {
				dst_ptr[col]
				        = (0.3 * src->r + 0.59 * src->g + 0.11 * src->b) * (src->a / 255.0);
			}
		}
	}
	else if (ft_bitmap.pixel_mode == FT_PIXEL_MODE_GRAY && m_atlas->size().z == 4) {
		// Grey in, RGBA out: Use grey level for alpha channel, with white color
		struct dst_pixel_t {
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		} *dst = (struct dst_pixel_t *)dst_ptr;
		for (auto row = 0; row < src_h; row++, dst += tgt_w) {
			for (int col = 0; col < src_w; col++, src_ptr++) {
				dst[col] = (struct dst_pixel_t){255, 255, 255, *src_ptr};
			}
		}
	}
	else {
		// Straight copy, per row
		for (auto i = 0; i < src_h; i++) {
			// difference between width and pitch:
			// https://www.freetype.org/freetype2/docs/reference/ft2-basic_types.html#FT_Bitmap
			memcpy(dst_ptr, src_ptr, ft_bitmap.width);
			dst_ptr += tgt_w * m_atlas->size().z;
			src_ptr += ft_bitmap.pitch;
		}
	}

	if (m_glyphMode == FTGlyph::e_signed_distance_field) {
		buffer = make_distance_mapb(buffer, tgt_w, tgt_h);
	}

	m_atlas->setRegion(Recti(x, y, tgt_w, tgt_h), buffer.data(), tgt_w * m_atlas->size().z);

	FTGlyph glyph;
	glyph.codepoint = glyph_index ? ucodepoint : 0;
	glyph.width = tgt_w;
	glyph.height = tgt_h;
	glyph.mode = m_glyphMode;
	glyph.outline_thickness = m_outlineThickness;
	glyph.offset_x = ft_glyph_left;
	glyph.offset_y = ft_glyphop;
	if (m_scaletex) {
		glyph.s0 = x / (float)m_atlas->size().x;
		glyph.t0 = y / (float)m_atlas->size().y;
		glyph.s1 = (x + glyph.width) / (float)m_atlas->size().x;
		glyph.t1 = (y + glyph.height) / (float)m_atlas->size().y;
	}
	else {
		// fix up unscaled coordinates by subtracting 0.5
		// this avoids drawing pixels from neighboring characters
		// note that you also have to paint these glyphs with an offset of
		// half a pixel each to get crisp rendering
		glyph.s0 = x - 0.5;
		glyph.t0 = y - 0.5;
		glyph.s1 = x + tgt_w - 0.5;
		glyph.t1 = y + tgt_h - 0.5;
	}
	slot = m_ftFace->glyph;
	if (FT_HAS_FIXED_SIZES(m_ftFace)) {
		// color fonts use actual pixels, not subpixels
		glyph.advance_x = slot->advance.x;
		glyph.advance_y = slot->advance.y;
	}
	else {
		glyph.advance_x = convert_F26Dot6_to_float(slot->advance.x) * m_scale;
		glyph.advance_y = convert_F26Dot6_to_float(slot->advance.y) * m_scale;
	}
	m_glyphs[ucodepoint] = glyph;

	// if (m_glyphMode != FTGlyph::e_normal && m_glyphMode != FTGlyph::e_signed_distance_field) {
	if (ft_glyph) {
		FT_Done_Glyph(ft_glyph);
	}
	generateKerning();
	return &m_glyphs.at(ucodepoint);
}

void FTFont::loadGlyphs(const char *codepoints)
{
	for (auto i = 0u; i < strlen(codepoints); i += utf8_surrogate_len(codepoints + i)) {
		loadGlyph(codepoints + i);
	}
}

void FTFont::enlargeAtlas(uint32_t width, uint32_t height)
{
	assert(m_atlas);
	assert(width >= m_atlas->size().x);
	assert(height >= m_atlas->size().y);
	assert(width + height > m_atlas->size().x + m_atlas->size().y);
	assert(width + height > m_atlas->size().x + m_atlas->size().y);

	FTAtlas *atlas = m_atlas;
	uint32_t width_old = atlas->size().x;
	uint32_t height_old = atlas->size().y;

	m_atlas->enlargeTexture(width, height);

	if (m_scaletex) {
		float mulw = (float)width_old / width;
		float mulh = (float)height_old / height;
		for (auto &pair: m_glyphs) {
			auto &glyph = pair.second;
			glyph.s0 *= mulw;
			glyph.s1 *= mulw;
			glyph.t0 *= mulh;
			glyph.t1 *= mulh;
		}
	}
}

void FTFont::setSpecial()
{
	assert(m_atlas);
	auto region = m_atlas->getRegion(5, 5);

	static uint8_t data[4 * 4 * 3] = {
	        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	if (region.ox < 0) {
		aux_error(true, "texture atlas full\n");
	}
	m_atlas->setRegion(Recti(region.ox, region.oy, 4, 4), data, 0);

	m_special.codepoint = -1;
	m_special.s0 = (region.ox + 2) / (float)m_atlas->size().x;
	m_special.t0 = (region.oy + 2) / (float)m_atlas->size().y;
	m_special.s1 = (region.ox + 3) / (float)m_atlas->size().x;
	m_special.t1 = (region.oy + 3) / (float)m_atlas->size().y;
}

}  // namespace spu::gs_painter::freetype
