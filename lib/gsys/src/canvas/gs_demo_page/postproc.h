//
// Postproc :
//
#pragma once
#include <gsys/canvas/gs_page.h>
#include <gsys/canvas/shadowmap.h>

namespace spu::gs_canvas::gs_demo_page {

class Postproc : public GsPage::IPostproc {
public:
	Postproc(const Attrs &attrs, gs_canvas::Shadowmap *shadowmap)
	{
		if (attrs.get("use_ssao", true)) {
			initSsaoCanvas();
		}
		initPostprocCanvas(attrs, shadowmap);
	}

	~Postproc()
	{
		delete m_postprocCanvas;
		delete m_ssaoCanvas;
	}

	void postproc(GsPage *page) override;

private:
	GsCanvas *m_postprocCanvas = nullptr;
	GsCanvas *m_ssaoCanvas = nullptr;

	void initSsaoCanvas();
	void initPostprocCanvas(const Attrs &attrs, gs_canvas::Shadowmap *shadowmap);
};
}  // namespace spu::gs_canvas::gs_demo_page
