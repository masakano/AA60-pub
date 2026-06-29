//
// TerrainInspector :
//
#pragma once
#include <gsys/node/terrain.h>
#include <gsys/painter/displaced_pbr.h>
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_node {
class TerrainInspector : public gui::Tweakbar {
public:
	using base_t = gui::Tweakbar;
	Terrain *m_node = nullptr;

	struct Param {
		int32_t debug_render = 0;
		int32_t render = 1;
		int32_t spread = 1;

	} m_param, m_prevParam;

	TerrainInspector(Terrain *node) : m_node(node)
	{
		base_t::setName(padstr(m_node->prettyName(), 32));
		base_t::addStdButton("debug render", &m_param.debug_render);
		base_t::addStdButton("render", &m_param.render);
		base_t::addStdButton("spread", &m_param.spread);
		base_t::bake();
	}

	void update() override
	{
		if (memcmp(&m_param, &m_prevParam, sizeof(Param))) {
			m_prevParam = m_param;
			m_node->setProperty(Terrain::e_render, m_param.render);
			m_node->setProperty(Terrain::e_debug_render, m_param.debug_render);
			m_node->setProperty(Terrain::e_spread, m_param.spread);
		};
		base_t::update();
	}
};
}  // namespace spu::gs_node
