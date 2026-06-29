//
// Container :
//
#include <algorithm>
#include <gsys/node/dui.h>

namespace spu::gs_node::dui {

void Container::setCondition(const Value &refvar, int32_t condvar)
{
	m_refvar = refvar;
	m_condvar = condvar;
	GsObject::setProperty(e_render, int32_t(m_refvar) == condvar);
}

bool Container::relink(DuiNode *element)
{
	auto child_nodes = moveChildren();
	auto ep = vector_find(child_nodes, element);

	if (ep != end(child_nodes)) {
		child_nodes.erase(ep);
		child_nodes.push_back(element);
		addChildren(child_nodes);
		return true;
	}
	addChildren(child_nodes);
	return false;
}

bool Container::remove(DuiNode *element)
{
	auto child_nodes = moveChildren();
	auto ep = vector_find(child_nodes, element);
	if (ep == end(child_nodes)) {
		addChildren(child_nodes);
		return false;
	}
	child_nodes.erase(ep);
	addChildren(child_nodes);
	return true;
}

void Container::update()
{
	DuiNode::update();
	if (!m_refvar.empty()) {
		GsObject::setProperty(e_render, int32_t(m_refvar) == m_condvar);
	}
}

void Container::doRender()
{
	DuiNode::doRender();
	for (auto &child: getChildren()) {
		auto *container = dynamic_cast<Container *>(child);
		if (container && !container->m_refvar.empty()) {
			container->GsObject::setProperty(
			        e_render, int32_t(container->m_refvar) == container->m_condvar);
		}
		child->render();
	}
}
}  // namespace spu::gs_node::dui
