//
// Camera :
//
#pragma once

#include <gsys/node/icamera.h>
#include <smath/geometry.h>
#include <ssys/serializer.h>

namespace spu::gs_node {

class Camera : public ICamera {
public:
	static constexpr hash32_t e_orbital = "orbital";
	static constexpr hash32_t e_fps = "fps";
	static constexpr hash32_t e_swipe = "swipe";
	static constexpr hash32_t e_birdview = "birdview";
	static constexpr hash32_t e_active = "active";

	explicit Camera(const char *name = nullptr) : ICamera(name) {}
	explicit Camera(const Attrs &attrs) : Camera() { init(attrs); }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void update() override;

protected:
	enum {
		e_none = 0,
		e_rotate = 1,
		e_translate = 2,
	};

	struct Watch {
		GsNode *node = nullptr;
		Range3f *range = nullptr;
	} m_watch;

	struct Anchor {
		Vec2f cursor;
		Transformf substance;
		Composition composition;
		Vec3f point;
	} m_anchor;

	Plane3f m_referencePlane = Plane3f({0, 1, 0, 0});
	float m_inertia = 0.1;
	hash32_t m_mode = "orbital";
	uint32_t m_action = 0;

	Plane3f getTargetPlane(const Line3f &anchor_ray) const;

	void recordAnchor();
	void autoAdjust();
	bool doSync(bool is_nonblock) override;

	virtual void doWheel();
	virtual void doDrag();
	virtual void doSave();
	virtual void doLoad();

	SPU_SERIALIZER_FRIENDS
};
}  // namespace spu::gs_node

namespace spu {
template<> size_t serialize(uint8_t *heap, bool is_dry, const gs_node::Camera &o);
template<> size_t deserialize(const uint8_t *heap, gs_node::Camera &o);
}  // namespace spu
