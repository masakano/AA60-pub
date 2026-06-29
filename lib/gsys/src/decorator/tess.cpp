//
// Tess :
//
#include <gsys/decorator/tess.h>

namespace spu::gs_decorator {

Tess::Tess(GsPainter *painter, const Attrs &attrs)
        : GsDecorator(painter, attrs), m_doPatch(attrs.get("do_patch", true))
{
	if (m_doPatch) {
		painter->addUniforms(uniforms());
		auto patch_vertices = attrs.get("patch_vertices", 3);
		painter->set("patch_vertices", patch_vertices);
	}
}

Attrs Tess::uniforms() const
{
	Attrs unif_attrs = {
	        {"u_tess_min_len", &u_tess_min_len},
	        {"u_tess_max",     &u_tess_max    },
	};
	return unif_attrs;
}

void Tess::doUse(uint32_t id)
{
	if (m_doPatch) {
		auto &drawcall = m_painter->getDrawcalls().at(id);
		for (auto &com: drawcall.coms) {
			com.mode = GL_PATCHES;
		}
	}
	GsDecorator::doUse(id);
}
}  // namespace spu::gs_decorator
