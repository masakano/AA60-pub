//
// DefaultInspector :
//
#include "default_inspector.h"
#include <cstdio>
#include "gsys/drawcall.h"

namespace spu::gs_node::gui {

DefaultInspector::DefaultInspector(GsCanvas *canvas, const char *conf_file) : m_canvas(canvas)
{
	File file(conf_file, "r");

	std::vector<gs_node::gui::Menu::Item> materialmap_labels;
	std::vector<gs_node::gui::Menu::Item> lightmap_labels;

	const auto tags = Attrs::inspect(file);

	// default
	{
		GsDrawcall drawcall = *GsDrawcall::getDefault();
		m_materialmaps.emplace_back(drawcall);
		materialmap_labels.emplace_back("(default)", 0);

		m_lightmaps.emplace_back(drawcall.lightmap);
		lightmap_labels.emplace_back("(default)", 0);
	}

	for (auto &tag: tags) {
		Attrs attrs;
		GsDrawcall drawcall;

		attrs.load(file, tag.c_str());
		drawcall.set(attrs);

		if (!isDefaultTexture(drawcall.albedomap.id())) {
			m_materialmaps.emplace_back(drawcall);
			materialmap_labels.emplace_back(tag.c_str(), materialmap_labels.size());
		}
		else if (!isDefaultTexture(drawcall.lightmap.id())) {
			m_lightmaps.emplace_back(drawcall.lightmap);
			lightmap_labels.emplace_back(tag.c_str(), lightmap_labels.size());
		}
		else {
			attrs.report(tag.c_str());
			aux_message(0, "path not found\n");
		}
	}
	aux_error(m_materialmaps.size() == 1, "no material path found\n");
	aux_error(m_lightmaps.size() == 1, "no lightmap path found\n");

	base_t::setName(base_t::padstr("default light", 24));

	// IBL
	{
		base_t::addMenus("materialmap", {materialmap_labels}, {&m_materialmapIndex});
		base_t::addMenus("lightmap", {lightmap_labels}, {&m_lightmapIndex});
	}

	// direct light
	{
		auto &ub_light = m_canvas->ub_light;
		auto &ls = ub_light.sources[0];

		base_t::addStdButton("y-up ", &m_isYup);
		base_t::addStdButton("point light ", &ls.type);
		base_t::addStdSlider("decay", 0.0, 1.0, &ls.decay);
		base_t::addStdSlider("exponent", 0.0, 16.0, &ls.exponent);
		base_t::addStdSlider("radius", 0.0, 0.2, &ls.radius, 2.0);
		base_t::addStdSlider("distance", 2.0, 32.0, &m_distance);
		base_t::addStdSlider("azimuth", -pi(), pi(), &m_azimuth);
		base_t::addStdSlider("elevation", 0.0, pi(), &m_elevation);
		base_t::addStdSlider("emission", 0.0, 16.0, &m_emission, 2.0);
		base_t::addStdSlider("ambient", 0.0, 4.0, &m_ambient, 2.0);
	}
	base_t::bake();
}

void DefaultInspector::update()
{
	auto touch_count = base_t::lastUpdateCount();
	auto is_focus = base_t::isFocus();

	base_t::update();
	if (is_focus) {
		if (touch_count != base_t::lastUpdateCount()) {
			coreUpdate();
		}
		else {
			reload();
		}
	}
}

void DefaultInspector::reload()
{
	auto &ub_light = m_canvas->ub_light;
	auto &ls = ub_light.sources[0];

	auto direction = normalize<Vec3f>(ls.direction);

	m_elevation = std::max(0.0f, asinf(direction.y));

	m_emission = tonemap(Vec3f(ls.emission));
	m_ambient = tonemap(Vec3f(ub_light.ambient));
}

void DefaultInspector::coreUpdate()
{
	// IBL
	{
		auto *default_drawcall = GsDrawcall::getDefault();
		auto &materialmap = m_materialmaps.at(m_materialmapIndex);
		auto &lightmap = m_lightmaps.at(m_lightmapIndex);

		default_drawcall->albedomap = materialmap.albedomap;
		default_drawcall->armmap = materialmap.armmap;
		default_drawcall->normalmap = materialmap.normalmap;
		default_drawcall->heightmap = materialmap.heightmap;
		default_drawcall->lightmap = lightmap;
	}

	// direct ligh
	{
		auto &ub_light = m_canvas->ub_light;
		auto &ls = ub_light.sources[0];

		auto position = Vec3f(ls.position);
		auto direction = normalize<Vec3f>(ls.direction);

		auto q0 = Quatf(m_elevation, -ey());  // rot -y
		auto q1 = Quatf(m_azimuth, -ez());    // rot z
		auto q2 = Quatf::from_target(up(), ez());

		direction = q2 * q1 * q0 * ex();
		position = direction * m_distance;

		ls.position = position;
		ls.direction = direction;
		ls.emission = retonemap(m_emission);
		ub_light.ambient = retonemap(m_ambient);
	}
}
}  // namespace spu::gs_node::gui
