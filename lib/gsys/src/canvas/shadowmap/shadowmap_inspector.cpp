//
// ShadowmapInspector :
//
#include "shadowmap_inspector.h"
#include "gsys/canvas.h"

namespace spu::gs_canvas {

ShadowmapInspector::ShadowmapInspector(Shadowmap *shadowmap) : m_shadowmap(shadowmap)
{
	auto pretty_name = shadowmap->prettyName();

	// tweakbar
	{
		base_t::setName(pretty_name);
		base_t::addStdButton("birdview", &m_isBirdview);
		base_t::addStdSlider("factor   ", 0.0, 4.0, &m_shadowmap->m_polyOffset.factor, 4.0);
		base_t::addStdSlider("units    ", 0.0, 1.0, &m_shadowmap->m_polyOffset.units, 4.0);

		if (m_shadowmap->gaussCanvas()) {
			base_t::addStdSlider("variance ", 0.0, 32.0, &m_shadowmap->m_variance, 2);
		}

		auto layer_count = m_shadowmap->layerCount();
		std::vector<uint32_t> ids;
		std::vector<uint32_t> layers;
		std::vector<uint32_t> levels;
		for (auto layer = 0; layer < layer_count; layer++) {
			ids.push_back(shadowmap->getBuffer("depth").id());
			layers.push_back(layer);
			levels.push_back(0);
		}
		base_t::addTexviews("depth", ids, layers, levels, std::min(layer_count, 2));

		const std::vector<gs_node::gui::Menu::Item> c_pattern_items = {
		        {"32",  Shadowmap::e_poisson_32 },
		        {"64",  Shadowmap::e_poisson_64 },
		        {"128", Shadowmap::e_poisson_128},
		};
		base_t::addMenus("blocker pattern", {c_pattern_items}, {&m_shadowmap->m_blockerPattern});
		base_t::addMenus("receiver pattern", {c_pattern_items}, {&m_shadowmap->m_receiverPattern});
		base_t::bake();
	}

	// birdview
	{
		Attrs birdview_attrs = {
		        {"shadowmap", m_shadowmap},
		};
		m_birdviewTweakbar.setName(pretty_name + " birdview");
		m_birdviewTweakbar.addBirdview<ShadowmapBirdview>(" ", birdview_attrs);
		m_birdviewTweakbar.setProperty(e_render, 0);
		m_birdviewTweakbar.bake();
	}
}

void ShadowmapInspector::update()
{
	if (m_windowModifier.rate() == 1.0) {  // window is open
		birdviewUpdate();
	}
	base_t::update();
}

void ShadowmapInspector::birdviewUpdate()
{
	if (m_shadowmap->getDrawfunc().points.empty()) {
		m_birdviewTweakbar.setProperty(e_render, 0);
		base_t::changeState({&m_isBirdview}, e_disabled);
	}
	else {
		m_birdviewTweakbar.setProperty(e_render, m_isBirdview);
		base_t::changeState({&m_isBirdview}, e_active);
	}
}

ShadowmapBirdview::ShadowmapBirdview(const Attrs &attrs) : Birdview(attrs)
{
	m_shadowmap = attrs.get<Shadowmap *>("shadowmap", nullptr);
	assert(m_shadowmap);
	Attrs camera_attrs = {
	        {"watch_range", &m_range},
	};
	getCamera().set(camera_attrs);
}

void ShadowmapBirdview::coreDraw()
{
	const auto &camera_composition = m_shadowmap->cameraComposition();
	const auto &shadow_composition = m_shadowmap->shadowComposition();

	auto *painter = gs_painter::Stdout::get();
	painter->begin();
	// painter->setIsFill(false);  // must be after begin()
	painter->setColor("yellow");
	painter->addPrim(camera_composition.worldscreen(0));
	painter->setColor("darkred");
	auto blocker_range = Range3f(m_shadowmap->getDrawfunc().points);
	painter->addPrim(blocker_range);

	for (auto layer = 0; layer < m_shadowmap->layerCount(); layer++) {
		Mat4f shadow_worldshadow = shadow_composition.worldscreen(layer);
		painter->setColor("aqua");
		painter->addPrim(shadow_worldshadow);
	}

	painter->end();

	painter->setFillAlpha(0.2);
	// painter->renderMode(GL_LINES);
	painter->draw(GL_LINES);

	m_range = Range3f(m_shadowmap->getDrawfunc().points);
	painter->begin();
	painter->setColor("red");
	painter->addPrim(m_range);
	painter->end();
	// painter->renderMode(GL_TRIANGLES);
	painter->draw(GL_TRIANGLES);
	// painter->renderMode(GL_LINES);
	painter->draw(GL_LINES);
}
}  // namespace spu::gs_canvas
