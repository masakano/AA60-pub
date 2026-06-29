//
// Gauss1D :
//
#pragma once
#include <gsys/canvas.h>

namespace spu::gs_canvas {

class Gauss1D : public GsCanvas {
public:
	explicit Gauss1D(const char *name = nullptr) : GsCanvas(name) {}
	explicit Gauss1D(const Attrs &attrs) : Gauss1D() { init(attrs); }
	void init(const Attrs &attrs) override;
	void render() override;

	Vec4f u_footstep = {1, 0, 0, 1};
	uint32_t u_target = GL_TEXTURE_2D;
	float u_variance = 1.0;

private:
	Vec2f u_texture_size;
};

class Gauss2D : public Gauss1D {
public:
	explicit Gauss2D(const char *name = nullptr) : Gauss1D(name) {}
	explicit Gauss2D(const Attrs &attrs) : Gauss2D() { init(attrs); }
	Gauss1D &lazyInitFirstGauss();
	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	using GsObject::set;
	void render() override;
	void startInspector() override;

protected:
	std::string m_firstPath;
	Gauss1D m_firstGauss;
};

// experimental

}  // namespace spu::gs_canvas
