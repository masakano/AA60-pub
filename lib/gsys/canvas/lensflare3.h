//
// Lensflare3 :
//
#pragma once

#include <gsys/canvas/tonemap.h>
#include <gsys/canvas/gauss.h>

namespace spu::gs_canvas {

/// intergrated image based lighting effect
class Lensflare3 : public Tonemap {
public:
	enum GlareType {
		e_camera = 0,
		e_filmic,
	};

	explicit Lensflare3(const char *name = nullptr) : Tonemap(name) {}
	explicit Lensflare3(const Attrs &attrs) : Lensflare3() { init(attrs); }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void render() override;
	void startInspector() override;

protected:
	friend class Lensflare3Inspector;  // view ghostFrame, streakFrame

	static constexpr int32_t c_levelTotal = 6;

	class Modulator {
	public:
		Vec4f starA[4];
		Vec4f starB[4];
		Vec4f starC[4];

		Vec4f filmic_ghostA[4];
		Vec4f filmic_ghostB[4];
		Vec4f camera_ghostA[4];
		Vec4f camera_ghostB[4];

		Vec4f horiA[4];
		Vec4f horiB[4];
		Vec4f horiC[4];

		Modulator();
	};
	Modulator m_modulator;
	SpuFrame m_workFrame;
	SpuFrame m_streakFramesA[4];
	SpuFrame m_streakFramesB[4];
	SpuFrame m_streakFrame;
	SpuFrame m_cameraGhostFrame;
	SpuFrame m_filmicGhostFrame;
	SpuShader m_extractShader;
	SpuShader m_starCompositeShader;
	SpuShader m_starStreakShader;
	SpuShader m_ghostImageShader;

	gs_canvas::Gauss2D m_gaussCanvas0;
	gs_canvas::Gauss2D m_gaussCanvas1;
	gs_canvas::Gauss2D m_gaussCanvas2;
	gs_canvas::Gauss2D m_gaussCanvas3;

	GsCanvas m_compositeCanvas;
	// SpuArray m_array;
	float m_aspectRatio = 1.0;

	float m_bias = -0.5;
	// float m_gain = 1.0;

	uint32_t m_lensMask = 0;
	int32_t m_type = e_camera;

	Vec4f u_color_coeff[4];
	Vec4f u_blur_coeff = ezero<Vec4f>();
	Vec4f u_texcoord_scaler = ezero<Vec4f>();
	Vec4f u_mix_coeff = ezero<Vec4f>();
	Vec2f u_step_size = Vec2f(0);
	float u_stride = 0;

	Vec4i getSize(const SpuFrame &frame) const
	{
		Rectf viewport;
		frame.get("viewport0", &viewport);
		return Vec4i(viewport.sx, viewport.sy, 0, 0);
	}

	void addCommonUniforms(SpuShader &shader);
	void horizontalGlare(int32_t dir);
	void starGlare(int32_t dir);

	void cameraGhostImage(const Vec4f ghostA[4], const Vec4f ghostB[4]);
	void filmicGhostImage(const Vec4f ghostA[4], const Vec4f ghostB[4]);
	void starStreak(
	        float DEC, const Vec2f &step_size, SpuFrame &dst, const Vec4f base_modulation[], int32_t level,
	        SpuFrame &src);

	void drawInternal(SpuShader &shader, SpuFrame &dst);
	void drawInternal(SpuShader &shader, SpuFrame &dst, uint32_t src_texture);

	static void initFrame(SpuFrame &frame, int32_t size, int32_t levels);
	static void initCanvas(GsCanvas &canvas, int32_t size, const Attrs &aux_attrs = Attrs());
};
}  // namespace spu::gs_canvas
