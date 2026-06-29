//
// Plane :
//
#pragma once
#include <gsys/node.h>
#include <gsys/painter/plane.h>

namespace spu::gs_node {
class Plane : public GsNode {
public:
	explicit Plane(const char *name = nullptr) : GsNode(name) {}
	explicit Plane(const Attrs &attrs) : Plane() { init(attrs); }

protected:
	bool doBuild(const std::vector<Mat4f> &nodeworlds) override
	{
		getARange() = getPainter()->getRange();
		return GsNode::doBuild(nodeworlds);
	}
};
}  // namespace spu::gs_node
