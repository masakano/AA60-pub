//
// LightmapDecorator :
//
#pragma once

#include <gsys/canvas/gs_demo_page.h>
#include <gsys/canvas/lightmap.h>

namespace spu::gs_canvas::gs_demo_page {

class LightmapDecorator : public GsPage::IDecorator {
public:
	LightmapDecorator(GsDemoPage &demo_page, const Attrs &attrs) : m_demoPage(demo_page)
	{
		Lightmap *lightmap = nullptr;

		auto bgname = std::string(attrs.get("canvas.bgname", "?lightmap:cloudydome:none"));
		if (bgname == "lightmap") {
			lightmap = new ImageLightmap(Attrs());
		}
		else if (bgname == "cloudydome") {
			lightmap = new CloudyLightmap(Attrs());
		}
		else {
			// do nothing
		}
		if (lightmap) {
			lightmap->startInspector();
			m_demoPage.replaceLightmap(lightmap);
		}
	}

	void begin() override
	{
		auto *lightmap = m_demoPage.getLightmap();
		if (lightmap) {
			lightmap->update();
			lightmap->render();
		}
	}
	void end() override {}
	void set(const Attrs &) override {}

private:
	GsDemoPage &m_demoPage;
};
}  // namespace spu::gs_canvas::gs_demo_page
