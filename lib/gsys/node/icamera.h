//
// ICamera :
//
#pragma once

#include <gsys/node.h>
#include <gsys/canvas.h>

namespace spu::gs_node {

class ICamera : public GsNode {
public:
	explicit ICamera(const char *name = nullptr) : GsNode(name) {}
	explicit ICamera(const Attrs &attrs) : GsNode() { init(attrs); }

	void init(const Attrs &attrs) override
	{
		GsNode::init(attrs);
		set(attrs);
	}
	void set(const Attrs &attrs) override
	{
		attrs.apply("canvas", m_canvas);
		attrs.apply("depth_canvas", m_depthCanvas);
		attrs.apply("gesture", m_gesture);
		GsNode::set(attrs);
	}
	using GsObject::set;

	void update() override
	{
		aux_error(m_gesture == nullptr, "no gesture\n");
		aux_error(m_canvas == nullptr, "no canvas\n");
		GsNode::update();
	}

	const Transformf &getTargetSubstance() const { return m_targetSubstance; }
	Transformf &getTargetSubstance() { return m_targetSubstance; }

	GsCanvas *getCanvas() const { return m_canvas; }
	GsCanvas *getDepthCanvas() const { return m_depthCanvas ? m_depthCanvas : m_canvas; }
	SpuGesture *getGesture() const { return m_gesture; }

	// protected:
private:
	Transformf m_targetSubstance;
	GsCanvas *m_canvas = nullptr;
	GsCanvas *m_depthCanvas = nullptr;
	SpuGesture *m_gesture = nullptr;
};
}  // namespace spu::gs_node
