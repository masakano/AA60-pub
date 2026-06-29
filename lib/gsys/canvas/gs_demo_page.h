//
// GsDemoPage :
//
#pragma once

#include <gsys/canvas/gs_page.h>
#include <gsys/canvas/shadowmap.h>
#include <gsys/canvas/lightmap.h>
#include <gsys/node/guizmo.h>
#include <gsys/painter.h>

namespace spu::gs_canvas {

class GsDemoPage : public GsPage {
public:
	explicit GsDemoPage(const char *name = nullptr) : GsPage(name) {}
	explicit GsDemoPage(const Attrs &attrs) : GsDemoPage(nullptr) { init(attrs); }

	~GsDemoPage();

	void init(const Attrs &attrs) override;
	void begin() override;

	GsPainter *createPainter(const char *def_name, const Attrs &attrs);
	GsNode *createNode(const Attrs &attrs, GsPainter *painter);
	GsNode *createFloor(const Attrs &attrs, const std::vector<GsNode *> &reflect_nodes = {});

	gs_node::Guizmo *getGuizmo() const { return m_guizmo; }
	Lightmap *getLightmap() const { return m_lightmap; }
	Shadowmap *getShadowmap() const { return m_shadowmap; }

	void replaceGuizmo(gs_node::Guizmo *guizmo) { reset(m_guizmo, guizmo); }
	void replaceLightmap(Lightmap *lightmap) { reset(m_lightmap, lightmap); }
	void replaceShadowmap(Shadowmap *shadowmap) { reset(m_shadowmap, shadowmap); }
	[[deprecated("use replaceGuizmo()")]] void setGuizmo(gs_node::Guizmo *guizmo) { replaceGuizmo(guizmo); }
	[[deprecated("use replaceLightmap()")]] void setLightmap(Lightmap *lightmap)
	{
		replaceLightmap(lightmap);
	}
	[[deprecated("use replaceShadowmap()")]] void setShadowmap(Shadowmap *shadowmap)
	{
		replaceShadowmap(shadowmap);
	}

private:
	const float c_min_notch = 0.01f;
	const float c_ndiv = 16;

	gs_node::Guizmo *m_guizmo = nullptr;
	Shadowmap *m_shadowmap = nullptr;
	Lightmap *m_lightmap = nullptr;

	GsPainter *createGrassFurPainter(Attrs &attrs);
	GsPainter *createTerrainPainter(Attrs &attrs);
	GsPainter *createPlanePainter(Attrs &attrs);
	GsPainter *createWaterPainter(Attrs &attrs);
	GsNode *createManifold2D(const Attrs &attrs, GsPainter *painter);
	GsNode *createWavefrontObj(const Attrs &attrs, GsPainter *painter);
	GsNode *createDividedWavefrontObj(const Attrs &attrs, GsPainter *painter);
	GsNode *createLodWavefrontObj(const Attrs &attrs, GsPainter *painter);
	void setSyncCallback(const Attrs &attrs, GsNode *node);

	static void shutdown() { assert(0); }  // safety
};

class Demowork : public GsDemoPage {
public:
	using GsDemoPage::GsDemoPage;
};

}  // namespace spu::gs_canvas
