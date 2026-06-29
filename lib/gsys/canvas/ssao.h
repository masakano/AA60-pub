//
// SSAO :
//
#pragma once

#include <gsys/canvas.h>

namespace spu::gs_canvas {

/// screen space ambient occlusion
class SSAO : public GsCanvas {
public:
	explicit SSAO(const char *name = nullptr) : GsCanvas(name) {}
	explicit SSAO(const Attrs &attrs) : SSAO() { init(attrs); }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void render() override;
	void startInspector() override;

protected:
	friend class SSAOInspector;

	GsCanvas m_ssaoCanvas;
	int32_t m_quality = 3; /* 0,1,2,3 */
	int32_t m_prevQuality = 3;

	Mat4f u_texcview;
	Mat4f u_viewtexc;
	Vec4f u_texture_size;
	Vec3f ub_samples[256] = {Vec3f(0)};
	float u_ssao_lerp = 1.0;
	float u_ssao_power = 1.0;
	float u_sample_radius = 0.1;
	uint32_t u_sample_count = 0;
	int32_t u_ssao_only = false;
};
}  // namespace spu::gs_canvas
