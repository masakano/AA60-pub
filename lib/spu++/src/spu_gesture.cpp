//
// SpuGesture :
//
#include "symbolmap.h"  // make_symbolmap
#include <spu++/spu_gesture.h>

namespace spu {

void SpuGesture::init(SpuPad *pad)
{
	assert(pad == nullptr);
	if ((m_native = pad) == nullptr) {
		spu_graphics_get("pad", &m_native);
	}
	update();
}

void SpuGesture::setUsage(int16_t code, const char *usage) { m_usages[code] = usage; }

std::vector<std::string> SpuGesture::usages()
{
	std::vector<std::string> strs;
	for (auto &usage: m_usages) {
		if (usage) {
			auto code = &usage - &m_usages[0];
			strs.push_back(
			        string_printf("%-4s:%2x: %s", s_symbolmap[code], m_curr.sticky(code), usage));
		}
	}
	return strs;
}

bool SpuGesture::touched() const
{
	return m_curr.winsize[0] != m_prev.winsize[0] || m_curr.winsize[1] != m_prev.winsize[1]
	    || m_curr.key != m_prev.key || m_curr.code != m_prev.code || m_curr.wheel != m_prev.wheel
	    || m_curr.cursor[0] != m_prev.cursor[0] || m_curr.cursor[1] != m_prev.cursor[1];
}

void SpuGesture::update()
{
	auto is_grab = ms_grab == nullptr || ms_grab == this;
	if (is_grab) {
		m_prev = m_curr;
		m_curr = *m_native;
	}
	if (!m_prev.mouse_R && !m_curr.mouse_R) {
		memcpy(m_anchorR, m_curr.cursor, sizeof(m_curr.cursor));
	}
	if (!m_prev.mouse_L && !m_curr.mouse_L) {
		memcpy(m_anchorL, m_curr.cursor, sizeof(m_curr.cursor));
	}
	m_wheel = m_curr.wheel - m_prev.wheel;
}

const char **SpuGesture::symbolmap() { return s_symbolmap; }
}  // namespace spu
