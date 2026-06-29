//
// SpuBackoffice :
//
#pragma once
#include "spu++.h"
#include <ssys/object_registry.h>

namespace spu {
class SharedMemory;
class SpuBackoffice {
public:
public:
	void postproc();

	SpuTexture &getBuffer(const hash32_t &slot)
	{
		if (slot == e_color0) return m_color0;
		if (slot == e_depth_stencil) return m_depthStencil;
		assert(0);
		return m_color0;
	}
	const std::string &senderPath() const { return m_senderPath; }
	const std::string &referencePath() const { return m_referencePath; }

	SpuRenderstate &getRenderstate() { return m_renderstate; }
	Attrs &getAttrs() { return m_attrs; }

	void setSenderEmbedValuePtr(uint32_t *embed_value) { m_senderEmbedValue = embed_value; }

	const hash32_t e_color0 = "color0";
	const hash32_t e_depth_stencil = "depth_stencil";

	class Frame {
	public:
		uint32_t u_color0 = 0;
		uint32_t u_color1 = 0;
		SpuArray &getArray() { return m_array; }
		SpuFrame &getFrame() { return m_frame; }
		void render(const Rectf &viewport0);
		void dispose()
		{
			m_array.dispose();
			m_frame.dispose();
			m_shader.dispose();
		}

	protected:
		SpuFrame m_frame;
		SpuArray m_array;
		SpuShader m_shader;
		void init(const Rectf &viewport0, const char *vert, const char *frag);
	};

	class EchoFrame : public Frame {
	public:
		void init(const Rectf &viewport0);
	};
	EchoFrame m_echoFrame;

	class BistFrame : public Frame {
	public:
		void init(const Rectf &viewport0);
	};
	BistFrame m_bistFrame;

	struct Bist {
		File file;
		uint32_t count = 0;
	};
	Bist m_bist;

	std::string m_senderPath;
	std::string m_referencePath;

	SpuTexture m_stageTexture;
	SpuTexture m_color0;
	SpuTexture m_depthStencil;
	SpuRenderstate m_renderstate;
	Attrs m_attrs;

	uint32_t m_width = 0;
	uint32_t m_height = 0;
	std::string m_basePath;

	uint32_t *m_senderEmbedValue = nullptr;
	SharedMemory *m_shm = nullptr;
	void (*m_sysStartup)(const Attrs &);
	void (*m_sysShutdown)();

	SpuBackoffice(const Attrs &attrs, void (*sys_startup)(const Attrs &), void (*sys_shutdown)());
	~SpuBackoffice();
};

template<class base_t, class app_t> class SpuBackofficeManager : public base_t {
public:
	using registry_t = ObjectRegistry<app_t>;

	void init(const Attrs &attrs)
	{
		Attrs init_attrs = attrs;
		m_backoffice = attrs.get<SpuBackoffice *>("backoffice", nullptr);
		if (m_backoffice) {
			Attrs canvas_attrs = {
			        {"color0.texture_id",        m_backoffice->getBuffer("color0").id()       },
			        {"depth_stencil.texture_id", m_backoffice->getBuffer("depth_stencil").id()},
			};
			init_attrs.prepend(canvas_attrs);
		}
		else {
			aux_message(0, "no backoffice found\n");
		}
		base_t::init(init_attrs);
	}

	app_t *create(const char *name, const Attrs &attrs)
	{
		auto app = registry_t::create(name);
		base_t::begin();
		app->init(attrs);
		base_t::end();
		return app;
	}

	void exec(app_t *app)
	{
		while (app->isAlive()) {
			base_t::begin();
			app->begin();
			app->update();
			if constexpr (requires(app_t *current) { current->render(); }) {
				app->render();
			}
			else {
				app->render();
			}
			app->end();
			base_t::end();
			if (m_backoffice) {
				m_backoffice->postproc();
			}
			if (!spu_graphics_swap()) {
				break;
			}
		}
	}

	static std::vector<std::string> selectedAppNames(const Attrs &attrs)
	{
		auto app_names = registry_t::appNames();
		auto black_lists = extract_from_string(attrs.get("startup.black_list", ""), ":");
		auto top_app_name = attrs.get("c", app_names.at(0).c_str());
		auto is_batch = attrs.get("batch", 0);

		if (std::find(begin(app_names), end(app_names), top_app_name) == std::end(app_names)) {
			aux_printf("no page objects. candidates are:\n");
			registry_t::printAppNames();
			aux_abort();
		}
		if (is_batch) {
			auto prune_and_rotate = [&](std::vector<std::string> &vec, const std::string &key) {
				auto det = [&](const std::string &name) {
					auto &vec = black_lists;
					return std::find(begin(vec), end(vec), name) != end(vec);
				};

				vec.erase(std::remove_if(begin(vec), end(vec), det), end(vec));
				auto it = std::find(begin(vec), end(vec), key);
				if (it != std::end(vec)) {
					vec.erase(begin(vec), it);
				}
				return vec;
			};
			return prune_and_rotate(app_names, top_app_name);
		}
		else {
			return {top_app_name};
		}
	}

protected:
	SpuBackoffice *m_backoffice = nullptr;  // not owner
	static auto &ms_creators() { return ObjectRegistry<app_t>::ms_creators(); }
};

}  // namespace spu
