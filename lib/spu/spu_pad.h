//
// SpuPad :
//
#pragma once
#include <ssys/ssys.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

namespace spu {

struct SpuPad {
	enum {
		e_escape = 0x1b,
		e_delete = 0x7f,
		e_enter = '\n',
		e_tab = '\t',
		e_backspace = '\b',

		e_f1 = 128,
		e_f2,
		e_f3,
		e_f4,
		e_f5,
		e_f6,
		e_f7,
		e_f8,
		e_f9,
		e_f10,
		e_f11,
		e_f12,
		e_up,
		e_down,
		e_left,
		e_right,
		e_lshift,
		e_rshift,
		e_lctrl,
		e_rctrl,
		e_lalt,
		e_ralt,
		e_home,
		e_pageup,
		e_pagedown,
		e_end,
		e_caps_lock,
	};

	union {
		struct {
			uint32_t mouse_L  : 1;
			uint32_t mouse_M  : 1;
			uint32_t mouse_R  : 1;
			uint32_t key_shift: 1;
			uint32_t key_ctrl : 1;
			uint32_t key_alt  : 1;
			uint32_t iconic   : 1;
			uint32_t reset    : 1;
		};
		uint16_t bits = 0;
	};
	struct {
		union {
			struct {
				uint32_t connect : 1;
				uint32_t cross   : 1;
				uint32_t circle  : 1;
				uint32_t square  : 1;
				uint32_t triangle: 1;
				uint32_t up      : 1;
				uint32_t down    : 1;
				uint32_t right   : 1;
				uint32_t left    : 1;
				uint32_t L       : 1;
				uint32_t R       : 1;
				uint32_t back    : 1;
				uint32_t start   : 1;
				uint32_t guide   : 1;
			};
			uint32_t bits = 0;
		};
		struct {
			float x = 0;        // [-1.0,+1.0] (right not working?)
			float y = 0;        // [-1.0,+1.0]
			float trigger = 0;  // (not working?)
		} axis_left, axis_right;
	} controllers[2];

	int16_t winsize[2] = {0};
	int16_t cursor[2] = {0};

	uint32_t swap_count = 0;
	uint32_t code_count = 0;
	int16_t key = 0;
	int16_t code = 0;
	int16_t wheel = 0;
	uint32_t stickies[256 / 16] = {};  // 2bit each

	void incr_sticky(int16_t code)
	{
		auto pos = code / 16;
		auto bit = (code % 16) * 2;
		auto mask = 0x3 << bit;
		auto sticky = (stickies[pos] + (1 << bit)) & mask;
		stickies[pos] &= ~mask;
		stickies[pos] |= sticky;
	}
	uint8_t sticky(int16_t code) const
	{
		auto pos = code / 16;
		auto bit = (code % 16) * 2;
		return (stickies[pos] >> bit) & 0x3;
	}
};
}  // namespace spu

#ifdef __clang__
#pragma clang diagnostic pop
#endif
