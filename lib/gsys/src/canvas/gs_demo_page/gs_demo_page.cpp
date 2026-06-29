//
// GsDemoPage :
//
#include <gsys/canvas/gs_demo_page.h>
#include "shadowmap_decorator.h"
#include "lightmap_decorator.h"
#include "tweakbar_decorator.h"
#include "postproc.h"

namespace spu::gs_canvas {

void GsDemoPage::init(const Attrs &attrs)
{
	GsPage::init(attrs);

	addDecorator(new gs_demo_page::ShadowmapDecorator(*this, attrs));
	addDecorator(new gs_demo_page::LightmapDecorator(*this, attrs));
	addDecorator(new gs_demo_page::TweakbarDecorator(*this, attrs));
	replacePostproc(new gs_demo_page::Postproc(attrs, getShadowmap()));
	m_guizmo = new gs_node::Guizmo(attrs);
}

GsDemoPage::~GsDemoPage()
{
	delete m_guizmo;
	delete m_shadowmap;
	delete m_lightmap;
}

void GsDemoPage::begin()
{
	GsPage::begin();
	m_guizmo->update();
	m_guizmo->render();
	
	//worldview(0).report("demo.worldview");
}
}  // namespace spu::gs_canvas
