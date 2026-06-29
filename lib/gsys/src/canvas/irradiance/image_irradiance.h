//
// ImageIrradiance :
//
#pragma once
#include <gsys/canvas/lightmap.h>

namespace spu::gs_canvas {

class ImageIrradiance : public Irradiance {
public:
	static constexpr float c_lightmap_distance = 32768.0;  // ad-hoc

	explicit ImageIrradiance(const char *name = nullptr) : Irradiance(name) {}
	explicit ImageIrradiance(const Attrs &attrs) : ImageIrradiance() { init(attrs); }
	~ImageIrradiance() = default;

	const SpuTexture &lightmap() const override
	{
		return m_lightmap.id() ? m_lightmap : GsDrawcall::getDefault()->lightmap;
	}
	const SpuTexture &irradmap() const override { return m_irradmap; }

	void init(const Attrs &attrs) override;
	void set(const Attrs &attrs) override;
	void update() override;

private:
	static constexpr float c_irradmap_sample_step = 0.01;  // ad-hoc
	static constexpr uint32_t c_irradmap_iformat = GL_RGBA32F;

	uint32_t m_cachedLightmapId = 0;
	SpuTexture m_lightmap;
	SpuTexture m_irradmap;
	uint32_t m_width = 1024;
	std::vector<float> m_powers = {1.0};

	void correctExposure(uint32_t texture_id, int32_t layer);
	std::string makeCachePath(const char *signature) const;
	GsCanvas *cubemapToSphere();
	GsCanvas *gaussianFilter(float variance, float footstep);

	bool load(const std::filesystem::path &path);
	void save(const std::filesystem::path &path) const;

	size_t selfSerialize(uint8_t *heap, bool is_dry) const;
	size_t selfDeserialize(uint8_t *heap);

	/*
	SpuTexture &getLightmap()
	{
	        return m_lightmap.id() ? m_lightmap : GsDrawcall::getDefault()->lightmap;
	}
	*/
};
}  // namespace spu::gs_canvas
