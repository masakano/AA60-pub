//
// Ansi :
//
#include <gsys/painter/ft_text.h>
#include "app.h"

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {
class Ansi : public App {
public:
	int32_t width() const override { return 800; }
	int32_t height() const override { return 600; }

	FTText m_text;
	FTMarkup m_markup;
	std::vector<Vec4f> m_colors;

	void init() override
	{
		Attrs text_attrs = {
		        {"atlas.size", Vec4i(512, 512, 1, 0)},
		};
		m_text.init(text_attrs);

		Vec4f black = {0.0, 0.0, 0.0, 1.0};
		Vec4f none = {1.0, 1.0, 1.0, 0.0};

		// Markup markup;
		m_markup.family = "assets/VeraMono.ttf";
		m_markup.size = 15.0;
		m_markup.bold = 0;
		m_markup.italic = 0;
		m_markup.spacing = 0.0;
		m_markup.gamma = 1.0;
		m_markup.foreground_color = black;
		m_markup.background_color = none;
		m_markup.underline = 0;
		m_markup.underline_color = black;
		m_markup.overline = 0;
		m_markup.overline_color = black;
		m_markup.strikethrough = 0;
		m_markup.strikethrough_color = black;
		m_markup.font = 0;

		File file("data/256colors.txt", "r");
		std::string line;

		m_text.getPen() = {10.0, 480.0};
		while (file.getline(line)) {
			print(line.c_str());
		}
		m_text.upload();

		m_bgcolor0 = {1.0, 1.0, 1.0, 1.0};
	}

	void doDisplay() override
	{
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);
		m_text.draw(GL_TRIANGLES);
	}

	void initColors(Vec4f *colors)
	{
		Vec4f defaults[16] = {
		        {46 / 256.0f,  52 / 256.0f,  54 / 256.0f,  1.0f},
		        {204 / 256.0f, 0 / 256.0f,   0 / 256.0f,   1.0f},
		        {78 / 256.0f,  154 / 256.0f, 6 / 256.0f,   1.0f},
		        {196 / 256.0f, 160 / 256.0f, 0 / 256.0f,   1.0f},
		        {52 / 256.0f,  101 / 256.0f, 164 / 256.0f, 1.0f},
		        {117 / 256.0f, 80 / 256.0f,  123 / 256.0f, 1.0f},
		        {6 / 256.0f,   152 / 256.0f, 154 / 256.0f, 1.0f},
		        {211 / 256.0f, 215 / 256.0f, 207 / 256.0f, 1.0f},
		        {85 / 256.0f,  87 / 256.0f,  83 / 256.0f,  1.0f},
		        {239 / 256.0f, 41 / 256.0f,  41 / 256.0f,  1.0f},
		        {138 / 256.0f, 226 / 256.0f, 52 / 256.0f,  1.0f},
		        {252 / 256.0f, 233 / 256.0f, 79 / 256.0f,  1.0f},
		        {114 / 256.0f, 159 / 256.0f, 207 / 256.0f, 1.0f},
		        {173 / 256.0f, 127 / 256.0f, 168 / 256.0f, 1.0f},
		        {52 / 256.0f,  226 / 256.0f, 226 / 256.0f, 1.0f},
		        {238 / 256.0f, 238 / 256.0f, 236 / 256.0f, 1.0f}
                };
		size_t i = 0;
		/* Default 16 colors */
		for (i = 0; i < 16; ++i) {
			colors[i] = defaults[i];
		}
		/* Color cube */
		for (i = 0; i < 6 * 6 * 6; i++) {
			Vec4f color = {(i / 6 / 6) / 5.0f, ((i / 6) % 6) / 5.0f, (i % 6) / 5.0f, 1.0f};
			colors[i + 16] = color;
		}
		/* Grascale ramp (24 tones) */
		for (i = 0; i < 24; i++) {
			Vec4f color = {i / 24.0f, i / 24.0f, i / 24.0f, 1.0f};
			colors[232 + i] = color;
		}
	}

	void ansiToMarkup(const char *sequence, size_t length)
	{
		size_t i;
		int code = 0;
		int set_bg = -1;
		int set_fg = -1;
		Vec4f none = {0, 0, 0, 0};

		if (m_colors.empty()) {
			m_colors.resize(256);
			initColors(m_colors.data());
		}

		if (length <= 1) {
			m_markup.foreground_color = m_colors[0];
			m_markup.underline_color = m_markup.foreground_color;
			m_markup.overline_color = m_markup.foreground_color;
			m_markup.strikethrough_color = m_markup.foreground_color;
			m_markup.outline_color = m_markup.foreground_color;
			m_markup.background_color = none;
			m_markup.underline = 0;
			m_markup.overline = 0;
			m_markup.bold = 0;
			m_markup.italic = 0;
			m_markup.strikethrough = 0;
			return;
		}

		for (i = 0; i < length; ++i) {
			char c = *(sequence + i);
			if (c >= '0' && c <= '9') {
				code = code * 10 + (c - '0');
			}
			else if ((c == ';') || (i == (length - 1))) {
				if (set_fg == 1) {
					m_markup.foreground_color = m_colors[code];
					set_fg = -1;
				}
				else if (set_bg == 1) {
					m_markup.background_color = m_colors[code];
					set_bg = -1;
				}
				else if ((set_fg == 0) && (code == 5)) {
					set_fg = 1;
					code = 0;
				}
				else if ((set_bg == 0) && (code == 5)) {
					set_bg = 1;
					code = 0;
				}
				/* Set fg color (30 + x, where x is the index of the color) */
				else if ((code >= 30) && (code < 38)) {
					m_markup.foreground_color = m_colors[code - 30];
				}
				/* Set bg color (40 + x, where x is the index of the color) */
				else if ((code >= 40) && (code < 48)) {
					m_markup.background_color = m_colors[code - 40];
				}
				else {
					switch (code) {
					case 0:
						m_markup.foreground_color = m_colors[0];
						m_markup.background_color = none;
						m_markup.underline = 0;
						m_markup.overline = 0;
						m_markup.bold = 0;
						m_markup.italic = 0;
						m_markup.strikethrough = 0;
						break;
					case 1: m_markup.bold = 1; break;
					case 21: m_markup.bold = 0; break;
					case 2: m_markup.foreground_color.a = 0.5; break;
					case 22: m_markup.foreground_color.a = 1.0; break;
					case 3: m_markup.italic = 1; break;
					case 23: m_markup.italic = 0; break;
					case 4: m_markup.underline = 1; break;
					case 24: m_markup.underline = 0; break;
					case 8: m_markup.foreground_color.a = 0.0; break;
					case 28: m_markup.foreground_color.a = 1.0; break;
					case 9: m_markup.strikethrough = 1; break;
					case 29: m_markup.strikethrough = 0; break;
					case 53: m_markup.overline = 1; break;
					case 55: m_markup.overline = 0; break;
					case 39: m_markup.foreground_color = m_colors[0]; break;
					case 49: m_markup.background_color = none; break;
					case 38: set_fg = 0; break;
					case 48: set_bg = 0; break;
					default: break;
					}
				}
				code = 0;
			}
		}
		m_markup.underline_color = m_markup.foreground_color;
		m_markup.overline_color = m_markup.foreground_color;
		m_markup.strikethrough_color = m_markup.foreground_color;
		m_markup.outline_color = m_markup.foreground_color;

		if (m_markup.bold && m_markup.italic) {
			m_markup.family = "assets/VeraMoBI.ttf";
		}
		else if (m_markup.bold) {
			m_markup.family = "assets/VeraMoBd.ttf";
		}
		else if (m_markup.italic) {
			m_markup.family = "assets/VeraMoIt.ttf";
		}
		else {
			m_markup.family = "assets/VeraMono.ttf";
		}
	}

	void print(const char *text)
	{
		const char *seq_start = text, *seq_end = text;
		const char *p;
		for (p = text; p < (text + strlen(text)); ++p) {
			const char *start = strstr(p, "\033[");
			const char *end = NULL;
			if (start) {
				end = strstr(start + 1, "m");
			}
			if ((start == p) && (end > start)) {
				seq_start = start + 2;
				seq_end = end;
				p = end;
			}
			else {
				int seq_size = (seq_end - seq_start) + 1;
				const char *text_start = p;
				int text_size = 0;
				if (start) {
					text_size = start - p;
					p = start - 1;
				}
				else {
					text_size = text + strlen(text) - p;
					p = text + strlen(text);
				}
				ansiToMarkup(seq_start, seq_size);

				// m_markup.font = m_text.getFontFromMarkup(m_markup);
				m_text.addText(m_markup, text_start, text_size);
			}
		}
	}
};  // ansi
App *createAnsi() { return new Ansi(); }
}  // namespace spu
