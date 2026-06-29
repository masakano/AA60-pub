//
// Lensflare1 :
//
#pragma once
#include <gsys/canvas/tonemap.h>
#include <gsys/canvas/gauss.h>

namespace spu::gs_canvas {

/// halo and god rays effect with dirt lens dispersal
class Lensflare1 : public Tonemap {
public:
	explicit Lensflare1(const char *name = nullptr) : Tonemap(name) {}
	explicit Lensflare1(const Attrs &attrs) : Lensflare1() { init(attrs); }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void render() override;
	void startInspector() override;

protected:
	friend class Lensflare1Inspector;
	Gauss2D m_gaussH;  // half
	Gauss2D m_gaussF;  // full
	GsCanvas m_halo;

	float m_bias = -2.0;
	float m_varianceH = 0.2;
	float m_varianceF = 12.0;

	Vec3f u_distortion = {0.94, 0.97, 1.00};
	Vec2f u_sun_pos_frag = Vec2f(0.0);
	float u_dispersal = 0.1675;
	float u_halo_width = 0.45;
	uint32_t u_dirt_texture = 0;
	uint32_t u_is_blur = 1;
};
}  // namespace spu::gs_canvas
