//
// Toon :
//
#include "toon_inspector.h"
#include <gsys/decorator/instance.h>
// #include <gsys/painter/toon.h>

namespace spu::gs_painter {
void Toon::init(const Attrs &attrs)
{
	Attrs def_attrs = {
	        {"path.radiance",     "painter/toon/radiance.us"},
	        {"path.depth",        "painter/toon/depth.us"   },
	        {"path.sub_radiance", "painter/toon/edge.us"    },
	};
	PBR::init(def_attrs + attrs);
	Attrs unif_attrs = {
	        {"u_edge_color",     &u_edge_color    },
                {"u_edge_width",     &u_edge_width    },
	        {"u_edge_mix_rate",  &u_edge_mix_rate },
                {"u_toon_ao",        &u_toon_ao       },
	        {"u_toon_threshold", &u_toon_threshold},
	};
	addUniforms(unif_attrs);
}

void Toon::doUse(uint32_t id)
{
	if (getShaderType() == e_sub_radiance) {
		auto &drawcall = getDrawcalls().at(id);
		drawcall.flags.blend = false;
		drawcall.flags.ccw = ~drawcall.flags.ccw;
		drawcall.ub_material.invisible = !drawcall.flags.cull_face;  // skip if true
	}
	GsPainter::doUse(id);
}

void Toon::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new ToonInspector(this);
	}
}

}  // namespace spu::gs_painter
