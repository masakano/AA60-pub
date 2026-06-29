//
// Shadowmap :
//
#include "poisson_filters.h"
#include <gsys/canvas/shadowmap.h>
#include <gsys/decorator/shadowmap.h>

namespace spu::gs_decorator {

Shadowmap::Shadowmap(GsPainter *painter, const spu::Attrs &attrs) : GsDecorator(painter, attrs)
{
	m_shadowmap = attrs.get<gs_canvas::Shadowmap *>("shadowmap", nullptr);
	if (!m_shadowmap) {
		aux_message(0, "no shadowmap specified (disabled)\n");
		// attrs.report("painter attrs");
	}
	painter->addUniforms(uniforms());
}

Attrs Shadowmap::uniforms() const
{
	Attrs unif_attrs = {
	        {"u_shadowmap",          &u_shadowmap         },
	        {"ub_shadowmap",         &ub_shadowmap        },
	        {"ub_shadowmap_pattern", &ub_shadowmap_pattern},
	};
	return unif_attrs;
}

void Shadowmap::doUse(uint32_t id)
{
	if (m_painter) {
		const auto *current = GsCanvas::getCurrent();
		auto &shader = m_painter->getShaders().at(current->getShaderType());
		auto &drawcall = m_painter->getDrawcalls().at(id);

		if (m_shadowmap && current != m_shadowmap && shader.uniformPtr("u_shadowmap")) {
			aux_error(
			        m_shadowmap->layerCount() >= int32_t(def_ub_shadowmap_max),
			        "too many layers (%d)\n", m_shadowmap->layerCount());

			ub_shadowmap = m_shadowmap->makeUniform();

			if (m_cache.blocker_pattern != m_shadowmap->blockerPattern()) {
				m_cache.blocker_pattern = m_shadowmap->blockerPattern();
				ub_shadowmap_pattern.blocker_count = load_poisson_coef(
				        m_cache.blocker_pattern, ub_shadowmap_pattern.blockers);
			}

			if (m_cache.receiver_pattern != m_shadowmap->receiverPattern()) {
				m_cache.receiver_pattern = m_shadowmap->receiverPattern();
				ub_shadowmap_pattern.receiver_count = load_poisson_coef(
				        m_cache.receiver_pattern, ub_shadowmap_pattern.receivers);
			}
			u_shadowmap = m_shadowmap->getBuffer("depth").id();
			ub_shadowmap.enable = true;
		}
		else {
			ub_shadowmap.enable = false;
		}

		// NEED FIX
		if (current == m_shadowmap) {
			auto &poly_offset = m_shadowmap->polyOffset();
			drawcall.flags.fill_offset = true;
			drawcall.poly_offset = poly_offset;
			// drawcall.flags.cull_face = 0; // disable cull face
			// drawcall.flags.ccw = !drawcall.flags.ccw;  // invert clockwise
		}
	}
	GsDecorator::doUse(id);
}
}  // namespace spu::gs_decorator
