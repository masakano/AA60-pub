//
// TimerDecorator :
//
#pragma once
#include <gsys/canvas/gs_page.h>
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_canvas::gs_page {

class TimerDecorator : public GsPage::IDecorator {
public:
	TimerDecorator(GsPage &page) : m_page(page) { m_startFrame = m_page.getSeconds().count(); }

	void begin() override { m_page.getCpuProf().start(); }

	void end() override
	{
		m_page.getCpuProf().stop();
		m_page.getSeconds().update();
		if (m_maxFrame > 0 && m_page.getSeconds().count() > m_startFrame + m_maxFrame) {
			aux_message(0, "terminated: max_frame=%d\n", m_maxFrame);
			m_page.setProperty("alive", 0);
		}
	}

	void set(const Attrs &attrs) override
	{
		auto fixed_delta = attrs.getf<float>("fixed_delta");
		if (fixed_delta.hit) {
			m_page.getSeconds().setFixedDelta(fixed_delta.value);
		}
		m_maxFrame = attrs.get("max_frame", m_maxFrame);
	}

private:
	uint32_t m_startFrame = 0;
	uint32_t m_maxFrame = 0;
	GsPage &m_page;
};
}  // namespace spu::gs_canvas::gs_page
