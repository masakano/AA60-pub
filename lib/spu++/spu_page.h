//
// SpuPage :
//
#pragma once
#include "spu++.h"
#include <ssys/object_registry.h>
#include <ssys/random_generator.h>

namespace spu {

class SpuPage {
public:
	SpuPage(const char *name, bool depth_test = false, const Vec4f &bgcolor0 = Vec4f(0.0, 0.0, 0.0, 1.0),
	        float bgdepth = 1.0, int32_t stencil = -1);

	virtual ~SpuPage();

	virtual void init(const Attrs &);
	virtual void begin();
	virtual void update() {};
	virtual void render() {};
	virtual void end();

	virtual void clear() { spu_frame_clear(-1); }
	virtual void set(const Attrs &attrs) { spu_frame_set(-1, attrs); }

	float frand() { return m_frand(); }
	SpuRenderstate &getRenderstate() { return m_renderstate; }
	Seconds &getSeconds() { return m_seconds; }

	const Rectf &viewport(int index) const { assert(index == 0); return m_viewport0; }

	const std::string &name() const { return m_name; }
	bool isAlive() const { return m_isAlive; }

	SpuGesture *getGesture() const { return m_gesture; }
	void replaceGesture(SpuGesture *gesture) { reset(m_gesture, gesture); }
	[[deprecated("use replaceGesture()")]] void setGesture(SpuGesture *gesture) { replaceGesture(gesture); }

private:
	template<class T> void reset(T *&ptr, T *new_ptr)
	{
		delete ptr;
		ptr = new_ptr;
	}
	std::string m_name;
	RandomGenerator<float> m_frand = {0.0, 1.0};
	Vec4f m_bgcolor0;
	float m_bgdepth;
	int32_t m_bgstencil;

	bool m_isAlive = true;
	SpuRenderstate m_renderstate;
	Seconds m_seconds;
	Rectf m_viewport0;
	SpuGesture *m_gesture = nullptr;
};

extern template class ObjectRegistry<SpuPage>;
}  // namespace spu
