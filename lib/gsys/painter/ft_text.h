//
// FTGlyph :
//
#pragma once

#include <ft2build.h>
#include <gsys/painter.h>
#include <ssys/ssys.h>
#include FT_FREETYPE_H

#include <map>
#include <string>
#include <vector>

namespace spu::gs_painter::freetype {
class FTAtlas;
class FTFontAtlas;
/*
 * FTGlyph metrics:
 * --------------
 *
 *                       xmin                     xmax
 *                        |                         |
 *                        |<-------- width -------->|
 *                        |                         |
 *              |         +-------------------------+----------------- ymax
 *              |         |    ggggggggg   ggggg    |     ^        ^
 *              |         |   g:::::::::ggg::::g    |     |        |
 *              |         |  g:::::::::::::::::g    |     |        |
 *              |         | g::::::ggggg::::::gg    |     |        |
 *              |         | g:::::g     g:::::g     |     |        |
 *    offset_x -|-------->| g:::::g     g:::::g     |  offset_y    |
 *              |         | g:::::g     g:::::g     |     |        |
 *              |         | g::::::g    g:::::g     |     |        |
 *              |         | g:::::::ggggg:::::g     |     |        |
 *              |         |  g::::::::::::::::g     |     |      height
 *              |         |   gg::::::::::::::g     |     |        |
 *  baseline ---*---------|---- gggggggg::::::g-----*--------      |
 *            / |         |             g:::::g     |              |
 *     origin   |         | gggggg      g:::::g     |              |
 *              |         | g:::::gg   gg:::::g     |              |
 *              |         |  g::::::ggg:::::::g     |              |
 *              |         |   gg:::::::::::::g      |              |
 *              |         |     ggg::::::ggg        |              |
 *              |         |         gggggg          |              v
 *              |         +-------------------------+----------------- ymin
 *              |                                   |
 *              |------------- advance_x ---------->|
 */

struct FTGlyph {
	enum Mode {
		e_normal,
		e_outline_edge,
		e_outline_positive,
		e_outline_negative,
		e_signed_distance_field,
	};

	uint32_t codepoint = -1;      // Unicode codepoint this glyph represents in UTF-32 LE encoding.
	uint32_t width = 0;           // FTGlyph's width in pixels.
	uint32_t height = 0;          // FTGlyph's height in pixels.
	int offset_x = 0;             // FTGlyph's left bearing expressed in integer pixels.
	int offset_y = 0;             // FTGlyphs's top bearing expressed in integer pixels.
	float advance_x = 0;          // For horizontal text layouts, horizontal distance (in fractional pixels)
	float advance_y = 0;          // For vertical text layouts, vertical distance (in fractional pixels)
	float s0 = 0;                 // First normalized texture coordinate (x) of top-left corner
	float t0 = 0;                 // Second normalized texture coordinate (y) of top-left corner
	float s1 = 0;                 // First normalized texture coordinate (x) of bottom-right corner
	float t1 = 0;                 // Second normalized texture coordinate (y) of bottom-right corner
	Mode mode = e_normal;         // Mode this glyph was rendered
	float outline_thickness = 0;  // FTGlyph outline thickness

	std::map<uint32_t, float> kernings;
	float getKerning(const char *codepoint) const;
};

class FTFont {
public:
	enum Location {
		e_file = 0,
		e_memory,
	};

	FTGlyph::Mode m_glyphMode = FTGlyph::e_normal;  // Mode the font is rendering its next glyph
	std::string m_filename;                         // Font filename, for when location == TEXTURE_FONT_FILE
	std::map<uint32_t, FTGlyph> m_glyphs;

	float m_size = 0;              // Font size
	float m_outlineThickness = 0;  // Outline thickness
	uint8_t m_filtering = 1;       // Whether to use our own lcd filter.
	uint8_t m_kerning = 1;         // Whether to use kerning if available
	uint8_t m_hinting = 1;         // Whether to use autohint when rendering font
	uint8_t m_scaletex = 1;        // Whether to scale texture coordinates
	float m_height = 0;            // the baseline-to-baseline distance) when writing text with this font.

	// distance that must be placed between two lines of text. The baseline-to-baseline
	// distance should be computed as ascender - descender + linegap
	float m_linegap = 0;

	/* Unfortunately, font formats define the ascender differently. For some, it represents the
	   ascent of all capital latin characters (without accents), for others it
	   is the ascent of the highest accented character, and finally, other
	   formats define it as being equal to bbox.yMax.
	*/
	// vertical distance from the horizontal baseline to the highest 'character'coordinate in a font face.
	float m_ascender = 0;

	// descender is the vertical distance from the horizontal baseline to the lowest'character' coordinate
	// in a font face
	float m_descender = 0;

	/* Unfortunately, font formats define the descender differently. For some, it represents the
	 * descent of all capital latin characters (without accents), for others it
	 * is the ascent of the lowest accented character, and finally, other
	 * formats define it as being equal to bbox.yMin. This field is negative
	 * for values below the baseline.
	 */

	// The position of the underline line for this face. It is the center of
	// the underlining stem. Only relevant for scalable formats.
	float m_underlinePosition;

	// The thickness of the underline for this face. Only relevant for
	// scalable formats.
	float m_underlineThickness;

	// The padding to be add to the glyph's texture that are loaded by this font.
	// Usefull when adding effects with shaders.
	int m_padding = 0;

	uint8_t m_lcdWeights[5] = {0};  // LCD filter weights

	FTFont() = default;
	~FTFont();

	int isVariable();
	void getWeight(FT_Fixed *def, FT_Fixed *min, FT_Fixed *max);
	void setWeight(FT_Fixed wght);

	const FTGlyph *loadGlyph(const char *codepoint);
	void loadGlyphs(const char *codepoints);
	void enlargeAtlas(uint32_t width, uint32_t height);
	void initFromFile(FTAtlas &atlas, const char *filename, const float fontsize);
	static void shutdown();

protected:
	struct Memory {
		const void *base = nullptr;
		uint32_t size = 0;
	};

	FT_Face m_ftFace = nullptr;  // Freetype face pointer
	FT_Size m_ftSize = 0;        // Freetype size pointer

	FTAtlas *m_atlas = nullptr;
	float m_scale = 1.0;           // factor to scale font coordinates
	Location m_location = e_file;  // font location
	Memory m_memory;               // Font memory address, for when location == TEXTURE_FONT_MEMORY
	FTGlyph m_special;             // special glyph. used for line drawing and background.

	void initInternal();
	void initSize();
	void generateKerning();
	void setSize(float size);
	void loadFace(float size);
	bool isColorFont() const;
	const FTGlyph *findGlyphInternal(uint32_t ucodepont);
	void setSpecial();

	static inline FT_Library ms_ftLibrary = nullptr;  // Freetype library pointer
};

struct FTMarkup {
	std::string family;         // A font family name such as "normal", "sans", "serif" or "monospace".
	float size;                 // Font size.
	int bold;                   // Whether text is bold.
	int italic;                 // Whether text is italic.
	float spacing;              // Spacing between letters.
	float gamma;                // Gamma correction.
	Vec4f foreground_color;     // Text color.
	Vec4f background_color;     // Background color.
	int outline;                // Whether outline is active.
	Vec4f outline_color;        // Outline color.
	int underline;              // Whether underline is active.
	Vec4f underline_color;      // Underline color.
	int overline;               // Whether overline is active.
	Vec4f overline_color;       // Overline color.
	int strikethrough;          // Whether strikethrough is active.
	Vec4f strikethrough_color;  // Strikethrough color.
	FTFont *font = nullptr;     // Pointer on the corresponding font (family/size/bold/italic)
};

extern std::vector<double> make_distance_mapd(std::vector<double> &img, uint32_t width, uint32_t height);
extern std::vector<uint8_t> make_distance_mapb(
        const std::vector<uint8_t> &img, uint32_t width, uint32_t height);
extern size_t utf8_surrogate_len(const char *character);
extern size_t utf8_strlen(const char *string);
extern uint32_t utf8_to_utf32(const char *character);

}  // namespace spu::gs_painter::freetype

namespace spu::gs_painter {
class FTText : public GsPainter {
public:
	enum RenderMode {
		e_render_default = 0,
		e_render_simple,
		e_render_fill,
		e_render_lcd,
		e_render_distance_field,
	};

	enum Align {
		e_align_left,    // Align text to the left hand side
		e_align_center,  // Align text to the center
		e_align_right    // Align text to the right hand side
	};

	struct Vertex {
		float x, y, z;
		float s, t;
		float r, g, b, a;
		float shift;
		float gamma;
		operator Vec3f() const { return {x, y, z}; }
	};

	~FTText();

	Mat4f u_textscreen;
	Vec3f u_atlas_size;
	Vec4f u_color = {1, 1, 1, 1};

	explicit FTText(const char *name = nullptr);
	explicit FTText(const Attrs &attrs) : FTText() { init(attrs); }
	/*
	auto getVerticesView() const { return GsPainter::getVerticesView<Vertex>(); }
	auto getVerticesView() { return GsPainter::getVerticesView<Vertex>(); }
	*/
	const Vec2f &getPen() const { return m_pen; }
	Vec2f &getPen() { return m_pen; }

	void init(const Attrs &attrs) override;
	void clear();

	void addChar(const freetype::FTMarkup &markup, const char *current, const char *previous);
	void addText(const freetype::FTMarkup &markup, const char *text, uint32_t length = 0);
	void addVertices(
	        const Vertex *vertices, const uint32_t vcount, const int32_t *indices, const uint32_t icount,
	        bool is_relative = true);

	void align(enum Align alignment);
	void transform(const Mat4f &matrix);
	void upload();

	Vec4i atlasSize() const;
	uint32_t atlasUsed() const;
	Recti getAtlasRegion(const uint32_t width, const uint32_t height);
	void setAtlasRegion(const Recti &region, const uint8_t *data, const uint32_t stride);

	freetype::FTFont *getFontFromFile(const char *filename, const float fontsize);
	freetype::FTFontAtlas *getAtlas() { return m_atlas; }

	static void allocGlobalAtlas(const Vec4i &size);
	static void freeGlobalAtlas();

	PAINTER_VERTEX_FUNCS_WITHOUT_MESH;

protected:
	enum State {
		e_clean = 0,
		e_dirty = 1,
	};
	struct LineInfo {
		size_t line_start;  // Index (in the vertex buffer) where this line starts
		Range3f range;      // bounds of this line
	};

	State m_state;
	freetype::FTFontAtlas *m_atlas = nullptr;
	bool m_isAtlasOwner = true;

	std::vector<Vec4i> m_items;
	std::vector<LineInfo> m_lineInfos;

	Vec2f m_pen = Vec2f(0);
	Vec2f m_origin = Vec2f(0);  // Pen origin

	float m_lastPenY = 0;              // Last pen y location
	size_t m_lineStart = 0;            // Index (in the vertex buffer) of the current line start
	float m_lineLeft = 0;              // Location of the start of the line
	float m_lineAscender = 0;          // Current line ascender
	float m_lineDescender = 0;         // Current line decender
	Align m_alignment = e_align_left;  // current alignment

	void finishLine(bool advancePen);
	void moveLastLine(float dy);
	void doRender() override;

	inline static freetype::FTFontAtlas *ms_globalAtlas = nullptr;
};

}  // namespace spu::gs_painter
