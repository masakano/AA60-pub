//
// Bloom :
//
#pragma once
#include <gsys/canvas/gauss.h>
#include <gsys/canvas/tonemap.h>

namespace spu::gs_canvas {

/// light bloom
class Bloom : public Tonemap {
public:
	explicit Bloom(const char *name = nullptr) : Tonemap(name) {}
	explicit Bloom(const Attrs &attrs) : Bloom() { init(attrs); }
	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void render() override;
	void startInspector() override;

private:
	friend class BloomInspector;
	Gauss2D m_gauss2dH;
	Gauss2D m_gauss2dQ;
	float m_variance = 1.0;
	float m_footstep = 1.0;
	float m_bias = -2.0;
	float m_gain = 1.0;
};
}  // namespace spu::gs_canvas
