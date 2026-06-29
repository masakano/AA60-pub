//
// Billboard :
//
#pragma once
#include <gsys/node.h>
#include <gsys/canvas/atlas.h>

namespace spu::gs_node {

class Billboard : public GsNode {
public:
	struct Mob {
		Vec3f position;
		int32_t id;
	};

	explicit Billboard(const char *name = nullptr) : GsNode(name) {}
	explicit Billboard(const Attrs &attrs) : Billboard() { init(attrs); }
	~Billboard() = default;

	void setMobs(const std::vector<Mob> &mobs);
	gs_canvas::Atlas &getAtlas() { return m_atlas; }
	const gs_canvas::Atlas &getAtlas() const { return m_atlas; }
	void init(const Attrs &attrs) override;

protected:
	gs_canvas::Atlas m_atlas;
};
}  // namespace spu::gs_node
