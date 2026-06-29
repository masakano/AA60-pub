//
// Shadowmap :
//
#pragma once

#include <gsys/node.h>
#include <gsys/canvas/gauss.h>
#include <smath/shadow_frustumf.h>
#include <gsys/shaders/default/ub_shadowmap.us>

namespace spu::gs_canvas {
class ShadowmapInspector;
class Shadowmap : public GsCanvas {
public:
	struct Drawfunc {
		std::function<void()> func = nullptr;
		std::vector<Vec3f> points;
	};

	enum {
		e_poisson_32 = 32,
		e_poisson_64 = 64,
		e_poisson_128 = 128,
	};

	explicit Shadowmap(const char *name = nullptr) : GsCanvas(name) {}
	explicit Shadowmap(const Attrs &attrs) : Shadowmap() { init(attrs); }

	~Shadowmap();

	Drawfunc &getDrawfunc() { return m_drawfunc; }
	const Drawfunc &getDrawfunc() const { return m_drawfunc; }

	Gauss2D *gaussCanvas() const { return m_gaussCanvas; }
	int32_t layerCount() const { return m_layerCount; }

	const Composition &shadowComposition() const { return m_shadowComposition; }
	const Composition &cameraComposition() const { return m_cameraComposition; }

	// const Attrs &shaderAttrs() const { return m_shaderAttrs; }
	const int &blockerPattern() const { return m_blockerPattern; }
	const int &receiverPattern() const { return m_receiverPattern; }

	UB_SHADOWMAP makeUniform(const GsCanvas *current = nullptr) const;
	const SpuRenderstate::PolyOffset &polyOffset() const { return m_polyOffset; }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	using GsObject::set;
	void update() override;
	const SpuTexture &getBuffer(const hash32_t &slot = "color0") const override;
	SpuTexture &getBuffer(const hash32_t &slot = "color0") override;
	void startInspector() override;

protected:
	friend ShadowmapInspector;

	Drawfunc m_drawfunc;
	SpuRenderstate::PolyOffset m_polyOffset;

	Composition m_shadowComposition;
	Composition m_cameraComposition;

	int32_t m_blockerPattern = e_poisson_32;
	int32_t m_receiverPattern = e_poisson_32;

	Gauss2D *m_gaussCanvas = nullptr;
	int32_t m_layerCount = 1;
	float m_variance = 1.0;
	float m_footstep = 1.0;

	void setup();
	Mat4f retriveShadowFrustum();
	void postFilter();
	void render() override { aux_message(0, "shadowmap canvas does not render anything. (ignored)\n"); }
};

class CascadeShadowmap : public Shadowmap {
public:
	explicit CascadeShadowmap(const char *name = nullptr) : Shadowmap(name) {}
	explicit CascadeShadowmap(const Attrs &attrs) { init(attrs); }
	void init(const Attrs &attrs) override;
	void update() override;

protected:
	std::vector<Mat4f> m_shifts;
	Mat4f m_screenfrag;
	Vec3f m_minSpan = ezero();
	float m_strength = 0.75;
	bool def_use_multi_viewport = false;

	float splitDepth(float full_near, float full_far, float split_rate) const;
	void splitShadowViewscreen(const Mat4f &shadow_worldscreen);
	void standardCapture(const std::function<void()> &drawfunc);
	void multiViewportCapture(const Mat4f &shadow_worldscreen, const std::function<void()> &drawfunc);
};

/// cube shadow mapping
class CubeShadowmap : public Shadowmap {
public:
	explicit CubeShadowmap(const char *name = nullptr) : Shadowmap(name) {}
	explicit CubeShadowmap(const Attrs &attrs) : CubeShadowmap() { init(attrs); }

	void init(const Attrs &attrs) override;
	void update() override;

protected:
	Mat4f cubeViewscreen(const Vec3f &light_position) const;
};

/// soft shadowmapping
class PenumbraShadowmap : public Shadowmap {
public:
	explicit PenumbraShadowmap(const char *name = nullptr) : Shadowmap(name) {}
	explicit PenumbraShadowmap(const Attrs &attrs) : PenumbraShadowmap() { init(attrs); }

	void init(const Attrs &attrs) override;
	void update() override;

protected:
	std::vector<Vec3f> m_jitters;
	float m_radius = -1.0;  // cache
	void setRadius(float radius);
};
}  // namespace spu::gs_canvas
