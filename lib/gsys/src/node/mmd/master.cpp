//
// MMDMaster :
//
#include "pmd.h"
#include "pmx.h"
#include "master_inspector.h"

#include <gsys/painter/stdout.h>
#include <gsys/util/bullet.h>

namespace spu::gs_node {

bool MMDFrame::update(float delta)
{
	if (advance == huge<int32_t>()) {
		current += delta * frame_per_sec;
	}
	else {
		current += advance;
	}
	if (current < range.p0 || current > range.p1) {
		current = range.p0;
		return true;
	}
	return false;
}

MMDActor *MMDMaster::addActor(
        int32_t index, const std::string &path, GsPainter *painter, const Attrs &node_attrs)
{
	Attrs aux_attrs = {
	        {"path",         path            },
	        {"bullet",       &m_bullet       },
	        {"master_frame", &m_frame.current},
	};

	auto ext = std::filesystem::path(path).extension();
	MMDActor *actor = nullptr;
	if (ext == ".pmd") {
		actor = new mmd::PMDActor(node_attrs + aux_attrs);
	}
	else if (ext == ".pmx") {
		actor = new mmd::PMXActor(node_attrs + aux_attrs);
	}
	else {
		aux_error(true, "%s: invalid extension. must be \".pmx\" or \".pmd\"\n", path.c_str());
	}
	if (painter) {
		actor->replacePainter(painter);
	}
	if (index == -1) {
		addChildren({actor});
	}
	else {
		auto child_nodes = moveChildren();
		delete child_nodes.at(index);
		child_nodes.at(index) = actor;
		addChildren(child_nodes);
	}
	m_bullet.reset();
	return actor;
}

void MMDMaster::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new MMDMasterInspector(this);
	}
}

void MMDMaster::set(const Attrs &attrs)
{
	if (attrs.get("reset", 0)) {
		m_bullet.reset();
	}
	GsNode::set(attrs);
}

void MMDMaster::update()
{
	GsNode::update();  // must be before bullet dispatch

	if (m_frame.update(getSeconds().delta())) {
		m_bullet.reset();
	}

	if (getProperty(e_physics)) {
		m_bullet.update();
	}

	auto &range = getARange();
	range.invalidate();
	for (auto &node: getChildren()) {
		auto node_range = m_bullet.getRange(node);
		if (node_range.valid()) {
			node->getARange() = node_range;
			range.expand(node_range);
		}
	}
}

MMDMaster::~MMDMaster()
{
	GsNode::dispose();  // dispose children before m_bullet destruction
}

void MMDMaster::init(const Attrs &)
{
	const float c_world_size = 256;  // need parameterize
	const float c_gravity = 9.8;
	m_bullet.newWorld(c_world_size, c_gravity);
	m_bullet.addPlane(ey(), 0.0, 1.0);  // ground
}

void MMDMaster::doDebugRender()
{
	if (getProperty(e_debug_render_body)) {
		auto nodeworlds = instancedNodeworlds();
		getBullet().view(nodeworlds.at(0));  // 1st slot only
	}
	GsNode::doDebugRender();
}
}  // namespace spu::gs_node
