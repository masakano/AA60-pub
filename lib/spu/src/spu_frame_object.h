//
// FrameObject :
//
#pragma once

#include "spu_frame_viewport.h"
#include "spu_frame_background.h"
#include "spu_frame_attach.h"
#include "multisample_resolver.h"

namespace spu::libspu::spu_frame {

const Attrs c_attachPointAttrs = {
        {"color0",        GL_COLOR_ATTACHMENT0       },
        {"color1",        GL_COLOR_ATTACHMENT1       },
        {"color2",        GL_COLOR_ATTACHMENT2       },
        {"color3",        GL_COLOR_ATTACHMENT3       },
        {"color4",        GL_COLOR_ATTACHMENT4       },
        {"color5",        GL_COLOR_ATTACHMENT5       },
        {"color6",        GL_COLOR_ATTACHMENT6       },
        {"color7",        GL_COLOR_ATTACHMENT7       },
        {"color8",        GL_COLOR_ATTACHMENT8       },
        {"color9",        GL_COLOR_ATTACHMENT9       },
        {"depth",         GL_DEPTH_ATTACHMENT        },
        {"stencil",       GL_STENCIL_ATTACHMENT      },
        {"depth_stencil", GL_DEPTH_STENCIL_ATTACHMENT},
};

const Attrs c_drawbufferAttrs = {
        {"color0", GL_COLOR_ATTACHMENT0},
        {"color1", GL_COLOR_ATTACHMENT1},
        {"color2", GL_COLOR_ATTACHMENT2},
        {"color3", GL_COLOR_ATTACHMENT3},
        {"color4", GL_COLOR_ATTACHMENT4},
        {"color5", GL_COLOR_ATTACHMENT5},
        {"color6", GL_COLOR_ATTACHMENT6},
        {"color7", GL_COLOR_ATTACHMENT7},
        {"color8", GL_COLOR_ATTACHMENT8},
        {"color9", GL_COLOR_ATTACHMENT9},
};

const Attrs c_defaultAttrs = {
        {"default_width",                  GL_FRAMEBUFFER_DEFAULT_WIDTH                 },
        {"default_height",                 GL_FRAMEBUFFER_DEFAULT_HEIGHT                },
        {"default_layers",                 GL_FRAMEBUFFER_DEFAULT_LAYERS                },
        {"default_samples",                GL_FRAMEBUFFER_DEFAULT_SAMPLES               },
        {"default_fixed_sample_locations", GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS},
};

class FrameObject : public Object {
public:
	FrameObject(const Attrs &attrs)
	{
		attrs.peek("viewport", "use 'viewport0' instead");
		attrs.peek("scissor", "use 'scissor0' instead");

		auto frame_id = attrs.getf<uint32_t>("frame_id");

		if (frame_id.hit) {
			m_handle.id = frame_id.value;
		}
		else {
			uint32_t ui_id;
			F(glGenFramebuffers, 1, &ui_id);
			m_handle.id = ui_id;
		}
		bind();

		// drawbuffer (must be here)
		if (m_handle.id) {
			std::vector<GLenum> drawbuffers;
			for (const auto &attr: attrs) {
				int32_t drawbuffer;
				if ((drawbuffer = c_drawbufferAttrs.get(attr.key(), 0)) != 0) {
					drawbuffers.push_back(drawbuffer);
				}
			}
			F(glDrawBuffers, drawbuffers.size(), drawbuffers.data());
		}
		else {
			m_backgrounds.activate(GL_COLOR_ATTACHMENT0);
			m_backgrounds.activate(GL_DEPTH_ATTACHMENT);
			m_backgrounds.activate(GL_STENCIL_ATTACHMENT);
		}

		// defaults
		for (auto &default_attr: c_defaultAttrs) {
			auto attr_value = attrs.getf<uint32_t>(default_attr.key());
			if (attr_value.hit) {
				F(glFramebufferParameteri, GL_FRAMEBUFFER, int32_t(default_attr),
				  attr_value.value);
			}
		}

		m_layer = attrs.get("layer", m_layer);  // global
		m_level = attrs.get("level", m_level);  // global

		for (auto &attach_point_attr: c_attachPointAttrs) {
			auto key = attach_point_attr.key();
			auto texture = Handle(attrs.get(key, 0u));

			if (texture.ui != 0u) {
				auto attach_point = int32_t(attach_point_attr);
				auto attach_attrs = attrs.select((std::string(key.c_str()) + ".").c_str());

				auto local_layer = attach_attrs.get("layer", m_layer);
				auto local_level = attach_attrs.get("level", m_level);

				Attach attach(key, attach_point, texture);
				attach.attach(local_layer, local_level);
				m_attaches.push_back(attach);
				m_backgrounds.activate(attach_point);
			}
		}
		checkStatus();

		std::vector<uint32_t> src_color_textures;
		std::vector<uint32_t> dst_color_textures;
		std::vector<uint32_t> src_depth_textures;
		std::vector<uint32_t> dst_depth_textures;
		auto is_resolve = false;

		for (auto &attach_point_attr: c_attachPointAttrs) {
			auto attach_key = attach_point_attr.key();
			auto resolve_attach_key = std::string(attach_key.c_str()) + ".resolve";
			auto dst_texture = attrs.getf<uint32_t>(resolve_attach_key.c_str());

			if (dst_texture.hit) {
				auto src_texture = attrs.getf<uint32_t>(attach_key);
				aux_error(!src_texture.hit, "%s not found\n", attach_key.c_str());

				if (attach_key == "depth") {
					src_depth_textures.push_back(src_texture.value);
					dst_depth_textures.push_back(dst_texture.value);
				}
				else {
					src_color_textures.push_back(src_texture.value);
					dst_color_textures.push_back(dst_texture.value);
				}
				m_resolveAttrs.append(resolve_attach_key.c_str(), dst_texture.value);
				m_resolveAttrs.preserve();
				is_resolve = true;
			}
		}
		if (is_resolve) {
			m_resolver.init(
			        dst_color_textures, src_color_textures, dst_depth_textures, src_depth_textures);
		}

		m_viewports.set(attrs);
		m_backgrounds.set(attrs);  // must be at last

		if (top()) top()->use();  // restore (skip top)
	}

	~FrameObject() override
	{
		auto ui_id = uint32_t(m_handle.id);
		F(glDeleteFramebuffers, 1, &ui_id);
	}

	void bind() { F(glBindFramebuffer, GL_FRAMEBUFFER, m_handle.id); }

	void use()
	{
		bind();
		m_viewports.send();
	}

	void begin()
	{
		auto prev = top();
		push(this);
		if (this != prev) {
			use();
		}
	}

	void end()
	{
		for (auto &attach: m_attaches) {
			attach.update();
		}
		pop();
		auto prev = top();
		if (prev && this != prev) {
			prev->use();
		}
		m_resolver.compute();
	}

	void clear()
	{
		for (auto &attach: m_attaches) {
			if (!attach.isClearable(m_layer)) {
				report();
				spu_message(0, "cannot clear all layered frame (layer=-1)\n");
				aux_error(true, "specify the layer number, then clear\n");
			}
		}
		if (top() != this) {
			use();
		}
		m_backgrounds.clear();
		if (top() && top() != this) {
			top()->use();  // resume
		}
	}

	void set(const Attrs &attrs)
	{
		attrs.peek("viewport", "use 'viewport0' instead");
		attrs.peek("scissor", "use 'scissor0' instead");
		attrs.peek("resolve_mask", "use '[target].do_resolve' instead");
		attrs.peek("resolve_rect", "deprecated");
		attrs.peek("do_resolve", "deprecated");

		m_viewports.set(attrs);
		m_backgrounds.set(attrs);

		auto is_changed = false;
		auto prev_layer = m_layer;
		auto prev_level = m_level;

		m_layer = attrs.get("layer", m_layer);
		m_level = attrs.get("level", m_level);

		if (m_layer != prev_layer || m_level != prev_level) {
			for (auto &attach: m_attaches) {
				auto key = attach.key();
				auto attach_attrs = attrs.select((std::string(key.c_str()) + ".").c_str());
				bind();
				attach.attach(m_layer, m_level);
			}
			is_changed = true;
		}

		if (is_changed) {
			checkStatus();
			top()->bind();
		}

		// reset immediately
		if (this == top()) {
			m_viewports.send();
		}
	}

	int32_t get(const hash32_t &key, void *value)
	{
		auto ret = 0;

		if ((ret = m_viewports.get(key, value))) {
			return ret;
		}
		if ((ret = m_backgrounds.get(key, value))) {
			return ret;
		}
		if ((ret = getvalue(key, value, "layer"_h32, m_layer))) {
			return ret;
		}
		if ((ret = getvalue(key, value, "level"_h32, m_level))) {
			return ret;
		}
		for (auto &attach: m_attaches) {
			if ((ret = getvalue(key, value, attach.key(), attach.get()))) {
				return ret;
			}
		}
		return ret;
	}

	void report()
	{
		aux_printf("frame [%05x] :\n", m_handle.id);
		aux_printf("    layer         : %2d\n", m_layer);
		aux_printf("    level         : %2d\n", m_level);
		aux_printf("    viewport:\n");
		m_viewports.report();

		aux_printf("    background:\n");
		m_backgrounds.report();

		aux_printf("    attachment:\n");
		for (auto &attach: m_attaches) {
			attach.report();
		}

		for (auto &attr: m_resolveAttrs) {
			aux_printf("\t%-16s : %08x\n", attr.key().c_str(), uint32_t(attr));
		}
		aux_printf("    defaults:\n");
		reportDefault();
	}

	Backgrounds &backgrounds() { return m_backgrounds; }

	static void push(FrameObject *object) { return ms_stack().push_back(object); }
	static void pop()
	{
		assert(!ms_stack().empty());
		ms_stack().pop_back();
	}
	static FrameObject *top() { return ms_stack().empty() ? nullptr : ms_stack().back(); }

	static void erase(FrameObject *object)
	{
		if (std::find(ms_stack().begin(), ms_stack().end(), object) != ms_stack().end()) {
			spu_message(0, "disposing frame is on the stack\n");
			object->report();
			auto &stack = ms_stack();
			stack.erase(std::remove(stack.begin(), stack.end(), object), stack.end());
		}
	}
	static size_t stackcount() { return ms_stack().size(); }

private:
	std::vector<Attach> m_attaches;
	Viewports m_viewports;
	Backgrounds m_backgrounds;
	MultisampleResolver m_resolver;
	Attrs m_resolveAttrs;  // report() only
	int32_t m_layer = -1;
	int32_t m_level = 0;

	static std::vector<FrameObject *> &ms_stack()
	{
		static std::vector<FrameObject *> v;
		return v;
	}

	void checkStatus()
	{
		if (m_attaches.empty()) {
			return;
		}

		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			for (auto &attach: m_attaches) {
				attach.report();
			}
			aux_error(true, "bad attachment [%s]\n", opengl_const(status));
		}
	}

	void reportDefault()
	{
		if (m_handle.id != 0u) {
			bind();
			for (auto &default_attr: c_defaultAttrs) {
				auto value = -1;
				F(glGetFramebufferParameteriv, GL_FRAMEBUFFER, int32_t(default_attr), &value);
				aux_printf("\t%-16s : %08x\n", default_attr.key(), value);
			}
			top()->bind();
		}
	}
};
}  // namespace spu::libspu::spu_frame
