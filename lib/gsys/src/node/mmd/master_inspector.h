//
// MMDMasterInspector :
//
#pragma once
#include <gsys/node/mmd.h>
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_node {

class MMDMasterInspector : public gui::Tweakbar {
public:
	using base_t = gui::Tweakbar;

	MMDMaster *m_master = nullptr;

	enum { e_prev = 0, e_stop, e_next, e_play };

	struct {
		int32_t draw = 1;
		int32_t debug_render = 0;
		int32_t physics = 1;
		int32_t grant = 1;
		int32_t ik = 1;
		int32_t debug_render_body = 1;
		int32_t debug_render_bone = 1;
		int32_t debug_render_grant = 1;
		int32_t debug_render_ik = 1;
		int32_t debug_render_wireframe = 1;
	} m_opts;

	int32_t m_action = e_play;
	Range1f m_frameRange = {0.0, 0.0};

	MMDMasterInspector(MMDMaster *master) : m_master(master)
	{
		const std::vector<gs_node::gui::Menu::Item> items = {
		        {"prev", e_prev},
		        {"stop", e_stop},
		        {"next", e_next},
		        {"play", e_play},
		};

		base_t::setName(padstr("mmd", 24));  // force compact
		base_t::addMenus("play", {items}, {&m_action});

		// m_frameRange = m_master->getFrame().range;
		rebake();
		coreUpdate();
	}

	void rebake()
	{
		auto &frame_range = m_master->getFrame().range;
		if (frame_range.p0 == m_frameRange.p0 && frame_range.p1 == m_frameRange.p1) {
			return;
		}
		printf("rebake..\n");

		m_frameRange = frame_range;
		base_t::clearStdNode();
		base_t::addStdSlider("frame", m_frameRange.p0, m_frameRange.p1, &m_master->m_frame.current);
		base_t::addStdSlider("FPS  ", 0.00, 64.0, &m_master->m_frame.frame_per_sec);

		base_t::addStdButton("main Draw  ", &m_opts.draw);
		base_t::addStdButton("use Physics", &m_opts.physics);
		base_t::addStdButton("use Grant  ", &m_opts.grant);
		base_t::addStdButton("use IK     ", &m_opts.ik);
		base_t::addStdButton("debug Draw ", &m_opts.debug_render);

		base_t::addStdButton("   show body     ", &m_opts.debug_render_body);
		base_t::addStdButton("   show bone     ", &m_opts.debug_render_bone);
		base_t::addStdButton("   show grant    ", &m_opts.debug_render_grant);
		base_t::addStdButton("   show IK       ", &m_opts.debug_render_ik);
		base_t::addStdButton("   show wireframe", &m_opts.debug_render_wireframe);

		base_t::bake();
	}

	void update() override
	{
		rebake();
		frameUpdate();
		if (base_t::isFocus()) {
			auto touch_count = base_t::lastUpdateCount();
			base_t::update();
			if (touch_count != base_t::lastUpdateCount()) {
				coreUpdate();
			}
		}
	}

private:
	void frameUpdate()
	{
		switch (m_action) {
		case e_play: m_master->getFrame().advance = MMDMaster::e_auto_frame_advance; break;
		case e_stop: m_master->getFrame().advance = 0; break;
		case e_next:
			m_master->getFrame().advance = 1;
			m_action = e_stop;
			break;
		case e_prev:
			m_master->getFrame().advance = -1;
			m_action = e_stop;
			break;
		}
	}

	void coreUpdate()
	{
		// state
		{
			auto state = m_opts.debug_render != 0 ? e_active : e_disabled;

			// clang-format off
			std::vector<const void *> ptrs = {
			        &m_opts.debug_render_body,  
				&m_opts.debug_render_bone, 
				&m_opts.debug_render_grant,
			        &m_opts.debug_render_ik, 
				&m_opts.debug_render_wireframe,
			};
			// clang-format on
			base_t::changeState(ptrs, state);
		}

		// property
		for (auto &actor: m_master->getChildren()) {
			// auto &props = actor->getProperties();
			actor->setProperty("render", m_opts.draw);
			actor->setProperty("grant", m_opts.grant);
			actor->setProperty("ik", m_opts.ik);
			actor->setProperty("debug_render", m_opts.debug_render);
			actor->setProperty("debug_render_bone", m_opts.debug_render_bone);
			actor->setProperty("debug_render_grant", m_opts.debug_render_grant);
			actor->setProperty("debug_render_ik", m_opts.debug_render_ik);
			actor->setProperty("debug_render_wireframe", m_opts.debug_render_wireframe);
		}
		{
			m_master->setProperty("physics", m_opts.physics);
			m_master->setProperty("debug_render", m_opts.debug_render);
			m_master->setProperty("debug_render_body", m_opts.debug_render_body);
		}
		Tweakbar::update();
	}
};
}  // namespace spu::gs_node
