//
// CloudyIrradiance :
//
#pragma once

#include <gsys/canvas/lightmap.h>
#include <gsys/shaders/canvas/irradiance/ub_cloudy_irradiance.us>

namespace spu::gs_canvas {
class CloudyIrradianceInspector;
class CloudyIrradiance : public Irradiance {
public:
	static constexpr uint32_t c_lightmap_width = 2048;
	static constexpr uint32_t c_irradmap_width = 512;
	static constexpr uint32_t c_skydome_width = 512;

	explicit CloudyIrradiance(const char *name = nullptr) : Irradiance(name) {}
	explicit CloudyIrradiance(const Attrs &attrs) : CloudyIrradiance() { init(attrs); }

	void init(const Attrs &attrs) override;
	void update() override;
	void startInspector() override;

	void set(const Attrs &attrs) override
	{
		attrs.apply("interval", m_interval);
		attrs.apply("auto_direction", m_sun.is_auto_direction);
	}

	irradiance::UB_CLOUDY_IRRADIANCE ub_cloudy_irradiance;

	const SpuTexture &lightmap() const override { return m_lightmapCanvas.getBuffer("color0"); }
	const SpuTexture &irradmap() const override { return m_irradmapCanvas.getBuffer("color0"); }

protected:
	friend class CloudyIrradianceInspector;

	struct Sun {
		Vec3f emission;
		Vec3f ambient;
		Vec3f direction = ey();
		Vec3f axis = normalize(Vec3f(0.0, 1.0, -0.66));
		float hour = 5.0;
		float speed = 0.2;                 // 1sec = 1hour
		int32_t is_auto_direction = true;  // for tweakbar
		void update(UB_LIGHT &ub_light);
	} m_sun;

	GsCanvas m_skydomeCanvas;
	GsCanvas m_lightmapCanvas;
	GsCanvas m_irradmapCanvas;
	float m_speed = 0.02;
	uint32_t m_interval = 4;
	// uint32_t m_touchCount = 0;
};
}  // namespace spu::gs_canvas
