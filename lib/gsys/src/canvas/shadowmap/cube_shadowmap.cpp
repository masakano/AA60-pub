//
// CubeShadowmap :
//
#include "shadowmap_inspector.h"
#include <gsys/canvas/gauss.h>
#include <gsys/canvas/shadowmap.h>

namespace spu::gs_canvas {

void CubeShadowmap::init(const Attrs &attrs)
{
	Attrs def_attrs = {
	        {"depth.depth",        6	                               },
	        {"depth.wrap_s",       GL_CLAMP_TO_EDGE                        },
	        {"depth.wrap_t",       GL_CLAMP_TO_EDGE                        },
	        {"def_shadowmap_path", "<decorator/shadowmap/shadowmap_sel.us>"},
	};
	Shadowmap::init(def_attrs + attrs);
}

void CubeShadowmap::update()
{
	setup();
	if (m_drawfunc.func == nullptr) return;

	auto &ls = ub_light.sources[0];
	ls.type = e_ub_light_point;

	assert(m_layerCount == 6);

	auto op = [&](const Vec3f &p) { return distance<Vec3f>(ls.position, p); };
	auto it = vector_max(m_drawfunc.points, op);
	auto max_d = op(*it);

	const auto c_near_far_ratio = 0.001f;
	m_shadowComposition = Composition::cubeComposition(ls.position, max_d * c_near_far_ratio, max_d);

	for (auto layer = 0; layer < m_layerCount; layer++) {
		Composition::takeover(m_shadowComposition, layer, 0);
		set("layer", layer);
		begin();
		clear();
		m_drawfunc.func();
		end();
	}
	postFilter();
	GsCanvas::update();
}

}  // namespace spu::gs_canvas
