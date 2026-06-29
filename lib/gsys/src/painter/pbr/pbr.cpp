//
// PBR :
//
#include "pbr_inspector.h"
#include <gsys/canvas/shadowmap.h>
#include <gsys/decorator/instance.h>
#include <gsys/decorator/material.h>
#include <gsys/decorator/shadowmap.h>

namespace spu::gs_painter {

void PBR::init(const Attrs &attrs)
{
	Attrs def_attrs = {
	        {"path.radiance", "painter/pbr/radiance.us"},
	        {"path.depth",    "painter/pbr/depth.us"   },
	        {"a.a_position",  3                        },
	        {"a.a_normal",    3                        },
	        {"a.a_texcoord",  2                        },
	};
	def_pbr_debug = attrs.get("def_pbr_debug", false);
	GsPainter::init(def_attrs + attrs);
	getDecorators().push_back(new gs_decorator::Instance(this, attrs));
	getDecorators().push_back(new gs_decorator::Material(this, attrs));
	getDecorators().push_back(new gs_decorator::Shadowmap(this, attrs));
}

void PBR::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new PBRInspector(this);
	}
}
}  // namespace spu::gs_painter
