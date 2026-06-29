//
// Glare :
//
#pragma once
#include <gsys/canvas/tonemap.h>
#include <gsys/canvas/gauss.h>

namespace spu::gs_canvas {

/// 2-way lens glare
class Glare : public Tonemap {
public:
	explicit Glare(const char *name = nullptr) : Tonemap(name) {}
	explicit Glare(const Attrs &attrs) : Glare() { init(attrs); }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void render() override;
	void startInspector() override;

private:
	friend class GlareInspector;
	Gauss2D m_gauss2dH;
	Gauss2D m_gauss2dQ;
	float m_variance = 1.0;
	float m_footstep = 1.0;
	float m_bias = -2.0;
	float m_gain = 1.0;
};
}  // namespace spu::gs_canvas
