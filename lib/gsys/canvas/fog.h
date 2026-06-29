//
// Fog :
//
#pragma once
#include <gsys/canvas.h>
#include <gsys/shaders/default/ub_shadowmap.us>
#include <gsys/canvas/shadowmap.h>
#include <gsys/shaders/canvas/fog/ub_fbm_fog.us>

namespace spu::gs_canvas {

class Fog : public GsCanvas {
public:
	fog::UB_FBM_FOG ub_fbm_fog = {
	        .speed = {0.05, 0.0, -0.05, 0.0},
	        .amplitude = 0.1,
	        .granularity = 2.0,
	        .lacunarity = 2.0,
	        .octave_decay = 0.5,
	        .density_min = 0.0,
	        .density_max = 1.0,
	        .octaves = 2,
	};
	float u_near = 0;
	float u_far = 8.0;
	float u_step = 64;

	explicit Fog(const char *name = nullptr) : GsCanvas(name) {}
	explicit Fog(const Attrs &attrs) : Fog() { init(attrs); }

	void init(const Attrs &attrs) override;
	void render() override;
	void startInspector() override;

protected:
	GsCanvas m_fogCanvas;
	gs_canvas::Shadowmap *m_shadowmap = nullptr;

	UB_SHADOWMAP ub_shadowmap;
	Mat4f u_worldview;
	Mat4f u_viewworld;
	Mat4f u_screenview;

	float u_esec = 0;
	//float u_near = 0;
	//float u_far = 4.0;

	uint32_t u_lightmap = 0;  // experimental
	uint32_t u_irradmap = 0;
	uint32_t u_shadowmap = 0;

	void createFogCanvas();
};
}  // namespace spu::gs_canvas
