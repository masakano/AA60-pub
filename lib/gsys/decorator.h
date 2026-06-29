//
// GsDecorator :
//
#pragma once

#include "object.h"

namespace spu {
class GsPainter;

/// decorate painter
class GsDecorator {
public:
	GsDecorator(GsPainter *painter, const Attrs &) : m_painter(painter) {}

	virtual ~GsDecorator() = default;
	virtual Attrs uniforms() const = 0;
	virtual void set(const Attrs &) {}
	virtual void doUse(uint32_t) {}
	virtual void doRender() {}

protected:
	GsPainter *m_painter = nullptr;  //!< master painter
};
}  // namespace spu
