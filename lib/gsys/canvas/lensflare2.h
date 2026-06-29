//
// Lensflare2 :
//
#pragma once
#include <gsys/canvas/tonemap.h>

namespace spu::gs_canvas {

class Lensflare2 : public Tonemap {
public:
	enum OutputMode {
		e_source_only = 0,
		e_bloom_only,
		e_flare_only,
		e_all,
	};

	explicit Lensflare2(const char *name = nullptr) : Tonemap(name) {}
	explicit Lensflare2(const Attrs &attrs) : Lensflare2() { init(attrs); }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void render() override;
	void startInspector() override;

	const GsCanvas &scaleBiasCanvas() const { return m_scaleBiasCanvas; }
	const GsCanvas &flareCanvas() const { return m_flareCanvas; }
	const GsCanvas &bloomCanvas() const { return m_bloomCanvas; }

protected:
	friend class Lensflare2Inspector;

	SpuTexture m_lensColorTexture;
	SpuTexture m_lensDirtTexture;  // not used for now
	SpuTexture m_lensStarTexture;  // not used for now
	GsCanvas m_gbufferCanvas;

	GsCanvas m_scaleBiasCanvas;
	GsCanvas m_gaussCanvas;
	GsCanvas m_bloomCanvas;
	GsCanvas m_flareCanvas;

	float m_bloomGain = 0.5;
	float m_bloomBias = -1.0;
	float m_bloomBlurRadius = 64.0;

	float m_flareGain = 0.5;
	float m_flareBias = -1.0;
	float m_flareBlurRadius = 24.0;
	float m_flareSamples = 8.0;

	float m_flareDispersal = 0.3;
	float m_flareHaloWidth = 0.6;
	float m_flareDistortion = 1.5;

	int32_t m_outputMode = e_all;

	Mat4f u_lens_star_matrix;
	Vec2f u_blur_direction = Vec2f(0);

	float u_flare_dispersal = 0.1;
	float u_flare_halo_width = 0.6;
	float u_flare_distortion = 1.5;

	uint32_t u_lens_flare_texture = 0;
	uint32_t u_lens_dirt_texture = 0;
	uint32_t u_lens_star_texture = 0;
	uint32_t u_lens_color_texture = 0;
	uint32_t u_input_texture = 0;
	int32_t u_blur_radius = 24;
	int32_t u_flare_samples = 8;

	void gaussBlur(GsCanvas &canvas, uint32_t input_texture, float blur_radius);
};
}  // namespace spu::gs_canvas
