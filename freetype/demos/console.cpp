//
// Console :
//
#include <gsys/painter/ft_text.h>
#include "app.h"

using namespace spu::gs_painter;
using namespace spu::gs_painter::freetype;

namespace spu {
const int c_signal_activate = 0;
const int c_signal_complete = 1;
const int c_signal_history_next = 2;
const int c_signal_history_prev = 3;
#define e_max_line_length 511

const int c_markup_normal = 0;
const int c_markup_error = 1;
const int c_markup_warning = 2;
const int c_markup_output = 3;
const int c_markup_bold = 4;
const int c_markup_italic = 5;
const int c_markup_bold_italic = 6;
const int c_markup_faint = 7;
#define e_markup_count 8

class Console : public App {
public:
	int32_t width() const override { return 600; }
	int32_t height() const override { return 400; }

	std::vector<std::string> m_lines;
	char *m_prompt;
	char m_killrings[e_max_line_length + 1];
	char m_inputs[e_max_line_length + 1];
	size_t m_cursor;

	FTMarkup m_markups[e_markup_count];
	FTText m_text;

	Vec2f m_pen;
	void (*m_handlers[4])(char *);

	int m_controlKeyHandled;
	SpuPad *m_pad;
	SpuPad m_curr;
	SpuPad m_prev;

	void init(float fontsize)
	{
		m_prompt = strdup(">>> ");
		m_cursor = 0;
		m_inputs[0] = '\0';
		m_killrings[0] = '\0';
		m_handlers[c_signal_activate] = 0;
		m_handlers[c_signal_complete] = 0;
		m_handlers[c_signal_history_next] = 0;
		m_handlers[c_signal_history_prev] = 0;
		m_pen.x = m_pen.y = 0;

		Attrs text_attrs = {
		        {"def_render_mode", FTText::e_render_simple},
		        {"atlas.size", Vec4i(512, 512, 1, 0)},
		};
		m_text.init(text_attrs);

		Vec4f white = {1, 1, 1, 1};
		Vec4f black = {0, 0, 0, 1};
		Vec4f none = {0, 0, 1, 0};

		FTMarkup normal;
		normal.family = "assets/VeraMono.ttf";
		normal.size = fontsize;
		normal.bold = 0;
		normal.italic = 0;
		normal.spacing = 0.0;
		normal.gamma = 1.0;
		normal.foreground_color = black;
		normal.background_color = none;
		normal.underline = 0;
		normal.underline_color = white;
		normal.overline = 0;
		normal.overline_color = white;
		normal.strikethrough = 0;
		normal.strikethrough_color = white;
		normal.font = m_text.getFontFromFile("assets/VeraMono.ttf", fontsize);

		FTMarkup bold = normal;
		bold.bold = 1;
		bold.font = m_text.getFontFromFile("assets/VeraMoBd.ttf", fontsize);

		FTMarkup italic = normal;
		italic.italic = 1;
		italic.font = m_text.getFontFromFile("assets/VeraMoIt.ttf", fontsize);

		FTMarkup bold_italic = normal;
		bold_italic.bold = 1;
		bold_italic.italic = 1;
		bold_italic.font = m_text.getFontFromFile("assets/VeraMoBI.ttf", fontsize);

		FTMarkup faint = normal;
		faint.foreground_color.r = 0.35;
		faint.foreground_color.g = 0.35;
		faint.foreground_color.b = 0.35;

		FTMarkup error = normal;
		error.foreground_color.r = 1.00;
		error.foreground_color.g = 0.00;
		error.foreground_color.b = 0.00;

		FTMarkup warning = normal;
		warning.foreground_color.r = 1.00;
		warning.foreground_color.g = 0.50;
		warning.foreground_color.b = 0.50;

		FTMarkup output = normal;
		output.foreground_color.r = 0.00;
		output.foreground_color.g = 0.00;
		output.foreground_color.b = 1.00;

		m_markups[c_markup_normal] = normal;
		m_markups[c_markup_error] = error;
		m_markups[c_markup_warning] = warning;
		m_markups[c_markup_output] = output;
		m_markups[c_markup_faint] = faint;
		m_markups[c_markup_bold] = bold;
		m_markups[c_markup_italic] = italic;
		m_markups[c_markup_bold_italic] = bold_italic;
	}

	void addGlyph(const char *current, const char *previous, FTMarkup *markup)
	{
		const auto *glyph = markup->font->loadGlyph(current);
		if (previous) {
			m_pen.x += glyph->getKerning(previous);
		}
		float r = markup->foreground_color.r;
		float g = markup->foreground_color.g;
		float b = markup->foreground_color.b;
		float a = markup->foreground_color.a;
		float x0 = m_pen.x + glyph->offset_x;
		float y0 = m_pen.y + glyph->offset_y;
		float x1 = x0 + glyph->width;
		float y1 = y0 - glyph->height;
		float s0 = glyph->s0;
		float t0 = glyph->t0;
		float s1 = glyph->s1;
		float t1 = glyph->t1;

		int32_t indices[] = {0, 1, 2, 0, 2, 3};
		FTText::Vertex vertices[] = {
		        {x0, y0, 0, s0, t0, r, g, b, a, 0, 0},
		        {x0, y1, 0, s0, t1, r, g, b, a, 0, 0},
		        {x1, y1, 0, s1, t1, r, g, b, a, 0, 0},
		        {x1, y0, 0, s1, t0, r, g, b, a, 0, 0},
		};
		m_text.addVertices(vertices, 4, indices, 6);

		m_pen.x += glyph->advance_x;
		m_pen.y += glyph->advance_y;
	}

	void render()
	{
		char *cur_char;
		char *prev_char;

		int viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		m_pen.x = 0;
		m_pen.y = viewport[3];
		m_text.clear();

		int cursor_x = m_pen.x;
		int cursor_y = m_pen.y;

		FTMarkup markup;

		// console_t buffer
		markup = m_markups[c_markup_faint];
		m_pen.y -= markup.font->m_height;

		for (auto i = 0u; i < m_lines.size(); ++i) {
			auto &text = m_lines.at(i);
			if (text.length() > 0) {
				const char *cur_char = text.c_str();
				const char *prev_char = nullptr;

				addGlyph(cur_char, prev_char, &markup);
				prev_char = cur_char;
				for (auto index = 1u; index < text.length() - 1; ++index) {
					cur_char = text.c_str() + index;
					addGlyph(cur_char, prev_char, &markup);
					prev_char = cur_char;
				}
			}
			m_pen.y -= markup.font->m_height - markup.font->m_linegap;
			m_pen.x = 0;
			cursor_x = m_pen.x;
			cursor_y = m_pen.y;
		}

		// Prompt
		markup = m_markups[c_markup_bold];
		if (strlen(m_prompt) > 0) {
			cur_char = m_prompt;
			prev_char = NULL;
			addGlyph(cur_char, prev_char, &markup);
			prev_char = cur_char;
			for (auto index = 1u; index < strlen(m_prompt); ++index) {
				cur_char = m_prompt + index;
				addGlyph(cur_char, prev_char, &markup);
				prev_char = cur_char;
			}
		}
		cursor_x = (int)m_pen.x;

		// Input
		markup = m_markups[c_markup_normal];
		if (strlen(m_inputs) > 0) {
			cur_char = m_inputs;
			prev_char = NULL;
			addGlyph(cur_char, prev_char, &markup);
			prev_char = cur_char;
			if (m_cursor > 0) {
				cursor_x = (int)m_pen.x;
			}
			for (auto index = 1u; index < strlen(m_inputs); ++index) {
				cur_char = m_inputs + index;
				addGlyph(cur_char, prev_char, &markup);
				prev_char = cur_char;
				if (index < m_cursor) {
					cursor_x = (int)m_pen.x;
				}
			}
		}

		if (m_lines.size() || m_prompt[0] != '\0' || m_inputs[0] != '\0') {
			m_text.upload();
		}

		// Cursor (we use the black character (NULL) as texture )
		const auto *glyph = markup.font->loadGlyph(NULL);
		float r = markup.foreground_color.r;
		float g = markup.foreground_color.g;
		float b = markup.foreground_color.b;
		float a = markup.foreground_color.a;
		float x0 = cursor_x + 1;
		float y0 = cursor_y + markup.font->m_descender;
		float x1 = cursor_x + 2;
		float y1 = y0 + markup.font->m_height - markup.font->m_linegap;
		float s0 = glyph->s0;
		float t0 = glyph->t0;
		float s1 = glyph->s1;
		float t1 = glyph->t1;
		int32_t indices[] = {0, 1, 2, 0, 2, 3};
		FTText::Vertex vertices[] = {
		        {x0, y0, 0, s0, t0, r, g, b, a, 0.0f, 0.0f},
		        {x0, y1, 0, s0, t1, r, g, b, a, 0.0f, 0.0f},
		        {x1, y1, 0, s1, t1, r, g, b, a, 0.0f, 0.0f},
		        {x1, y0, 0, s1, t0, r, g, b, a, 0.0f, 0.0f}
                };
		m_text.addVertices(vertices, 4, indices, 6);

		m_text.draw(GL_TRIANGLES);
	}

	void connect(const char *signal, void (*handler)(char *))
	{
		if (strcmp(signal, "activate") == 0) {
			m_handlers[c_signal_activate] = handler;
		}
		else if (strcmp(signal, "complete") == 0) {
			m_handlers[c_signal_complete] = handler;
		}
		else if (strcmp(signal, "history-next") == 0) {
			m_handlers[c_signal_history_next] = handler;
		}
		else if (strcmp(signal, "history-prev") == 0) {
			m_handlers[c_signal_history_prev] = handler;
		}
	}

	void print(const std::string &text)
	{
		if (m_lines.size() == 0) {
			m_lines.resize(1);
		}
		auto current = m_lines.back();
		m_lines.pop_back();
		for (auto &c: text) {
			current += c;
			if (c == '\n') {
				m_lines.push_back(current);
				current = "";
			}
		}
		m_lines.push_back(current);
	}

	void process(const char *action, const uint8_t key)
	{
		size_t len = strlen(m_inputs);

		if (strcmp(action, "type") == 0) {
			if (len < e_max_line_length) {
				memmove(m_inputs + m_cursor + 1, m_inputs + m_cursor, (len - m_cursor + 1));
				m_inputs[m_cursor] = key;
				m_cursor++;
			}
			else {
				fprintf(stderr, "Input buffer is full\n");
			}
		}
		else {
			if (strcmp(action, "enter") == 0) {
				if (m_handlers[c_signal_activate]) {
					(*m_handlers[c_signal_activate])(m_inputs);
				}
				print(m_prompt);
				print(m_inputs);
				print("\n");
				m_inputs[0] = '\0';
				m_cursor = 0;
			}
			else if (strcmp(action, "right") == 0) {
				if (m_cursor < strlen(m_inputs)) {
					m_cursor += 1;
				}
			}
			else if (strcmp(action, "left") == 0) {
				if (m_cursor > 0) {
					m_cursor -= 1;
				}
			}
			else if (strcmp(action, "delete") == 0) {
				memmove(m_inputs + m_cursor, m_inputs + m_cursor + 1, (len - m_cursor));
			}
			else if (strcmp(action, "backspace") == 0) {
				if (m_cursor > 0) {
					memmove(m_inputs + m_cursor - 1, m_inputs + m_cursor,
					        (len - m_cursor + 1));
					m_cursor--;
				}
			}
			else if (strcmp(action, "kill") == 0) {
				if (m_cursor < len) {
					strcpy(m_killrings, m_inputs);
					m_inputs[m_cursor] = '\0';
					fprintf(stderr, "Kill ring: %s\n", m_killrings);
				}
			}
			else if (strcmp(action, "yank") == 0) {
				size_t l = strlen(m_killrings);
				if ((len + l) < e_max_line_length) {
					memmove(m_inputs + m_cursor + l, m_inputs + m_cursor, (len - m_cursor));
					memcpy(m_inputs + m_cursor, m_killrings, l);
					m_cursor += l;
				}
			}
			else if (strcmp(action, "home") == 0) {
				m_cursor = 0;
			}
			else if (strcmp(action, "end") == 0) {
				m_cursor = strlen(m_inputs);
			}
			else if (strcmp(action, "clear") == 0) {
			}
			else if (strcmp(action, "history-prev") == 0) {
				if (m_handlers[c_signal_history_prev]) {
					(*m_handlers[c_signal_history_prev])(m_inputs);
				}
			}
			else if (strcmp(action, "history-next") == 0) {
				if (m_handlers[c_signal_history_next]) {
					(*m_handlers[c_signal_history_next])(m_inputs);
				}
			}
			else if (strcmp(action, "complete") == 0) {
				if (m_handlers[c_signal_complete]) {
					(*m_handlers[c_signal_complete])(m_inputs);
				}
			}
		}
	}

	static void consoleActivate(char *inputs) { fprintf(stderr, "Activate callback : %s\n", inputs); }

	static void complete(char *inputs) { fprintf(stderr, "Complete callback : %s\n", inputs); }

	static void historyPrev(char *inputs) { fprintf(stderr, "History prev callback : %s\n", inputs); }

	static void historyNext(char *inputs) { fprintf(stderr, "History next callback : %s\n", inputs); }

	void init() override
	{
		auto pix_width = 600;
		float fontsize = 13.0 * pix_width / 600;

		m_controlKeyHandled = 0;

		init(fontsize);
		print("OpenGL Freetype console\n"
		      "Copyright 2011 Nicolas P. Rougier. All rights reserved.\n \n");
		connect("activate", consoleActivate);
		connect("complete", complete);
		connect("history-prev", historyPrev);
		connect("history-next", historyNext);

		spu_graphics_get("pad", &m_pad);
	}

	void doDisplay() override
	{
		m_text.u_textscreen = Mat4f::projection(0, width(), 0, height(), -1, 1, false);

		padread();
		render();
	}

	void padread()
	{
		m_prev = m_curr;
		m_curr = *m_pad;

		if (m_curr.key && m_curr.key != m_prev.key) {
			switch (m_curr.key) {
			case SpuPad::e_home: process("home", 0); break;
			case SpuPad::e_delete: process("delete", 0); break;
			case SpuPad::e_end: process("end", 0); break;
			case SpuPad::e_backspace: process("backspace", 0); break;
			case SpuPad::e_tab: process("complete", 0); break;
			case SpuPad::e_enter: process("enter", 0); break;
			case SpuPad::e_escape: process("escape", 0); break;
			case SpuPad::e_up: process("history-prev", 0); break;
			case SpuPad::e_down: process("history-next", 0); break;
			case SpuPad::e_left: process("left", 0); break;
			case SpuPad::e_right: process("right", 0); break;
			default: break;
			}

			if (m_curr.key_ctrl) {
				switch (m_curr.key) {
				case 'K':
					m_controlKeyHandled = 1;
					process("kill", 0);
					break;
				case 'L':
					m_controlKeyHandled = 1;
					process("clear", 0);
					break;
				case 'Y':
					m_controlKeyHandled = 1;
					process("yank", 0);
					break;
				default: break;
				}
			}
		}
		if (m_curr.code_count != m_prev.code_count) {
			if (!m_curr.key_ctrl && isprint(m_curr.code)) {
				process("type", m_curr.code);
			}
		}
	}

	void errorCallback(int error, const char *description)
	{
		(void)error;
		fputs(description, stderr);
	}
};  // Console
App *createConsole() { return new Console(); }
}  // namespace spu
