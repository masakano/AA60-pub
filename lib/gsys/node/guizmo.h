//
// Guizmo :
//
#pragma once

#include <gsys/painter/stdout.h>
#include <gsys/node.h>
#include <smath/object_picker.h>

namespace spu::gs_node {

/// node manipulator
class Guizmo : public GsNode {
public:
	explicit Guizmo(const char *name = nullptr) : GsNode(name) { m_gesture.init(nullptr); }
	explicit Guizmo(const Attrs &attrs) : Guizmo() { init(attrs); }

	void init(const Attrs &attrs) override;
	void update() override;
	void setRelatedNodes(const std::vector<GsNode *> &nodes) override;
	void begin();
	void end();
	void setVisible(int32_t index, bool visible);

	ObjectPicker &getPicker() { return m_picker; }
	const ObjectPicker &getPicker() const { return m_picker; }

	const Transformf &getPrimTransform(int32_t index) const { return m_picker.getPrim(index).transform; }
	Transformf &getPrimTransform(int32_t index) { return m_picker.getPrim(index).transform; }

	template<class T> void addPrim(const T &object, const Transformf &nodeworld = Transformf())
	{
		internalAddPrim(object, nodeworld, nullptr, 0, 0);
	}

private:
	struct Element {
		bool visible;
		uint32_t first;
		uint32_t count;
		GsNode *node = nullptr;
		uint32_t slot_index = 0;
		uint32_t instance_index = 0;
		Mat4f initial_transform = Mat4f();  // includes scale
	};
	std::vector<Element> m_elements;
	ObjectPicker m_picker;
	SpuGesture m_gesture;

	template<class T>
	void internalAddPrim(
	        const T &prim, const Transformf &nodeworld, GsNode *node, uint32_t slot_index,
	        uint32_t instance_index, const Mat4f &initial_transform = Mat4f())
	{
		auto *painter = dynamic_cast<gs_painter::Stdout *>(getPainter());
		auto &line_indices = painter->subIndices().at(GL_LINES);
		auto first = line_indices.size();
		painter->addPrim(prim);

		auto count = line_indices.size() - first;
		auto element
		        = Element(false, first, count, node, slot_index, instance_index, initial_transform);

		m_elements.push_back(element);
		m_picker.add({prim, nodeworld});
	}
	bool doSync(bool is_nonblock) override;
	void doRender() override;
	void addNode(GsNode *node);
};

}  // namespace spu::gs_node
