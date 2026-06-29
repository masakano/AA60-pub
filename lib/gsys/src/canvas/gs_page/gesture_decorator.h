//
// GestureDecorator :
//
#pragma once
#include <gsys/canvas/gs_page.h>
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_canvas::gs_page {

class GestureDecorator : public GsPage::IDecorator {
public:
	enum {
		e_fkey_show_stdout = SpuPad::e_f5,
		e_fkey_save_screen = SpuPad::e_f6,
		e_fkey_reload_shader = SpuPad::e_f7,
		e_fkey_organize_tweakbar = SpuPad::e_f8,
		e_fkey_show_tweakbar = SpuPad::e_f9,
		e_fkey_save_tweakbar = SpuPad::e_f10,
		e_fkey_load_tweakbar = SpuPad::e_f11,
		e_fkey_report_resources = SpuPad::e_f12,
	};

	GestureDecorator(GsPage &page) : m_page(page)
	{
		auto *gesture = new SpuGesture(nullptr);

		gesture->native()->swap_count = 0;

		m_keys[e_fkey_show_stdout] = {"show stdout", 0};
		m_keys[e_fkey_save_screen] = {"save screen", 0};
		m_keys[e_fkey_reload_shader] = {"reload shader", 0};
		m_keys[e_fkey_organize_tweakbar] = {"organize tweakbars", 0};
		m_keys[e_fkey_show_tweakbar] = {"show tweakbars", 1};
		m_keys[e_fkey_save_tweakbar] = {"save tweakbars", 0};
		m_keys[e_fkey_load_tweakbar] = {"load tweakbars", 0};
		m_keys[e_fkey_report_resources] = {"report resources", 0};

		auto window_w = gesture->native()->winsize[0];
		auto window_h = gesture->native()->winsize[1];
		m_page.getViewports().at(0) = Rectf(0, 0, window_w, window_h);
		m_page.adjustAspect(0);
		m_page.replaceGesture(gesture);
	}

	void begin() override { m_page.getGesture()->update(); }

	void end() override
	{
		auto *gesture = m_page.getGesture();
		if (gesture->keyPressed(m_escapeCode)) {
			aux_message(0, "terminated: code=0x%02x\n", m_escapeCode);
			m_page.setProperty("alive", 0);
		}
		if (gesture->curr().key_shift && gesture->pressed()) {
			action(gesture->curr().key);
		}
		display();
	}

	void set(const Attrs &attrs) override
	{
		auto show_tweakbar = attrs.getf("show_tweakbar", 1);
		if (show_tweakbar.hit) {
			m_keys[e_fkey_show_tweakbar].count = show_tweakbar.value;
		}
		m_escapeCode = attrs.get("escape", m_escapeCode);
	}

private:
	struct FunctionKey {
		const char *help;
		uint32_t count = 0;
	};
	std::map<int16_t, FunctionKey> m_keys;
	uint32_t m_escapeCode = 0x1b;
	GsPage &m_page;

	void action(int16_t key)
	{
		auto it = m_keys.find(key);
		if (it == std::end(m_keys)) {
			return;
		}
		it->second.count++;

		switch (key) {
		case e_fkey_show_stdout: {
			break;
		}
		case e_fkey_show_tweakbar: {
			break;
		}
		case e_fkey_save_screen: {
			namespace fs = std::filesystem;
			fs::path dir = "rec";
			if (!fs::exists(dir)) {
				fs::create_directory(dir);
			}
			auto image_path = string_printf("rec/i%05d.bmp", m_page.getSeconds().count());
			auto depth_path = string_printf("rec/d%05d.bmp", m_page.getSeconds().count());

			spu_texture_save(0, image_path.data(), "color"_h32);
			spu_texture_save(0, depth_path.data(), "depth"_h32);
			break;
		}
		case e_fkey_reload_shader: {
			aux_message(0, "reload shader...\n");
			SpuShader::reload();
			break;
		}
		case e_fkey_organize_tweakbar: {
			aux_message(0, "organize tweakbar...\n");
			gs_node::gui::Tweakbar::organize();
			break;
		}
		case e_fkey_save_tweakbar: {
			aux_message(0, "save tweakbar...\n");
			gs_node::gui::Tweakbar::saveAll();
			break;
		}
		case e_fkey_load_tweakbar: {
			aux_message(0, "load tweakbar...\n");
			gs_node::gui::Tweakbar::loadAll();
			break;
		}
		case e_fkey_report_resources: {
			aux_message(0, "report resources...\n");
			spu_graphics_report(m_page.name().c_str());
			report();
			break;
		}
		default: aux_error(true, "unhandled key (%d)\n", key);
		}
	}

	void display()
	{
		if (m_keys[e_fkey_show_stdout].count % 3 == 1) {
			spu_printf(
			        0, "total: %.2f msec %.2f fps\n", m_page.getSeconds().delta() * 1000.0,
			        1.0f / m_page.getSeconds().delta());
		}
		if (m_keys[e_fkey_show_stdout].count % 3 == 2) {
			spu_printf(0, "keys (with shift):\n");
			for (auto &pair: m_keys) {
				spu_printf(
				        0, "    %-3s %2d %s\n", SpuGesture::symbolmap()[pair.first],
				        pair.second.count, pair.second.help);
			}
		}
		// printf("show_tweakbar = %d\n", m_keys[e_fkey_show_tweakbar].count % 2);
		m_page.setProperty(m_page.e_show_tweakbar, m_keys[e_fkey_show_tweakbar].count % 2);
	}

	void report()
	{
		auto all_objects = GsObject::aliveObjects();

		printf("nodes:\n");
		for (const auto &object: all_objects) {
			auto node = dynamic_cast<GsNode *>(object);
			if (node && node->getPainter()) {
				auto painter = node->getPainter();
				printf("    %s\n", node->prettyName().c_str());
				printf("\t%2d Painter %s\n", painter->id(), painter->prettyName().c_str());

				for (auto &pair: painter->getShaders()) {
					auto &type = pair.first;
					auto &shader = pair.second;
					if (shader.id()) {
						printf("\t    %2d Shader %s \n", shader.id(), type.c_str());
					}
				}
			}
		}
		printf("canvas:\n");
		for (const auto &object: all_objects) {
			auto canvas = dynamic_cast<GsCanvas *>(object);
			if (canvas && canvas->isReal()) {
				auto viewport = canvas->viewport(0);
				printf("    %2d %-24s %5.0f %5.0f\n", canvas->id(),
				       canvas->prettyName().c_str(), viewport.sx, viewport.sy);
				auto shader = canvas->getShader();
				if (shader.id()) {
					printf("\t    %2d Shader\n", shader.id());
				}
			}
		}
		printf("painter:\n");
		for (const auto &object: all_objects) {
			auto painter = dynamic_cast<GsPainter *>(object);
			if (painter) {
				printf("    %2d %s\n", painter->id(), painter->prettyName().c_str());

				for (auto &pair: painter->getShaders()) {
					auto &type = pair.first;
					auto &shader = pair.second;
					if (shader.id()) {
						printf("\t    %2d Shader %s \n", shader.id(), type.c_str());
					}
				}
			}
		}
	}
};
}  // namespace spu::gs_canvas::gs_page
