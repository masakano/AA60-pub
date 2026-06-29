//
// Tonemap :
//
#pragma once
#include <gsys/canvas/copy.h>

namespace spu::gs_canvas {

/// Uncharted HDR tonemapping
class Tonemap : public GsCanvas {
public:
	enum {
		e_none = 0,
		e_linear,
		e_reinhard,
		e_hugo,
		e_filmic,
	};

	explicit Tonemap(const char *name = nullptr) : GsCanvas(name) {}
	explicit Tonemap(const Attrs &attrs) : Tonemap() { init(attrs); }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void render() override;
	void startInspector() override;

	const SpuTexture &captureTexture() const
	{
		return m_captureCanvas.getBuffer("color0");
		;
	}
	const SpuTexture &ambientTexture() const { return m_ambientTexture; }
	const SpuTexture &luminanceTexture() const { return m_luminanceTexture; }

protected:
	friend class TonemapInspector;
	static constexpr int32_t c_local_size = 16;
	gs_canvas::Copy m_captureCanvas;
	SpuComputeArray m_adaptArray;
	SpuTexture m_ambientTexture;
	SpuTexture m_luminanceTexture;

	float m_adaptSpeed = 0.01;  // 1% per sec
	int32_t m_size = 0;
	int32_t m_maxLevel = 0;
	
	float u_tonemap_curve_bias = 1.0;
	float u_tonemap_adapt_rate = 0.0;
	float u_tonemap_min_luminance = 0.01;
	int32_t u_tonemap_type = e_reinhard;
	int32_t u_tonemap_is_vignette = true;
	int32_t u_tonemap_is_adaptive_exposure = 1;

	uint32_t u_captured_texture = 0;
	uint32_t u_ambient_texture = 0;
	uint32_t u_luminance_texture = 0;

	void adapt();
};
}  // namespace spu::gs_canvas
