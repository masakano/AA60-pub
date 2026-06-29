//
// Irradiance :
//
#pragma once
#include <gsys/canvas.h>
#include <gsys/drawcall.h>
#include <gsys/shaders/default/ub_lightmap.us>

namespace spu::gs_canvas {

class Irradiance : public GsCanvas {
public:
	explicit Irradiance(const char *name = nullptr) : GsCanvas(name) {}
	explicit Irradiance(const Attrs &attrs) : Irradiance() { init(attrs); }

	virtual const SpuTexture &lightmap() const = 0;
	virtual const SpuTexture &irradmap() const = 0;
};

class Lightmap : public GsCanvas {
public:
	explicit Lightmap(const char *name = nullptr) : GsCanvas(name) {}
	explicit Lightmap(const Attrs &attrs) : Lightmap() { init(attrs); }

	~Lightmap() { delete m_irradiance; }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	using GsObject::set;
	void update() override;
	void render() override;
	void startInspector() override;

	virtual Irradiance *irradiance() const { return m_irradiance; }

	UB_LIGHTMAP ub_lightmap;
	uint32_t u_irradmap = 0;
	uint32_t u_lightmap = 0;

protected:
	Mat4f m_worldlightmap;
	uint32_t m_touchCount = 0;
	uint32_t m_unupdateCount = 0;
	mutable Irradiance *m_irradiance = nullptr;
};

class ImageLightmap : public Lightmap {
public:
	explicit ImageLightmap(const char *name = nullptr) : Lightmap(name) {}
	explicit ImageLightmap(const Attrs &attrs) : ImageLightmap() { init(attrs); }

	void init(const Attrs &attrs) override;

private:
	bool doSync(bool is_nonblock) override;
};

class CloudyLightmap : public Lightmap {
public:
	explicit CloudyLightmap(const char *name = nullptr) : Lightmap(name) {}
	explicit CloudyLightmap(const Attrs &attrs) : CloudyLightmap() { init(attrs); }

	void init(const Attrs &attrs) override;
};
}  // namespace spu::gs_canvas
