//
// SpuPad :
//
#include "spu/GL/gl.h"
#include "dev.h"
#include "icon.h"
#include <GLFW/glfw3.h>

namespace spu::libspu::glfw {

namespace {

DeviceConfig *s_dev_config = nullptr;
GLFWwindow *s_window = nullptr;
GLFWmonitor *s_monitor = nullptr;

// SpuPad s_pads[2];  // 0: curr 1: prev
SpuPad s_pad;
vec4i_t s_location = {0, 0, 0, 0};
const char *s_title = nullptr;

uint32_t get_special_key(uint32_t key)
{
	static const std::map<uint32_t, uint32_t> s_special_keymaps = {
	        {GLFW_KEY_KP_0,          '0'                },
	        {GLFW_KEY_KP_1,          '1'                },
	        {GLFW_KEY_KP_2,          '2'                },
	        {GLFW_KEY_KP_3,          '3'                },
	        {GLFW_KEY_KP_4,          '4'                },
	        {GLFW_KEY_KP_5,          '5'                },
	        {GLFW_KEY_KP_6,          '6'                },
	        {GLFW_KEY_KP_7,          '7'                },
	        {GLFW_KEY_KP_8,          '8'                },
	        {GLFW_KEY_KP_9,          '9'                },
	        {GLFW_KEY_KP_EQUAL,      '='                },
	        {GLFW_KEY_KP_MULTIPLY,   '*'                },
	        {GLFW_KEY_KP_ADD,        '+'                },
	        {GLFW_KEY_KP_SUBTRACT,   '-'                },
	        {GLFW_KEY_KP_DECIMAL,    '.'                },
	        {GLFW_KEY_KP_DIVIDE,     '/'                },
	        {GLFW_KEY_KP_ENTER,      '\n'               },

	        {GLFW_KEY_ESCAPE,        SpuPad::e_escape   }, // ascii ESC
	        {GLFW_KEY_DELETE,        SpuPad::e_delete   }, // ascii DEL
	        {GLFW_KEY_ENTER,         SpuPad::e_enter    }, // ascii '\n'
	        {GLFW_KEY_TAB,           SpuPad::e_tab      }, // ascii '\t'
	        {GLFW_KEY_BACKSPACE,     SpuPad::e_backspace}, // ascii '\b'

	        {GLFW_KEY_LEFT,          SpuPad::e_left     },
	        {GLFW_KEY_RIGHT,         SpuPad::e_right    },
	        {GLFW_KEY_DOWN,          SpuPad::e_down     },
	        {GLFW_KEY_UP,            SpuPad::e_up       },
	        {GLFW_KEY_LEFT_SHIFT,    SpuPad::e_lshift   },
	        {GLFW_KEY_RIGHT_SHIFT,   SpuPad::e_rshift   },
	        {GLFW_KEY_LEFT_CONTROL,  SpuPad::e_lctrl    },
	        {GLFW_KEY_RIGHT_CONTROL, SpuPad::e_rctrl    },
	        {GLFW_KEY_LEFT_ALT,      SpuPad::e_lalt     },
	        {GLFW_KEY_RIGHT_ALT,     SpuPad::e_ralt     },
	        {GLFW_KEY_HOME,          SpuPad::e_home     },
	        {GLFW_KEY_PAGE_UP,       SpuPad::e_pageup   },
	        {GLFW_KEY_PAGE_DOWN,     SpuPad::e_pagedown },
	        {GLFW_KEY_END,           SpuPad::e_end      },
	        {GLFW_KEY_F1,            SpuPad::e_f1       },
	        {GLFW_KEY_F2,            SpuPad::e_f2       },
	        {GLFW_KEY_F3,            SpuPad::e_f3       },
	        {GLFW_KEY_F4,            SpuPad::e_f4       },
	        {GLFW_KEY_F5,            SpuPad::e_f5       },
	        {GLFW_KEY_F6,            SpuPad::e_f6       },
	        {GLFW_KEY_F7,            SpuPad::e_f7       },
	        {GLFW_KEY_F8,            SpuPad::e_f8       },
	        {GLFW_KEY_F9,            SpuPad::e_f9       },
	        {GLFW_KEY_F10,           SpuPad::e_f10      },
	        {GLFW_KEY_F11,           SpuPad::e_f11      },
	        {GLFW_KEY_F12,           SpuPad::e_f12      },

	        {GLFW_KEY_CAPS_LOCK,     SpuPad::e_caps_lock},
	};
	auto it = s_special_keymaps.find(key);
	return it == s_special_keymaps.end() ? 0 : it->second;
}

void close_callback(GLFWwindow *window) { glfwSetWindowShouldClose(window, GL_TRUE); }

void resize_callback(GLFWwindow *, int32_t width, int32_t height)
{
	s_location.z = width;
	s_location.w = height;
	s_pad.winsize[0] = width;
	s_pad.winsize[1] = height;
}

void cursor_position_callback(GLFWwindow *, double xpos, double ypos)
{
	s_pad.cursor[0] = xpos;
	s_pad.cursor[1] = s_pad.winsize[1] - ypos;
}

void mouse_button_callback(GLFWwindow *, int32_t button, int32_t action, int32_t)
{
	auto is_press = action == GLFW_PRESS ? 1 : 0;
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT: s_pad.mouse_L = is_press; break;
	case GLFW_MOUSE_BUTTON_RIGHT: s_pad.mouse_R = is_press; break;
	case GLFW_MOUSE_BUTTON_MIDDLE: s_pad.mouse_M = is_press; break;
	}
}

void scroll_callback(GLFWwindow *, double, double yoffset) { s_pad.wheel += yoffset; }

void character_callback(GLFWwindow *, uint32_t codepoint)  // supports auto-repeat
{
	s_pad.code_count++;
	s_pad.code = codepoint;
	s_pad.incr_sticky(s_pad.code);
}

void key_callback(GLFWwindow *window, int32_t key, int32_t, int32_t action, int32_t)
{
	auto is_press = action == GLFW_PRESS;
	auto special_key = get_special_key(key);

	auto is_shift = [&]() { return special_key == SpuPad::e_lshift || special_key == SpuPad::e_rshift; };
	auto is_ctrl = [&]() {
		return special_key == SpuPad::e_lctrl || special_key == SpuPad::e_rctrl
		    || special_key == SpuPad::e_caps_lock;
	};
	auto is_alt = [&]() { return special_key == SpuPad::e_lalt || special_key == SpuPad::e_ralt; };

	if (is_shift()) s_pad.key_shift = is_press;
	if (is_ctrl()) s_pad.key_ctrl = is_press;
	if (is_alt()) s_pad.key_alt = is_press;

	s_pad.key = is_press ? (special_key ? special_key : key) : 0;

	if (SpuPad::e_f1 <= s_pad.key && s_pad.key <= SpuPad::e_f12) {
		if (!s_pad.key_shift && !s_pad.key_ctrl && !s_pad.key_alt) {
			character_callback(window, special_key);
		}
	}
}
}  // namespace

void GlfwDevice::init(DeviceConfig *dev_config)
{
	aux_error(!glfwInit(), "Failed to initialize GLFW.\n");

	auto monitor_count = 0;
	auto *monitors = glfwGetMonitors(&monitor_count);
	assert(monitor_count > 0);
	s_monitor = monitors[monitor_count - 1];
	assert(s_monitor);

	if (s_monitor) {
		const GLFWvidmode *mode = glfwGetVideoMode(s_monitor);
		// printf("bits= %d:%d:%d\n", mode->redBits, mode->greenBits, mode->blueBits);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		// glfwWindowHint(GLFW_SRGB_CAPABLE, 0);
	}

	s_dev_config = dev_config;
	s_dev_config->name = "glfw";
}

void GlfwDevice::terminate() { glfwTerminate(); }

void GlfwDevice::open(const Attrs &attrs)
{
	const vec4i_t c_location = {0, 0, 640, 480};
	const char *c_title = "tea for two";

	s_title = attrs.get("title", c_title);
	s_location = attrs.get("window", c_location);

	int32_t &ox = s_location.x;
	int32_t &oy = s_location.y;
	int32_t &sx = s_location.z;
	int32_t &sy = s_location.w;

	if (attrs.get("fullscreen", 0)) {
		const GLFWvidmode *mode = glfwGetVideoMode(s_monitor);
		sx = mode->width;
		sy = mode->height;
		s_window = glfwCreateWindow(sx, sy, s_title, s_monitor, nullptr);
	}
	else {
		s_window = glfwCreateWindow(sx, sy, s_title, nullptr, nullptr);
	}

	if (attrs.get("iconic", 0)) {
		glfwIconifyWindow(s_window);
	}

	aux_error(
	        !s_window, "Failed to create GLFW window. sx=%d sy=%d title=[%s] monitor=%p\n", sx, sy, s_title,
	        s_monitor);

	glfwMakeContextCurrent(s_window);

	s_pad.winsize[0] = sx;
	s_pad.winsize[1] = sy;

	glfwSetWindowSize(s_window, sx, sy);
	glfwSetWindowPos(s_window, ox, oy);
	glfwSetWindowTitle(s_window, s_title);

	glfwSetWindowCloseCallback(s_window, close_callback);
	glfwSetFramebufferSizeCallback(s_window, resize_callback);
	glfwSetCursorPosCallback(s_window, cursor_position_callback);
	glfwSetMouseButtonCallback(s_window, mouse_button_callback);
	glfwSetScrollCallback(s_window, scroll_callback);
	glfwSetKeyCallback(s_window, key_callback);
	glfwSetCharCallback(s_window, character_callback);

	// iconic
	GLFWimage image = {
	        gimp_image.width,
	        gimp_image.height,
	        gimp_image.pixel_data,
	};
	glfwSetWindowIcon(s_window, 1, &image);

	resize_callback(s_window, sx, sy);

	for (auto joystick: {GLFW_JOYSTICK_1, GLFW_JOYSTICK_2}) {
		if (glfwJoystickPresent(joystick)) {
			auto name = glfwGetGamepadName(joystick);
			printf("Controller: '%s'\n", name ? name : "no name");
		}
	}
	s_dev_config->sync_pad(s_pad);
}

void GlfwDevice::close()
{
	glfwSetWindowShouldClose(s_window, GLFW_TRUE);
	glfwDestroyWindow(s_window);
}

bool GlfwDevice::swap()
{
#ifndef _WIN32  // slow in win
	if (glfwWindowShouldClose(s_window)) {
		return false;
	}
	static auto s_interval = -1;
	if (int32_t(s_dev_config->interval) != s_interval) {
		s_interval = int32_t(s_dev_config->interval);
		glfwSwapInterval(s_interval);
	}
#endif
	s_pad.swap_count++;
	s_dev_config->sync_pad(s_pad);

	// sleep_microsec(2000 * 1000);
	glfwSwapBuffers(s_window);
	return true;
}

void GlfwDevice::poll()
{
	glfwPollEvents();

	auto index = 0;
	for (auto joystick: {GLFW_JOYSTICK_1, GLFW_JOYSTICK_2}) {
		GLFWgamepadstate state;

		if (glfwGetGamepadState(joystick, &state)) {
			auto &b = state.buttons;
			auto &c = s_pad.controllers[index];
			c.connect = true;
			c.cross = b[GLFW_GAMEPAD_BUTTON_CROSS];
			c.circle = b[GLFW_GAMEPAD_BUTTON_CIRCLE];
			c.square = b[GLFW_GAMEPAD_BUTTON_SQUARE];
			c.triangle = b[GLFW_GAMEPAD_BUTTON_TRIANGLE];
			c.up = b[GLFW_GAMEPAD_BUTTON_DPAD_UP];
			c.down = b[GLFW_GAMEPAD_BUTTON_DPAD_DOWN];
			c.right = b[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT];
			c.left = b[GLFW_GAMEPAD_BUTTON_DPAD_LEFT];
			c.L = b[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER];
			c.R = b[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER];
			c.back = b[GLFW_GAMEPAD_BUTTON_BACK];
			c.start = b[GLFW_GAMEPAD_BUTTON_START];
			c.guide = b[GLFW_GAMEPAD_BUTTON_GUIDE];
		}
		else {
			s_pad.controllers[index].bits = 0;
		}

		int32_t axis_count;
		const float *axes = glfwGetJoystickAxes(joystick, &axis_count);
		if (axes) {
			auto &c = s_pad.controllers[index];
			c.axis_left.x = axes[GLFW_GAMEPAD_AXIS_LEFT_X];
			c.axis_left.y = axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
			c.axis_left.trigger = axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];

			c.axis_right.x = axes[GLFW_GAMEPAD_AXIS_RIGHT_X];  // not working
			c.axis_right.y = axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
			c.axis_right.trigger = axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];
		}
		index++;
	}
}

int32_t GlfwDevice::get(const hash32_t &key, void *value)
{
	if (key == "glfw_window") {
		*reinterpret_cast<GLFWwindow **>(value) = s_window;
		return sizeof(s_window);
	}
	if (key == "glfw_monitor") {
		*reinterpret_cast<GLFWmonitor **>(value) = s_monitor;
		return sizeof(s_monitor);
	}
	return 0;
}

}  // namespace spu::libspu::glfw
