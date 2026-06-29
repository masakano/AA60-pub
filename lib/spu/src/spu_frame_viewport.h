//
// Rect :
//
#pragma once
#include "spu_object.h"

namespace spu::libspu::spu_frame {

struct Rect : public vec4f_t {
	Rect() noexcept { ox = oy = sx = sy = 0; }

	Rect(const vec4f_t &v4) noexcept { fv = v4.fv; }

	bool isValid() { return !(ox == 0 && oy == 0 && sx == 0 && sy == 0); }
	void report(const char *fmt, const char *name)
	{
		if (isValid()) {
			aux_printf(fmt, name, ox, oy, sx, sy);
		}
	}
};

class Viewports {
public:
	static constexpr const float c_scissor_size_max = float(0xffff);
	static constexpr const int32_t c_viewport_max = 8;  // num of viewport scissor

	struct Slot {
		const hash32_t viewport_name;  // viewport name
		const hash32_t scissor_name;   // scissor name
		Rect viewport;                 // viewport
		Rect scissor;                  // scissor;
	};

	void set(const Attrs &attrs)
	{
		for (auto &slot: m_slots) {
			attrs.apply<vec4f_t>(slot.viewport_name, slot.viewport);
			attrs.apply<vec4f_t>(slot.scissor_name, slot.scissor);
		}
	}

	int32_t get(const hash32_t &name, void *value)
	{
		auto ret = 0;
		for (auto &slot: m_slots) {
			if ((ret = getvalue(name, value, slot.viewport_name, slot.viewport))) {
				return ret;
			}
			if ((ret = getvalue(name, value, slot.scissor_name, slot.scissor))) {
				return ret;
			}
		}
		return ret;
	}

	void report()
	{
		for (auto &slot: m_slots) {
			Rect v = slot.viewport;
			Rect s = slot.scissor.isValid() ? slot.scissor : slot.viewport;

			v.report("\t%-10s: %7.1f %7.1f %7.1f %7.1f\n", slot.viewport_name.c_str());
			s.report("\t%-10s: %7.1f %7.1f %7.1f %7.1f\n", slot.scissor_name.c_str());
		}
	}

	void send()
	{
		for (auto &slot: m_slots) {
			if (slot.viewport.isValid()) {
				uint32_t i = &slot - &m_slots[0];
				Rect v = slot.viewport;
				Rect s = slot.scissor.isValid() ? slot.scissor : slot.viewport;

				s.ox = std::max(s.ox, 0.0f);
				s.oy = std::max(s.oy, 0.0f);
				s.ox = std::max(s.ox, v.ox);
				s.oy = std::max(s.oy, v.oy);

				s.sx = std::min(s.sx, c_scissor_size_max);
				s.sy = std::min(s.sy, c_scissor_size_max);
				s.sx = std::min(s.sx, std::max(0.0f, v.ox + v.sx - s.ox));
				s.sy = std::min(s.sy, std::max(0.0f, v.oy + v.sy - s.oy));

				F(glViewportIndexedf, i, v.ox, v.oy, v.sx, v.sy);
				F(glScissorIndexed, i, s.ox, s.oy, s.sx, s.sy);
			}
		}
	}

	const Slot &slot(uint32_t index) const { return m_slots[index]; }

private:
	Slot m_slots[c_viewport_max] = {
	        {"viewport0", "scissor0", Rect(), Rect()},
                {"viewport1", "scissor1", Rect(), Rect()},
	        {"viewport2", "scissor2", Rect(), Rect()},
                {"viewport3", "scissor3", Rect(), Rect()},
	        {"viewport4", "scissor4", Rect(), Rect()},
                {"viewport5", "scissor5", Rect(), Rect()},
	        {"viewport6", "scissor6", Rect(), Rect()},
                {"viewport7", "scissor7", Rect(), Rect()},
	};
};
}  // namespace spu::libspu::spu_frame
