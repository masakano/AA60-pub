//
// DefaultInspector :
//
#pragma once
#include <gsys/drawcall.h>
#include <gsys/node/gui/tweakbar.h>
#include "spu++/spu_texture.h"

namespace spu::gs_node::gui {

class DefaultInspector : public Tweakbar {
public:
	using base_t = gui::Tweakbar;
	DefaultInspector(GsCanvas *canvas, const char *conf_file = "default_inspector.conf");
	void update() override;

private:
	struct Materialmap {
		SpuTexture albedomap;
		SpuTexture armmap;
		SpuTexture normalmap;
		SpuTexture heightmap;

		Materialmap() = default;
		Materialmap(const GsDrawcall &d)
		{
			albedomap = d.albedomap;
			armmap = d.armmap;
			normalmap = d.normalmap;
			heightmap = d.heightmap;
		}
	};

	std::vector<Materialmap> m_materialmaps;
	std::vector<SpuTexture> m_lightmaps;

	int32_t m_materialmapIndex = 0;
	int32_t m_lightmapIndex = 0;

	GsCanvas *m_canvas = nullptr;
	int32_t m_isYup = true;
	float m_distance = 16.0;
	float m_azimuth = radians(0.0);
	float m_elevation = radians(45.0);
	float m_emission = 1.0;
	float m_ambient = 0.1;

	void coreUpdate();
	void reload();
	Vec3f up() const { return m_isYup ? ey() : ez(); }
	float tonemap(const Vec3f &value) { return 1.0f - expf(-(value.x + value.y + value.z) / 3); }
	float retonemap(float value) { return -logf(std::clamp(1.0f - value, 0.001f, 1.0f)); }
};
}  // namespace spu::gs_node::gui
