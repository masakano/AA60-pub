//
// ManagerObject :
//
#include "spu_object.h"
#include "EGL/dev.h"
#include "GLFW/dev.h"

using namespace spu::libspu;

namespace spu {
namespace {

struct VSyncCallback {
	Attr::callback_t func;
	void *arg;
	VSyncCallback(const Attr::callback_t &func, void *arg) : func(func), arg(arg) {}
	bool operator==(const VSyncCallback &cb) const { return cb.func == func && cb.arg == arg; }
};

enum Status {
	e_uninitialized = 0,
	e_active,
	e_down,
};

DeviceConfig s_config;
glfw::GlfwDevice s_glfw_device;
egl::EglDevice s_egl_device;
Device *s_device = nullptr;
std::vector<VSyncCallback> s_cb_vsyncs;

int32_t s_print_id = -1;
uint32_t s_status = e_uninitialized;
bool s_is_flush_at_swap = false;
// bool s_is_srgb_encode = true;

}  // namespace

void spu_graphics_init(const Attrs &attrs)
{
	SpuMessage::setMainThreadId();
	set_microsec(0);
	SET();

	auto is_safe = attrs.get("safe", 0);
	auto is_iconic = attrs.get("iconic", 0);
	auto is_fullscreen = attrs.get("fullscreen", 0);
	const char *title = attrs.get("title", "tea for two");
	const char *device = attrs.get("device", "glfw");

	if (s_status == e_uninitialized) {
		if (strcmp(device, "glfw") == 0) {
			s_device = &s_glfw_device;
		}
		else if (strcmp(device, "egl") == 0) {
			s_device = &s_egl_device;
		}
		else {
			aux_error(true, "unknown device \"%s\"\n", device);
		}

		s_device->init(&s_config);

		Attrs dev_attrs = {
		        {"samples",      4            },
                        {"red_size",     8            },
		        {"green_size",   8            },
                        {"blue_size",    8            },
		        {"alpha_size",   8            },
                        {"depth_size",   32           },
		        {"stencil_size", 8            },
                        {"title",        title        },
		        {"iconic",       is_iconic    },
                        {"fullscreen",   is_fullscreen},
		};
		// attrs.report("spu_graphics_init");
		s_device->open(attrs + dev_attrs);
		s_status = e_down;
	}
	spu_graphics_set(attrs);

	if (s_status == e_down) {
		ManagerObject::startupAll();
		if (!is_safe) {
			s_print_id = spu_print_new(attrs.select("stdout."));
		}
		s_status = e_active;
	}
}

void spu_graphics_shutdown()
{
	SET();
	if (s_status != e_down) {
		if (s_print_id >= 0) {
			spu_print_delete(s_print_id);
		}
		ManagerObject::shutdownAll();
		auto c = s_config.pads[0].swap_count;
		auto w = s_config.pads[0].winsize[0];
		auto h = s_config.pads[0].winsize[1];

		s_config.pads[0] = SpuPad();
		s_config.pads[0].swap_count = c;
		s_config.pads[0].winsize[0] = w;
		s_config.pads[0].winsize[1] = h;
		s_status = e_down;
		s_print_id = -1;
	}
}

std::vector<std::vector<uint32_t>> spu_graphics_get_alives_list()
{
	SET();
	return ManagerObject::getAlivesList();
}

void spu_graphics_prune(const std::vector<std::vector<uint32_t>> &alives_list)
{
	SET();
	return ManagerObject::prune(alives_list);
}

void spu_graphics_set(const Attrs &attrs)
{
	MSG();

	s_config.interval = attrs.get("interval", s_config.interval);
	s_config.use_pad = attrs.get("use_pad", s_config.use_pad);
	s_is_flush_at_swap = attrs.get("flush_at_swap", s_is_flush_at_swap);
	// s_is_srgb_encode = attrs.get("srgb_encode", s_is_srgb_encode);

	auto add_cb_vsync = attrs.getf<Attr::callback_t>("add_cb_vsync");
	auto del_cb_vsync = attrs.getf<Attr::callback_t>("del_cb_vsync");
	auto *cb_vsync_arg = attrs.get<void *>("cb_vsync_arg", nullptr);

	if (add_cb_vsync.hit) {
		auto callback = VSyncCallback(add_cb_vsync.value, cb_vsync_arg);
		auto cp = find(begin(s_cb_vsyncs), end(s_cb_vsyncs), callback);
		if (cp == end(s_cb_vsyncs)) {
			s_cb_vsyncs.push_back(callback);
		}
	}
	if (del_cb_vsync.hit) {
		auto callback = VSyncCallback(del_cb_vsync.value, cb_vsync_arg);
		auto cp = find(begin(s_cb_vsyncs), end(s_cb_vsyncs), callback);
		if (cp != end(s_cb_vsyncs)) {
			s_cb_vsyncs.erase(cp);
		}
	}
}

bool spu_graphics_swap()
{
	SET();
	assert(s_device);

	for (auto cb: s_cb_vsyncs) {
		cb.func(cb.arg);
	}
	if (s_print_id >= 0) {
		spu_print_end(s_print_id);
		spu_print_begin(s_print_id);
	}
	if (s_is_flush_at_swap) {
		F(glFlush);
	}

	auto status = s_device->swap();
	if (status == true) {
		s_device->poll();
	}
	return status;
}

int32_t spu_graphics_get(const hash32_t &key, void *value)
{
	auto n = 0;
	if ((n = getvalue(key, value, "pad"_h32, &s_config.pads[0]))) {
		return n;
	}
	if ((n = getvalue(key, value, "interval"_h32, s_config.interval))) {
		return n;
	}
	if ((n = getvalue(key, value, "use_pad"_h32, &s_config.use_pad))) {
		return n;
	}
	if ((n = getvalue(key, value, "device_status"_h32, s_status))) {
		return n;
	}
	if ((n = getvalue(key, value, "flush_at_swap"_h32, s_is_flush_at_swap))) {
		return n;
	}
	/*
	if ((n = getvalue(key, value, "srgb_encode"_h32, s_is_srgb_encode))) {
	        return n;
	}
	*/
	if ((n = s_device->get(key, value))) {
		return n;
	}
	return n;
}

void spu_graphics_report(const char *str)
{
	if (str && *str) {
		auto line = std::string(strlen(str) + 1, '-');
		aux_printf("%s\n", line.c_str());
		aux_printf("%s:\n", str);
		aux_printf("%s\n", line.c_str());
	}
	ManagerObject::reportAll();
}

void spu_graphics_memory_barrier(uint32_t mode) { F(glMemoryBarrier, mode); }

}  // namespace spu
