//
// Launcher :
//
#include "app.h"
#include <spu++/spu_backoffice.h>
#include <gsys/object.h>

namespace spu {
class Launcher {
public:
	void init() { spu_graphics_get("pad", &m_pad); }
	void add(const char *desc, App *app)
	{
		m_lists.emplace_back(desc, app);
		app->init();
	}

	void display()
	{
		auto &list = m_lists.at(m_id);
		list.app->display();
	}

	bool menu()
	{
		if (m_prev.key == 0 && m_pad->key == SpuPad::e_down) {
			m_id = (m_id + 1) % m_lists.size();
		}
		if (m_prev.key == 0 && m_pad->key == SpuPad::e_up) {
			m_id = (m_id + m_lists.size() - 1) % m_lists.size();
		}
		m_prev = *m_pad;

		spu_frame_set(-1, "viewport0", vec4f_t(0, 0, c_win_sx, c_win_sy));
		for (auto i = 0u; i < m_lists.size(); i++) {
			spu_printf(0, "%c %s\n", i == m_id ? '*' : ' ', m_lists[i].desc);
		}
		return m_pad->key != SpuPad::e_escape;
	}

	void destroy()
	{
		for (auto &list: m_lists) {
			delete list.app;
		}
		m_lists.clear();
	}

private:
	struct List {
		const char *desc;
		App *app;
	};
	std::vector<List> m_lists;
	uint32_t m_id = 0;
	SpuPad *m_pad = nullptr;
	SpuPad m_prev;
};

extern App *createAnsi();
extern App *createBenchmark();
extern App *createCartoon();
extern App *createConsole();
extern App *createDistanceField();
extern App *createDistanceField2();
extern App *createDistanceField3();
extern App *createFontBasic();
extern App *createGamma();
extern App *createGlyphBasic();
extern App *createLcd();
extern App *createMarkupFont();
extern App *createOutline();
extern App *createSubpixel();
extern App *createTexture();
extern App *createMarkupFont2();
}  // namespace spu

using namespace spu;

int main(int argc, const char **argv)
{
	(void)argc;
	Attrs attrs(argv + 1);
	attrs.trace("main", false);

	const char *post_argv[] = {
	        "-stdout.color", "0.01 0.01 0.01 1.0", "-stdout.origin", "32 700 0 0", "-stdout.pitch", "10",
	        nullptr,
	};
	Attrs post_attrs(post_argv);
	attrs += post_attrs;
	auto *backoffice = new SpuBackoffice(attrs, GsObject::startup, GsObject::shutdown);
	attrs = backoffice->getAttrs();

	// GsObject::startup(attrs);

	Launcher launcher;
	launcher.init();

	launcher.add("ansi", createAnsi());
	launcher.add("benchmark", createBenchmark());
	launcher.add("cartoon", createCartoon());
	launcher.add("console", createConsole());
	launcher.add("distance_field", createDistanceField());
	launcher.add("distance_field2", createDistanceField2());
	launcher.add("distance_field3", createDistanceField3());
	launcher.add("font_basic", createFontBasic());
	launcher.add("gamma", createGamma());
	launcher.add("glyph_basic", createGlyphBasic());
	launcher.add("lcd", createLcd());
	launcher.add("markup_font", createMarkupFont());
	launcher.add("outline", createOutline());
	launcher.add("subpixel", createSubpixel());
	launcher.add("texture", createTexture());
	launcher.add("markup_font2", createMarkupFont2());

	bool is_open = true;
	while (is_open) {
		launcher.display();
		is_open &= launcher.menu();
		is_open &= spu_graphics_swap();  // need check
	}
	launcher.destroy();
	// GsObject::shutdown();  //
	// SpuBackoffice::destroy();
	return 0;
}
