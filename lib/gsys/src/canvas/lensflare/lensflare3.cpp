//
// Lensflare3 :
//
#include "lensflare3_inspector.h"
#include <smath/quatf.h>

namespace spu::gs_canvas {

namespace {
const int32_t c_postProcessingSize = 1024;  // need fix
const char *c_maskTex = {"canvas/lensflare3/mask.dds"};
const Vec4f c_cameraMixCoeff = {1.2, 0.80, 0.10, 0.0};
const Vec4f c_filmicMixCoeff = {0.6, 0.55, 0.08, 0.0};
}  // namespace

void Lensflare3::addCommonUniforms(SpuShader &shader)
{
	Attrs unif_attrs = {
	        {"ub_connect",        &ub_connect       },
                {"u_texcoord_scaler", &u_texcoord_scaler},
	        {"u_color_coeff",     &u_color_coeff    },
                {"u_blur_coeff",      &u_blur_coeff     },
	        {"u_mix_coeff",       &u_mix_coeff      },
                {"u_step_size",       &u_step_size      },
	        {"u_stride",          &u_stride         },
                {"u_color0",          &u_color0         },
	        {"u_color1",          &u_color1         },
                {"u_color2",          &u_color2         },
	        {"u_color3",          &u_color3         },
                {"u_color4",          &u_color4         },
	        {"u_color5",          &u_color5         },
	};
	shader.addUniforms(unif_attrs);
}

Lensflare3::Modulator::Modulator()
{
	auto modulateR = [](Vec4f &dst, float src, const Vec3f &mod) {
		dst.r = src * mod.r;
		dst.g = dst.r * mod.g;
		dst.b = dst.r * mod.b;
		dst.a = src;
	};

	auto modulate = [](Vec4f &dst, float src, const Vec3f &mod) {
		dst.r = src * mod.r;
		dst.g = src * mod.g;
		dst.b = src * mod.b;
		dst.a = src;
	};

	const auto BLUE_SHIFT0B = Vec3f(1.00, 1.0, 1.00);
	const auto BLUE_SHIFT1B = Vec3f(0.20, 0.3, 0.95);
	const auto BLUE_SHIFT2B = Vec3f(0.10, 0.2, 0.90);
	const auto BLUE_SHIFT3B = Vec3f(0.02, 0.1, 0.99);

	// clang-format off
	// star
	Vec3f base[][4] = {
		// start
	        {{1.0, 0.95, 0.9}, {0.8, 1.00, 0.9}, {0.9, 0.90, 1.0},  {0.9, 1.00, 0.9}  },
	        {{1.0, 0.90, 0.8}, {1.0, 0.60, 0.5}, {0.5, 1.00, 0.6},  {0.6, 0.40, 1.0}  },
	        {{1.0, 1.00, 1.0}, {1.0, 0.60, 0.6}, {0.6, 1.00, 0.6},  {0.6, 0.60, 1.0}  },

		// horizontal
	        {BLUE_SHIFT1B,     BLUE_SHIFT1B,     BLUE_SHIFT2B,      BLUE_SHIFT1B      },
	        {BLUE_SHIFT1B,     BLUE_SHIFT2B,     BLUE_SHIFT3B,      BLUE_SHIFT3B      },
	        {BLUE_SHIFT0B,     BLUE_SHIFT0B,     BLUE_SHIFT0B,      BLUE_SHIFT0B      },

		// ghost camerama
	        {{1.0, 0.9, 0.8},  {1.0, 0.6, 0.5},  {0.5, 1.0, 0.6},   {1.0, 0.7, 0.3}   },
	        {{0.2, 0.3, 0.7},  {0.5, 0.3, 0.2},  {0.1, 0.5, 0.2},   {0.1, 0.1, 1.0}   },

		// ghost filmic
	        {{0.1, 0.1, 1.0},  {0.2, 0.30, 1.0}, {0.10, 0.20, 0.6}, {0.60, 0.30, 1.0} },
	        {{0.6, 0.2, 0.2},  {0.2, 0.06, 0.6}, {0.15, 0.00, 0.1}, {0.06, 0.00, 0.55}},
	};
	// clang-format on

	for (auto i = 0; i < 4; i++) {
		modulateR(starA[i], 0.25, base[0][i]);
		modulateR(starB[i], 0.25, base[1][i]);
		modulateR(starC[i], 0.25, base[2][i]);

		modulate(horiA[i], 0.5, base[3][i]);
		modulate(horiB[i], 0.5, base[4][i]);
		modulate(horiC[i], 0.5, base[5][i]);

		modulateR(camera_ghostA[i], 1.0, base[6][i]);
		modulateR(camera_ghostB[i], 1.0, base[7][i]);

		modulate(filmic_ghostA[i], 1.0, base[8][i]);
		modulate(filmic_ghostB[i], 1.0, base[9][i]);
	}
}

void Lensflare3::init(const Attrs &attrs)
{
	auto *current = getCurrent();
	auto viewport0 = attrs.get("viewport0", current->viewport(0));

	const auto s02 = c_postProcessingSize / 2;
	const auto s04 = c_postProcessingSize / 4;
	const auto s08 = c_postProcessingSize / 8;
	const auto s16 = c_postProcessingSize / 16;
	const auto s32 = c_postProcessingSize / 32;

	const auto width = int32_t(viewport0.sx);
	const auto height = int32_t(viewport0.sy);

	m_aspectRatio = float(width) / float(height);

	{
		const Attrs def_attrs = {
		        {"def_blend_max", "1"},
                        {"size",          s16}, // must be 16^n
		};
		Tonemap::init(def_attrs + attrs);
	}

	// buffers
	{
		initFrame(m_workFrame, s04, c_levelTotal);
		for (auto i = 0; i < 4; i++) {
			initFrame(m_streakFramesA[i], s04, 1);
			initFrame(m_streakFramesB[i], s04, 1);
		}

		initFrame(m_streakFrame, s04, 1);
		initFrame(m_cameraGhostFrame, s02, 1);
		initFrame(m_filmicGhostFrame, s02, 1);
	}

	// canvas
	{
		Attrs aux_attrs = {
		        {"path", "canvas/lensflare3/composite.us"},
		};
		initCanvas(m_compositeCanvas, s02, aux_attrs);
		addCommonUniforms(m_compositeCanvas.getShader());
	}
	// gauss
	{
		initCanvas(m_gaussCanvas0, s04);
		initCanvas(m_gaussCanvas1, s08);
		initCanvas(m_gaussCanvas2, s16);
		initCanvas(m_gaussCanvas3, s32);
	}

	// lensmask
	{
		const Attrs attrs = {
		        {"min_filter", GL_LINEAR       },
		        {"mag_filter", GL_LINEAR       },
		        {"wrap_s",     GL_CLAMP_TO_EDGE},
		        {"wrap_t",     GL_CLAMP_TO_EDGE},
		};
		m_lensMask = spu_inventory_new("texture", c_maskTex, attrs);
	}

	// shaders
	{
		m_extractShader.init("canvas/lensflare3/extract.us");
		m_starCompositeShader.init("canvas/lensflare3/star_composite.us");
		m_starStreakShader.init("canvas/lensflare3/star_streak.us");
		m_ghostImageShader.init("canvas/lensflare3/ghost_image.us");

		addCommonUniforms(m_extractShader);
		addCommonUniforms(m_starCompositeShader);
		addCommonUniforms(m_starStreakShader);
		addCommonUniforms(m_ghostImageShader);
	}
}

void Lensflare3::set(const Attrs &attrs)
{
	attrs.peek("gain", "deprecated");
	attrs.peek("u_luminance_threshold", "use 'luminance_thresholde' instead");
	attrs.peek("u_luminance_scaler", "use 'luminance_scaler' instead");

	attrs.peek("luminance_threshold", "use 'bias' instead");
	attrs.peek("luminance_scalar", "use 'gain' instead");
	attrs.peek("glare_type", "use 'type' instead");

	attrs.apply("bias", m_bias);
	attrs.apply("type", m_type);
	Tonemap::set(attrs);
}

void Lensflare3::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new Lensflare3Inspector(this);
	}
}

//
// common:
//   operation  to            from
//   -----------------------------------------
//   extract    workFrame     input
//   gauss      gaussCanvas   workFrame
//
// starStreak:
//   operation  to            from
//   -----------------------------------------
//   modulate   streakFrameA  workFrame
//   modulate   streakFrameB  streakFrameA
//   modulate   streakFrameA  streakFrameB
//
// horizontalStreak
//   operation  to            from
//   -----------------------------------------
//   modulate   streakFrameB  workFrame
//   modulate   streakFrameA  streakFrameB
//   modulate   streakFrameB  streakFrameA
//   modulate   streakFrameA  streakFrameB
//
// ghost
//   operation  to          from
//   -----------------------------------------
//   ghost     ghostFrameA  gaussCanvas
//   ghost     ghostFrameB  gaussCanvas
//
// starComp
//   operation  to          from
//   -----------------------------------------
//   starComp   streakFrame streakFramesA
//
// composite
//   operation  to             from
//   -----------------------------------------
//   composite  compositeCanvas gaussCanvas[0-3] streakFrame, ghostFrameB
//

namespace {
[[maybe_unused]] inline void full_clear(SpuFrame &frame)
{
	Vec4f bgcolor0 = {0, 0, 0, 0};
	auto max_level = 0;
	frame.getBuffer("color0").get("max_level", &max_level);
	for (auto level = 0; level < max_level; level++) {
		int32_t loc[4] = {0, 0, 0, level};
		frame.getBuffer("color0").send(&bgcolor0, GL_R32F, loc, nullptr, true);  // clear
	}
}
}  // namespace

void Lensflare3::render()
{
	auto u_color0_save = u_color0;
	auto u_color1_save = u_color1;

	SpuScopedRenderstate renderstate = getRenderstate();
	renderstate.use();

	{
		auto ub_connect_save = ub_connect;
		// ub_connect.sources[0].gain = m_gain;
		ub_connect.sources[0].bias = m_bias;
		drawInternal(m_extractShader, m_workFrame);
		ub_connect = ub_connect_save;
	}

	{
		m_gaussCanvas0.u_color0 = m_workFrame.getBuffer("color0").id();
		m_gaussCanvas0.u_variance = 2.0;
		m_gaussCanvas0.begin();
		m_gaussCanvas0.render();
		m_gaussCanvas0.end();
	}
	{
		m_gaussCanvas1.u_color0 = m_gaussCanvas0.getBuffer("color0").id();
		m_gaussCanvas1.u_variance = 4.0;
		m_gaussCanvas1.begin();
		m_gaussCanvas1.render();
		m_gaussCanvas1.end();
	}
	{
		m_gaussCanvas2.u_color0 = m_gaussCanvas1.getBuffer("color0").id();
		m_gaussCanvas2.u_variance = 4.0;
		m_gaussCanvas2.begin();
		m_gaussCanvas2.render();
		m_gaussCanvas2.end();
	}
	{
		m_gaussCanvas3.u_color0 = m_gaussCanvas2.getBuffer("color0").id();
		m_gaussCanvas3.u_variance = 4.0;
		m_gaussCanvas3.begin();
		m_gaussCanvas3.render();
		m_gaussCanvas3.end();
	}

	// create glare
	{
		switch (m_type) {
		case e_camera:
			starGlare(0);
			starGlare(1);
			starGlare(2);
			starGlare(3);
			cameraGhostImage(m_modulator.camera_ghostA, m_modulator.camera_ghostB);
			break;
		case e_filmic:
			horizontalGlare(0);
			horizontalGlare(1);
			filmicGhostImage(m_modulator.filmic_ghostA, m_modulator.filmic_ghostB);
			break;
		default: assert(0);
		}
	}

	// composite glare
	{
		switch (m_type) {
		case e_camera:
			u_color0 = m_streakFramesA[0].getBuffer("color0").id();
			u_color1 = m_streakFramesA[1].getBuffer("color0").id();
			u_color2 = m_streakFramesA[2].getBuffer("color0").id();
			u_color3 = m_streakFramesA[3].getBuffer("color0").id();
			break;

		case e_filmic:
			u_color0 = m_streakFramesA[0].getBuffer("color0").id();
			u_color1 = m_streakFramesA[1].getBuffer("color0").id();
			u_color2 = defaultBlackTexture().id();  // need fix
			u_color3 = defaultBlackTexture().id();  // needf fix

			break;
		default: assert(0);
		}
		drawInternal(m_starCompositeShader, m_streakFrame /*, 0*/);
	}

	// final composite
	{
		u_color0 = m_gaussCanvas0.getBuffer("color0").id();
		u_color1 = m_gaussCanvas1.getBuffer("color0").id();
		u_color2 = m_gaussCanvas2.getBuffer("color0").id();
		u_color3 = m_gaussCanvas3.getBuffer("color0").id();
		u_color4 = m_streakFrame.getBuffer("color0").id();
		u_blur_coeff = Vec4f(0.3, 0.3, 0.25, 0.20);

		switch (m_type) {
		case e_camera:
			u_color5 = m_cameraGhostFrame.getBuffer("color0").id();
			u_mix_coeff = c_cameraMixCoeff;
			break;
		case e_filmic:
			u_color5 = m_filmicGhostFrame.getBuffer("color0").id();
			u_mix_coeff = c_filmicMixCoeff;
			break;
		default: assert(0);
		}
		m_compositeCanvas.begin();
		m_compositeCanvas.render();
		m_compositeCanvas.end();
	}
	u_color0 = u_color0_save;
	u_color1 = m_compositeCanvas.getBuffer("color0").id();
	Tonemap::render();
	u_color1 = u_color1_save;
}

void Lensflare3::initFrame(SpuFrame &frame, int32_t size, int32_t levels)
{
	auto viewport = Rectf(0, 0, size, size);
	auto max_level = levels;
	Attrs attrs = {
	        {"viewport0",          viewport               },
	        {"color0.target",      GL_TEXTURE_2D          },
	        {"color0.iformat",     GL_RGBA16F             },
	        {"color0.min_filter",  GL_LINEAR_MIPMAP_LINEAR},
	        {"color0.mag_filter",  GL_LINEAR              },
	        {"color0.wrap_s",      GL_CLAMP_TO_EDGE       },
	        {"color0.wrap_t",      GL_CLAMP_TO_EDGE       },
	        {"color0.max_level",   max_level              },
	        {"color0.auto_mipmap", 0                      },
	};
	frame.init(attrs);
}
void Lensflare3::initCanvas(GsCanvas &canvas, int32_t size, const Attrs &aux_attrs)
{
	auto viewport = Rectf(0, 0, size, size);
	Attrs attrs = {
	        {"viewport0",          viewport               },
	        {"color0.target",      GL_TEXTURE_2D          },
	        {"color0.iformat",     GL_RGBA16F             },
	        {"color0.min_filter",  GL_LINEAR_MIPMAP_LINEAR},
	        {"color0.mag_filter",  GL_LINEAR              },
	        {"color0.wrap_s",      GL_CLAMP_TO_EDGE       },
	        {"color0.wrap_t",      GL_CLAMP_TO_EDGE       },
	        {"color0.max_level",   0                      },
	        {"color0.auto_mipmap", 0                      },
	};
	canvas.init(attrs + aux_attrs);
}

void Lensflare3::starStreak(
        float DEC, const Vec2f &step_size, SpuFrame &dst, const Vec4f base_modulation[], int32_t level,
        SpuFrame &src)
{
	const float stride = powf(4.0f, float(level - 1));
	u_stride = stride;
	u_step_size = step_size;
	for (auto i = 0; i < 4; i++) {
		u_color_coeff[i] = base_modulation[i] * powf(DEC, stride * float(i));
	}
	u_color0 = src.getBuffer("color0").id();
	drawInternal(m_starStreakShader, dst /*, src.getBuffer("color0")*/);
}

void Lensflare3::starGlare(int32_t dir)
{
	const auto DEC = 0.9f;
	const auto delta = 0.9f;
	auto size = getSize(m_workFrame);

	assert(size.x == c_postProcessingSize / 4);
	assert(size.y == c_postProcessingSize / 4);

	const auto dx = delta / size.x;
	const auto dy = delta / size.y * m_aspectRatio;

	Vec2f step;

	switch (dir) {
	case 0:
		step.f[0] = +dx;
		step.f[1] = +dy;
		break;
	case 1:
		step.f[0] = -dx;
		step.f[1] = +dy;
		break;
	case 2:
		step.f[0] = +dx;
		step.f[1] = -dy;
		break;
	case 3:
		step.f[0] = -dx;
		step.f[1] = -dy;
		break;
	default: assert(0);
	}

	// Note: CLAMP_TO_EDGE may cause artifact due to stride is clamped
	// 3 passes to generate 64 pixel blur in each direction

	// composFrame -> streakFramesA
	starStreak(DEC, step, m_streakFramesA[dir], m_modulator.starA, 1, m_workFrame);

	// streakFramesA -> streakFramesB
	starStreak(DEC, step, m_streakFramesB[dir], m_modulator.starB, 2, m_streakFramesA[dir]);

	// streakFrameB -> streakFramesA
	starStreak(DEC, step, m_streakFramesA[dir], m_modulator.starB, 3, m_streakFramesB[dir]);
}

void Lensflare3::horizontalGlare(int32_t dir)
{
	const auto delta = 0.9;
	const auto DEC = 0.96;

	auto size = getSize(m_workFrame);

	assert(size.x == c_postProcessingSize / 4);
	assert(size.y == c_postProcessingSize / 4);

	const auto dx = delta / size.x;

	Vec2f step;
	step.f[0] = dir == 0 ? +dx : -dx;
	step.f[1] = 0;

	// 4 passes to generate 256 pixel blur in each direction

	// compose -> sreakFramesB
	starStreak(DEC, step, m_streakFramesB[dir], m_modulator.horiA, 1, m_workFrame);

	// streamFrameB -> streakFramesA
	starStreak(DEC, step, m_streakFramesA[dir], m_modulator.horiA, 2, m_streakFramesB[dir]);

	// streamFrameA -> streakFramesB
	starStreak(DEC, step, m_streakFramesB[dir], m_modulator.horiB, 3, m_streakFramesA[dir]);

	// streamFrameB -> streakFramesA
	starStreak(DEC, step, m_streakFramesA[dir], m_modulator.horiC, 4, m_streakFramesB[dir]);
}

void Lensflare3::cameraGhostImage(const Vec4f ghostA[4], const Vec4f[4])
{
	u_color0 = m_gaussCanvas0.getBuffer("color0").id();
	u_color1 = m_gaussCanvas1.getBuffer("color0").id();  // ??
	u_color2 = m_gaussCanvas1.getBuffer("color0").id();  // ??
	u_color3 = m_lensMask;
	u_texcoord_scaler = Vec4f(-4.0, 3.0, -2.0, 0.3);

	memcpy(u_color_coeff, ghostA, sizeof(u_color_coeff));
	drawInternal(m_ghostImageShader, m_cameraGhostFrame /*, 0*/);
}
void Lensflare3::filmicGhostImage(const Vec4f[4], const Vec4f ghostB[4])
{
	u_color0 = m_gaussCanvas0.getBuffer("color0").id();
	u_color1 = m_gaussCanvas0.getBuffer("color0").id();  // ??
	u_color2 = m_gaussCanvas1.getBuffer("color0").id();  // ??
	u_color3 = m_lensMask;
	u_texcoord_scaler = Vec4f(3.6, 2.0, 0.9, -0.55);

	memcpy(u_color_coeff, ghostB, sizeof(u_color_coeff));
	drawInternal(m_ghostImageShader, m_filmicGhostFrame /*, 0*/);
}

void Lensflare3::drawInternal(SpuShader &shader, SpuFrame &dst)
{
	dst.begin();
	shader.use();
	spu_array_draw(0, GL_TRIANGLE_STRIP);
	dst.end();
}
}  // namespace spu::gs_canvas
