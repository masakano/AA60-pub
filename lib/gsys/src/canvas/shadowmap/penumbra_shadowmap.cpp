//
// PenumbraShadowmap :
//
#include <gsys/canvas/shadowmap.h>

namespace spu::gs_canvas {

void PenumbraShadowmap::setRadius(float radius)
{
	if (radius != m_radius) {
		m_radius = radius;
		m_jitters[0] = ezero();
		m_jitters[1] = Vec3f(m_radius, 0, 0);

		Quatf q = Quatf(radians(360.0f) / float(m_layerCount - 1), ez());
		for (auto layer = 2; layer < m_layerCount; layer++) {
			m_jitters[layer] = q * m_jitters[layer - 1];
		}
	}
}

void PenumbraShadowmap::init(const Attrs &attrs)
{
	const auto c_layer_count = 6;
	Attrs def_attrs = {
	        {"depth.depth",        c_layer_count                           },
	        {"def_shadowmap_path", "<decorator/shadowmap/shadowmap_add.us>"},
	};
	Shadowmap::init(def_attrs + attrs);
	m_jitters.resize(m_layerCount, ezero());
	setRadius(0.0);
}

void PenumbraShadowmap::update()
{
	setup();
	if (m_drawfunc.func == nullptr) return;

	auto &ls = ub_light.sources[0];
	auto ls_save = ls;
	auto shadow_viewworld = Mat4f::direction_matrix(ls.position, ls.direction);

	setRadius(ls.radius);
	for (auto layer = 0; layer < m_layerCount; layer++) {
		ls.position = Vec4f(ls_save.position + shadow_viewworld.ortho3(m_jitters[layer]), 1);
		set("layer", layer);
		m_shadowComposition.getWorldviews().at(layer) = retriveShadowFrustum();
		m_shadowComposition.getViewscreen() = Mat4f();

		Composition::takeover(m_shadowComposition, layer, 0);
		begin();
		clear();
		m_drawfunc.func();
		end();
	}
	ls = ls_save;
	postFilter();
	GsCanvas::update();
}
}  // namespace spu::gs_canvas
