//
// TweakbarDecorator :
//
#pragma once

#include "default_inspector.h"
#include <gsys/canvas/gs_page.h>
#include <gsys/node/gui/tweakbar.h>
#include <gsys/node/gui/loupe_birdview.h>
#include <spu++/spu_query.h>

namespace spu::gs_canvas::gs_demo_page {

class TweakbarDecorator : public GsPage::IDecorator {
public:
	TweakbarDecorator(GsCanvas &canvas, const Attrs &attrs)
	{
		bool is_plotter = attrs.get("plotter", 0);
		bool is_loupe = attrs.get("loupe", 0);
		bool is_default_inspector = attrs.get("default_inspector", 0);

		if (is_plotter) {
			m_plotter = new gs_node::gui::Tweakbar();
			m_plotter->setName(gs_node::gui::Tweakbar::padstr("time", 24));
			std::vector<const char *> items = {
			        "cpu",
			        "gpu",
			};
			std::vector<float *> value_ptrs = {
			        &m_cpuTime,
			        &m_spuTime,
			};
			m_plotter->addPlotters(" ", items, value_ptrs);
			m_plotter->bake();
		}
		if (is_loupe) {
			m_loupe = new gs_node::gui::Tweakbar();
			m_loupe->setName("loupe");
			m_loupe->addBirdview<gs_node::gui::LoupeBirdview>(" ");
			m_loupe->bake();
		}
		if (is_default_inspector) {
			m_defaultInspector = new gs_node::gui::DefaultInspector(&canvas);
		}
	}

	~TweakbarDecorator()
	{
		delete m_plotter;
		delete m_loupe;
		delete m_defaultInspector;
	}

	void begin() override
	{
		if (m_plotter) {
			m_usec0 = get_microsec();
			m_spuQuery.start();
		}
	}
	void end() override
	{
		if (m_plotter) {
			m_spuQuery.stop(true);
			m_spuTime = m_spuQuery.usec();
			m_cpuTime = get_microsec() - m_usec0;
		}
	}

	void set(const Attrs &) override {}

private:
	gs_node::gui::Tweakbar *m_plotter = nullptr;
	gs_node::gui::Tweakbar *m_loupe = nullptr;
	gs_node::gui::Tweakbar *m_defaultInspector = nullptr;

	SpuQuery m_spuQuery;
	float m_spuTime = 0;
	float m_cpuTime = 0;
	uint64_t m_usec0 = 0;
};
}  // namespace spu::gs_canvas::gs_demo_page
