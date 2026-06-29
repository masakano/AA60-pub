//
// GsNode :
//
#include <gsys/canvas.h>
#include <gsys/node.h>
#include <gsys/painter/stdout.h>

namespace spu {

namespace {
template<class T> auto vector_flatten(const std::vector<std::vector<T>> &vecs)
{
	std::vector<T> flat_vec;
	for (auto vec: vecs) {
		vector_cat(flat_vec, vec);
	}
	return flat_vec;
}
}  // namespace

GsNode::GsNode(const char *name) : GsObject(name)
{
	auto &ranges = getRanges();
	ranges.resize(1);
	ranges[0].invalidate();

	setProperty(e_lazy, 1);
	setProperty(e_lod, 0);
	setProperty(e_flip, 0);
}

GsNode::~GsNode() { dispose(); }

void GsNode::dispose()
{
	replaceChildren({});
	replacePainter(nullptr);
	GsObject::dispose();
}

std::vector<Vec3f> GsNode::points(const std::vector<Mat4f> &nodeworlds)
{
	// printf("points..\n");
	auto self_points = childPoints(nodeworlds);  // implies build()
	for (auto instanced_points: instancedPoints()) {
		vector_cat(self_points, instanced_points);
	}
	return self_points;
}

std::vector<Vec3f> GsNode::childPoints(const std::vector<Mat4f> &nodeworlds)
{
	buildInstances(nodeworlds);

	std::vector<Mat4f> child_nodeworlds;
	for (auto nodeworlds: instancedNodeworlds()) {
		vector_cat(child_nodeworlds, nodeworlds);
	}

	std::vector<Vec3f> child_points;
	for (auto &child: m_children) {
		vector_cat(child_points, child->points(child_nodeworlds));
	}
	return child_points;
}

void GsNode::replacePainter(GsPainter *painter)
{
	if (m_painter == painter) {
		return;
	}

	auto *old_painter = movePainter();
	auto &alives = GsObject::aliveObjects();
	if (vector_is_find(alives, old_painter)) {
		delete old_painter;
	}

	if ((m_painter = painter)) {
		auto painter_range = m_painter->getRange();
		if (painter_range.valid()) {
			for (auto &range: getRanges()) {
				range = painter_range;
			}
		}
		auto painter_stride = painter->instanceStride();
		initSubstances(painter_stride, 1);
		m_painter->setNode(this);
	}
}

GsPainter *GsNode::movePainter()
{
	auto *painter = m_painter;
	auto &alives = GsObject::aliveObjects();
	if (vector_is_find(alives, painter)) {
		// painter->setParentNode(nullptr);
		painter->setNode(nullptr);
	}
	m_painter = nullptr;
	return painter;
}

bool GsNode::hasShader(const hash32_t &type) const
{
	if (m_painter && m_painter->hasShader(type)) {
		return true;
	}
	for (const auto &child: m_children) {
		if (child->hasShader(type)) {
			return true;
		}
	}
	return false;
}

void GsNode::addChildren(const std::vector<GsNode *> &children)
{
	for (auto &child: children) {
		aux_error(child == nullptr, "%s: adding nullptr\n", prettyName().c_str());
		if (!vector_is_find(m_children, child)) {
			auto *parent = child->getParent();
			aux_error(parent != nullptr, 0, "%s: child already has parent\n", prettyName().c_str());
			child->m_parent = this;
			m_children.push_back(child);
		}
	}
}

void GsNode::replaceChildren(const std::vector<GsNode *> &children)
{
	auto old_children = moveChildren();
	auto &alives = GsObject::aliveObjects();
	for (auto &child: old_children) {
		if (vector_is_find(alives, child)) {
			delete child;
		}
	}
	addChildren(children);
}

std::vector<GsNode *> GsNode::moveChildren()
{
	auto children = m_children;
	auto &alives = GsObject::aliveObjects();
	for (auto &child: children) {
		if (vector_is_find(alives, child)) {
			child->m_parent = nullptr;
		}
	}
	m_children.clear();
	return children;
}

void GsNode::render(const std::vector<Mat4f> &nodeworlds)
{
	if (sync(true) == 0 && (m_painter == nullptr || m_painter->sync(true) == 0)) {
		if (doBuild(nodeworlds)) {
			if (getProperty(e_render)) {
				doRender();
			}
			if (getProperty(e_debug_render)) {
				doDebugRender();
			}
		}
	}
}

void GsNode::startInspector()
{
	auto *painter = m_painter;
	if (painter) {
		painter->setName(name() + " painter");
		painter->startInspector();
	}

	for (auto &child: m_children) {
		child->startInspector();
	}
}

void GsNode::setProperty(const hash32_t &type, int32_t value)
{
	GsObject::setProperty(type, value);
	for (auto &node: m_children) {
		node->setProperty(type, value);
	}
}

void GsNode::set(const Attrs &attrs)
{
	attrs.subpeek({"array."}, "use \"painter.\" prefix\n");
	attrs.subpeek({"u_", "ub_"}, "use \"painter.shader.\" prefix");
	attrs.subpeek({"a."}, "use \"painter.\" prefix");

	auto *painter = m_painter;
	GsObject::set(attrs);

	if (painter) {
		painter->set(attrs.select("painter."));  // add prefix
	}
	for (auto &child: m_children) {
		child->set(attrs);
	}
}

void GsNode::update()
{
	GsObject::update();

	auto *painter = m_painter;
	if (painter) {
		painter->update();
	}
	for (auto &node: m_children) {
		node->update();
	}
}

bool GsNode::doSync(bool is_nonblock)
{
	for (auto &node: m_children) {
		if (node->sync(is_nonblock)) {
			return true;
		}
	}
	return false;
}

bool GsNode::doBuild(const std::vector<Mat4f> &nodeworlds)
{
	auto is_lazy = getProperty(e_lazy);
	auto is_lod = getProperty(e_lod);
	auto child_points = childPoints(nodeworlds);  // implies build()

	if (is_lazy || is_lod) {
		auto org_ranges = getRanges();
		auto det = [](const Range3f &range) { return !range.valid(); };

		if (vector_is_find_if(org_ranges, det)) {
			aux_message(
			        0, "%s: invalid range. (disable 'e_lazy' and 'e_lod'). \n",
			        prettyName().c_str());
			is_lazy = is_lod = 0;
		}
		else {
			auto *current = GsCanvas::getCurrent();
			auto worldscreen = current->worldscreen(0);
			auto child_range = Range3f(child_points);
			child_range.expand(getRanges()[0]);  // use 1st range only
			pruneInstances(worldscreen, child_range, is_lod);
		}
	}

	auto base_instance = 0;
	std::vector<uint32_t> instance_counts(instances().size());
	std::vector<uint32_t> base_instances(instances().size());
	for (auto slot = 0u; slot < instances().size(); slot++) {
		auto instance_count = instances().at(slot).size() / substanceStride();
		instance_counts.at(slot) = instance_count;
		base_instances.at(slot) = base_instance;
		base_instance += instance_count;
	}

	if (base_instance == 0) {
		return false;  // clip
	}
	auto *painter = m_painter;
	if (painter) {
		for (auto &drawcall: painter->getDrawcalls()) {
			for (auto slot = 0u; slot < drawcall.coms.size(); slot++) {
				auto &com = drawcall.coms[slot];
				if (slot < base_instances.size()) {
					com.base_instance = base_instances.at(slot);
					com.instance_count = instance_counts.at(slot);
				}
				else if (is_lod) {
					com.base_instance = 0;
					com.instance_count = 0;
				}
				else {
					// advanced /alex
					com.base_instance = base_instances.back();
					com.instance_count = instance_counts.back();
				}
			}
		}
	}
	return true;
}

void GsNode::doRender()
{
	// body
	{
		auto *painter = m_painter;
		if (painter) {
			auto flat_instances = vector_flatten(instances());
			painter->setProperty(e_flip, getProperty(e_flip));
			painter->render(flat_instances);
		}
	}

	// children
	{
		auto flat_nodeworlds = vector_flatten(instancedNodeworlds());
		for (auto &node: m_children) {
			node->render(flat_nodeworlds);
		}
	}
}

void GsNode::doDebugRender()
{
	const Vec4f c_colors[] = {
	        {0, 0, 1, 1},
                {0, 1, 0, 1},
                {1, 0, 0, 1},
                {1, 1, 0, 1},
                {1, 0, 1, 1},
                {0, 1, 1, 1},
	};

	// range
	{
		auto *painter = gs_painter::Stdout::get();

		painter->begin();
		for (auto slot = 0u; slot < instancedFrustums().size(); slot++) {
			painter->setColor(c_colors[slot % 6]);
			for (auto &frustum: instancedFrustums().at(slot)) {
				painter->addPrim(frustum);
			}
		}
		painter->end();
		// painter->renderMode(GL_LINES);
		painter->draw(GL_LINES);
	}

	// wirefreme
	{
		auto *painter = m_painter;
		if (painter) {
			for (auto slot = 0u; slot < instances().size(); slot++) {
				if (instances().at(slot).empty()) continue;  // don't forget

				auto drawcalls_save = painter->getDrawcalls();
				for (auto &drawcall: painter->getDrawcalls()) {
					drawcall.poly_offset = {-1.0, -1.0};
					drawcall.flags.line_offset = true;
					drawcall.ub_material.albedo = Vec4f(c_colors[slot % 6], 0.5);
					drawcall.ub_material.ao = 1.0;
					drawcall.flags.fill = false;
					drawcall.flags.blend = true;
					drawcall.coms = {drawcall.coms[slot]};  // single instances
				}
				auto flat_instances = vector_flatten(instances());
				painter->render(flat_instances);
				painter->getDrawcalls() = drawcalls_save;
			}
		}
	}

	// children
	{
		auto flat_nodeworlds = vector_flatten(instancedNodeworlds());
		for (auto &node: m_children) {
			auto prev_render = node->getProperty(e_render);
			auto prev_debug_render = node->getProperty(e_debug_render);

			node->setProperty(e_render, 0);
			node->setProperty(e_debug_render, 1);
			node->render(flat_nodeworlds);
			node->setProperty(e_render, prev_render);
			node->setProperty(e_debug_render, prev_debug_render);
		}
	}
}

void GsNode::report(const char *str) const
{
	GsObject::report(str);
	aux_printf("\n");

	const char *count_fmt = "%7d ";
	auto com_count = 0u;

	if (m_painter) {
		aux_printf("    painter: '%s'\n", m_painter->prettyName().c_str());
		auto &drawcalls = m_painter->getDrawcalls();
		for (auto &d: drawcalls) {
			com_count = std::max(com_count, uint32_t(d.coms.size()));
		}
	}
	else {
		com_count = instances().size();
	}

	if (com_count > 0) {
		auto headline = std::string("\tinst range notch ");
		headline += string_printf(" %9s %9s %9s ", "total", "from", "to");

		if (m_painter) {
			for (auto i = 0u; i < m_painter->getDrawcalls().size(); i++) {
				headline += string_printf(count_fmt, i);
			}
		}
		aux_printf("%s\n", pretty_string(headline).c_str());

		// body
		auto &ranges = getRanges();
		auto &notches = getNotches();
		for (auto i = 0u; i < com_count; i++) {
			std::string line = "\t";
			if (instances().size() > i) {
				line += string_printf("%4ld ", instances().at(i).size());
			}
			else {
				line += string_printf("%4s ", "-");
			}
			line += string_printf("%5s ", ranges.size() > i ? "o" : "-");
			line += string_printf("%5s ", notches.size() > i ? "o" : "-");

			auto total_count = 0u;
			auto max_index = 0u;
			auto index_count = 0x7fffffffu;

			if (m_painter) {
				auto &drawcalls = m_painter->getDrawcalls();
				for (auto &d: drawcalls) {
					auto first = d.coms.at(i).first;
					auto count = d.coms.at(i).count;
					index_count = std::min(index_count, first);
					max_index = std::max(max_index, first + count);
					total_count += count;
				}
				line += string_printf(" %9d %9d %9d ", total_count, index_count, max_index);

				for (auto &d: drawcalls) {
					line += string_printf(count_fmt, d.coms.at(i).count);
				}
			}
			aux_printf("%s\n", pretty_string(line).c_str());
		}
	}
	// aux_printf("\n");
	for (auto &node: m_children) {
		node->report(nullptr);
	}
}
}  // namespace spu
