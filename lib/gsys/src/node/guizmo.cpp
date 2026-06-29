//
// Guizmo :
//
#include <gsys/node/guizmo.h>

namespace spu::gs_node {

void Guizmo::init(const Attrs & /*attrs*/)
{
	auto *painter = new gs_painter::Stdout(Attrs());
	replacePainter(painter);
	setProperty(e_lazy, 0);  // safety
}

void Guizmo::begin()
{
	auto *painter = dynamic_cast<gs_painter::Stdout *>(getPainter());
	assert(painter);
	m_elements.clear();
	m_picker.clear();
	painter->begin();
	painter->setColor("aqua");
}

void Guizmo::end()
{
	auto *painter = dynamic_cast<gs_painter::Stdout *>(getPainter());
	assert(painter);
	painter->end();
	getARange() = m_picker.getRange();
}

void Guizmo::setRelatedNodes(const std::vector<GsNode *> &nodes)
{
	GsNode::setRelatedNodes(nodes);
	begin();
	for (auto &node: relatedNodes()) {
		addNode(node);
	}
	end();
}

void Guizmo::addNode(GsNode *node)
{
	node->sync(0);
	auto range = node->getRanges().at(0);
	aux_error(!range.valid(), "'%s' invalid range\n", node->prettyName().c_str());
	for (auto slot_index = 0u; slot_index < node->substanceSlotCount(); slot_index++) {
		auto instances = node->getSubstances<Mat4f>(slot_index);
		for (auto instance_index = 0u; instance_index < instances.size(); instance_index++) {
			auto initial_instance = instances.at(instance_index);
			auto initial_transform = Transformf(initial_instance.c[3]);
			initial_instance.c[3] = {0, 0, 0, 1}; // reset
			auto instanced_range = initial_instance * range;
			internalAddPrim(
			        instanced_range, initial_transform, node, slot_index, instance_index,
			        initial_instance);
		}
	}
}

bool Guizmo::doSync(bool is_nonblock)
{
	if (!is_nonblock) {
		for (auto &e: m_elements) {
			if (e.node) {
				auto element_index = &e - &m_elements[0];
				auto &substance = e.node->getSubstances(e.slot_index).at(e.instance_index);
				getPrimTransform(element_index) = substance * e.initial_transform.inverse();
			}
		}
	}
	return false;
}

void Guizmo::update()
{
	m_gesture.update();
	auto stat = (m_gesture.prev().mouse_L << 1) | m_gesture.curr().mouse_L;
	if (stat) {
		const auto *current = GsCanvas::getCurrent();
		const auto fragscreen = current->fragscreen(0);
		const auto cursor_screen = fragscreen.ortho3(Vec2f(m_gesture.curr().cursor));

		switch (stat) {
		case 0x1: {  // press
			m_picker.getComposition() = *current;
			m_picker.pick(cursor_screen, m_gesture.curr().key_shift);
			break;
		}
		case 0x3: {  // drag
			m_picker.drag(cursor_screen);

			for (auto &e: m_elements) {
				if (e.node) {
					auto element_index = &e - &m_elements[0];
					auto &substance
					        = e.node->getSubstances(e.slot_index).at(e.instance_index);

					substance = getPrimTransform(element_index) * e.initial_transform;
				}
			}

			getARange() = m_picker.getRange();
			break;
		}
		case 0x2: {  // release
			m_picker.drop();
			break;
		}
		default: break;
		}
	}
	GsNode::update();
}

void Guizmo::setVisible(int32_t index, bool visible) { m_elements.at(index).visible = visible; }

void Guizmo::doRender()
{
	auto *painter = dynamic_cast<gs_painter::Stdout *>(getPainter());
	if (painter) {
		auto prim_index = m_picker.primIndex();
		auto &drawcall = painter->getADrawcall();

		for (auto &e: m_elements) {
			auto index = &e - &m_elements[0];
			if (index == prim_index || e.visible) {
				drawcall.ub_material.albedo = index == prim_index ? 1.0 : 0.5;
				painter->GsPainter::draw(GL_LINES, {getPrimTransform(index)}, e.first, e.count);
			}
		}
	}
}
}  // namespace spu::gs_node
