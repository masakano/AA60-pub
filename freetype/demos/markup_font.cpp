//
// MarkupFont :
//
#include <fontconfig/fontconfig.h>
#include <gsys/painter/ft_text.h>
#include "app.h"

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {
class MarkupFont : public App {
public:
	int32_t width() const override { return 500; }
	int32_t height() const override { return 300; }

	FTText m_text;
	FTText m_lines;

	std::string matchDescription(const char *description)
	{
		std::string filename;
		FcInit();
		FcPattern *pattern = FcNameParse((const FcChar8 *)description);
		FcConfigSubstitute(0, pattern, FcMatchPattern);
		FcDefaultSubstitute(pattern);
		FcResult result;
		FcPattern *match = FcFontMatch(0, pattern, &result);
		FcPatternDestroy(pattern);

		if (!match) {
			fprintf(stderr, "fontconfig error: could not match description '%s'", description);
			return 0;
		}
		else {
			FcValue value;
			FcResult result = FcPatternGet(match, FC_FILE, 0, &value);
			if (result) {
				fprintf(stderr, "fontconfig error: could not match description '%s'",
				        description);
			}
			else {
				filename = (char *)(value.u.s);
			}
		}
		FcPatternDestroy(match);
		return filename;
	}

	void init() override
	{
		Attrs text_attrs = {
		        {"path", "shaders/text.us"},
		        {"atlas.size", Vec4i(512, 512, 3, 0)},
		        {"def_render_mode", FTText::e_render_lcd},
		};
		m_text.init(text_attrs);

		Attrs line_attrs = {
		        {"path",            "shaders/text.us"    },
		        {"def_render_mode", FTText::e_render_fill},
		};
		m_lines.init(text_attrs);

		Vec4f black = {0.0, 0.0, 0.0, 1.0};
		Vec4f white = {1.0, 1.0, 1.0, 1.0};
		Vec4f yellow = {1.0, 1.0, 0.0, 1.0};
		Vec4f grey = {0.5, 0.5, 0.5, 1.0};
		Vec4f none = {1.0, 1.0, 1.0, 0.0};

		auto f_normal = matchDescription("Droid Serif:size=24");
		auto f_bold = matchDescription("Droid Serif:size=24:weight=bold");
		auto f_italic = matchDescription("Droid Serif:size=24:slant=italic");
		auto f_japanese = matchDescription("Droid Sans Japanese:size=18");
		auto f_math = matchDescription("DejaVu Sans:size=24");

		FTMarkup normal = {
		        .family = f_normal,
		        .size = 24.0,
		        .bold = 0,
		        .italic = 0,
		        .spacing = 0.0,
		        .gamma = 2.,
		        .foreground_color = white,
		        .background_color = none,
		        .outline = 0,
		        .outline_color = none,
		        .underline = 0,
		        .underline_color = white,
		        .overline = 0,
		        .overline_color = white,
		        .strikethrough = 0,
		        .strikethrough_color = white,
		        .font = 0,
		};
		FTMarkup highlight = normal;
		highlight.background_color = grey;
		FTMarkup reverse = normal;
		reverse.foreground_color = black;
		reverse.background_color = white;
		reverse.gamma = 1.0;
		FTMarkup overline = normal;
		overline.overline = 1;
		FTMarkup underline = normal;
		underline.underline = 1;
		FTMarkup small = normal;
		small.size = 10.0;
		FTMarkup big = normal;
		big.size = 48.0;
		big.italic = 1;
		big.foreground_color = yellow;
		FTMarkup bold = normal;
		bold.bold = 1;
		bold.family = f_bold;
		FTMarkup italic = normal;
		italic.italic = 1;
		italic.family = f_italic;
		FTMarkup japanese = normal;
		japanese.family = f_japanese;
		japanese.size = 18.0;
		FTMarkup math = normal;
		math.family = f_math;

		m_text.getPen() = {30, 260};
		m_text.addText(underline, "The");
		m_text.addText(normal, " Quick");
		m_text.addText(big, " brown ");
		m_text.addText(reverse, " fox \n");
		m_text.addText(italic, "jumps over ");
		m_text.addText(bold, "the lazy ");
		m_text.addText(normal, "dog.\n");
		m_text.addText(small, "Now is the time for all good men to come to the aid of the party.\n");
		m_text.addText(italic, "Ég get etið gler án þess að meiða mig.\n");
		m_text.addText(japanese, "私はガラスを食べられます。 それは私を傷つけません\n");
		m_text.addText(math, "ℕ ⊆ ℤ ⊂ ℚ ⊂ ℝ ⊂ ℂ");

		m_text.upload();
		m_text.align(FTText::e_align_center);

		Range3f range = m_text.getRange();
		float left = range.p0.x;
		float right = range.p1.x;
		float top = range.p1.y;
		float bottom = range.p0.y;

		// lines_buffer = new FTText();
		FTText::Vertex vertices[] = {
		        {left - 10,  top,         0, 0, 0, 0, 0, 0, 1, 0, 0}, // top
		        {right + 10, top,         0, 0, 0, 0, 0, 0, 1, 0, 0},

		        {left - 10,  bottom,      0, 0, 0, 0, 0, 0, 1, 0, 0}, // bottom
		        {right + 10, bottom,      0, 0, 0, 0, 0, 0, 1, 0, 0},

		        {left,       top + 10,    0, 0, 0, 0, 0, 0, 1, 0, 0}, // left
		        {left,       bottom - 10, 0, 0, 0, 0, 0, 0, 1, 0, 0},
		        {right,      top + 10,    0, 0, 0, 0, 0, 0, 1, 0, 0}, // right
		        {right,      bottom - 10, 0, 0, 0, 0, 0, 0, 1, 0, 0}
                };
		int32_t indices[] = {0, 1, 2, 3, 4, 5, 6, 7};
		m_lines.addVertices(vertices, 8, indices, 8);

		m_bgcolor0 = Vec4f(0.40, 0.40, 0.45, 1.00);
	}

	void doDisplay() override
	{
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		m_lines.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);

		m_text.draw(GL_TRIANGLES);
		m_lines.getADrawcall().blend_func = {
		        GL_ONE,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_ONE,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
		m_lines.draw(GL_LINES);
	}
};
App *createMarkupFont() { return new MarkupFont(); }
}  // namespace spu
