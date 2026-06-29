//
// Shadowmap :
//
#include "shadowmap_inspector.h"
#include <gsys/node.h>
#include <gsys/painter.h>

namespace spu::gs_canvas {

namespace {
struct ProbDesc {
	bool is_vsm;
	const char *def_shadowmap_probability_path;
};

std::map<hash32_t, ProbDesc> c_prob_descs = {
        {"none",   {false, "<decorator/shadowmap/shadowmap_probability_none.us>"}  },
        {"simple", {false, "<decorator/shadowmap/shadowmap_probability_simple.us>"}},
        {"vsm",    {true, "<decorator/shadowmap/shadowmap_probability_vsm.us>"}    },
        {"smooth", {false, "<decorator/shadowmap/shadowmap_probability_smooth.us>"}},
        {"pcf",    {false, "<decorator/shadowmap/shadowmap_probability_pcf.us>"}   },
};

}  // namespace

Shadowmap::~Shadowmap() { delete m_gaussCanvas; }

void Shadowmap::init(const Attrs &attrs)
{
	Rectf viewport = Rectf(0, 0, 512, 512);

	attrs.peek("vsm", "deprecated");
	attrs.peek("def_shadowmap_probability_path", "deprecated");

	auto mode = attrs.get("mode", "simple"_h32);

	m_layerCount = attrs.get("depth.depth", 1);
	aux_error(m_layerCount > 8, "too many layers\n");
	viewport = attrs.get<vec4f_t>("viewport0", viewport);

	auto &prob_desc = c_prob_descs[mode];

	// depth
	{
		Attrs init_attrs = {
		        {"shader_type",       hash32_t("depth")    },
		        {"viewport0",         viewport             },
		        {"depth.target",      GL_TEXTURE_2D_ARRAY  },
		        {"depth.iformat",     GL_DEPTH_COMPONENT32F},
		        {"depth.border",      1.0                  },
		        {"depth.wrap_s",      GL_CLAMP_TO_BORDER   },
		        {"depth.wrap_t",      GL_CLAMP_TO_BORDER   },
		        {"depth.min_filter",  GL_LINEAR            },
		        {"depth.mag_filter",  GL_LINEAR            },
		        {"depth.max_level",   0                    },
		        {"depth.auto_mipmap", 0                    },
		};

		init_attrs += attrs.select("depth.", true);
		GsCanvas::init(init_attrs);
		set("layer", 0);
	}

	// gauss
	if (prob_desc.is_vsm) {
		auto wrap_s = attrs.get("depth.wrap_s", GL_CLAMP_TO_BORDER);
		auto wrap_t = attrs.get("depth.wrap_t", GL_CLAMP_TO_BORDER);
		auto first_path = "canvas/shadowmap/vsm_first_gauss.us";

		Attrs init_attrs = {
		        {"viewport0",          viewport           },
                        {"color0.target",      GL_TEXTURE_2D_ARRAY},
		        {"color0.iformat",     GL_RG32F           },
                        {"color0.depth",       m_layerCount       },
		        {"color0.wrap_s",      wrap_s             },
                        {"color0.wrap_t",      wrap_t             },
		        {"color0.border",      1.0                },
                        {"color0.min_filter",  GL_LINEAR          },
		        {"color0.mag_filter",  GL_LINEAR          },
                        {"color0.max_level",   0                  },
		        {"color0.auto_mipmap", 0                  },
                        {"path.first",         first_path         },
		};
		init_attrs += attrs.select("color0.", true);
		m_gaussCanvas = new gs_canvas::Gauss2D(init_attrs);
		m_gaussCanvas->set("layer", 0);
	}
	m_shadowComposition.getWorldviews().resize(m_layerCount);
	m_shadowComposition.getViewscreen() = Mat4f();

	auto &shader_attrs = getShaderAttrs();
	shader_attrs = {
	        {"def_shadowmap_probability_path", prob_desc.def_shadowmap_probability_path},
	        {"def_shadowmap_path",             "<decorator/shadowmap/shadowmap_mul.us>"},
	};
	shader_attrs += attrs.select("def_", true);  // keep prefix
	shader_attrs.preserve();

	setProperty(e_lazy, 1);
}

const SpuTexture &Shadowmap::getBuffer(const hash32_t &slot) const
{
	aux_error(slot != hash32_t("depth"), "depth buffer only");
	return m_gaussCanvas ? m_gaussCanvas->getBuffer(hash32_t("color0")) :
	                       GsCanvas::getBuffer(hash32_t("depth"));
}

SpuTexture &Shadowmap::getBuffer(const hash32_t &slot)
{
	return const_cast<SpuTexture &>(const_cast<const Shadowmap *>(this)->getBuffer(slot));
}

UB_SHADOWMAP Shadowmap::makeUniform(const GsCanvas *current) const
{
	if (current == nullptr) {
		current = getCurrent();
	}

	UB_SHADOWMAP ub_shadowmap;
	auto worldview = getProperty(e_lazy) ? current->worldview(0) : Mat4f();
	auto &ls = current->ub_light.sources[0];

	ub_shadowmap.light_position = worldview.ortho3(ls.position);
	ub_shadowmap.light_type = ls.type;
	ub_shadowmap.light_radius = ls.radius;
	ub_shadowmap.enable = 1;
	ub_shadowmap.layer_count = m_layerCount;
	ub_shadowmap.viewport_mask = 0;

	auto viewworld = worldview.inverse();
	for (auto layer = 0; layer < m_layerCount; layer++) {
		ub_shadowmap.viewport_mask |= (1 << layer);
		ub_shadowmap.view_to_shadowtexc[layer]
		        = Mat4f::screentexc() * m_shadowComposition.worldscreen(layer) * viewworld;
		ub_shadowmap.worldview = worldview;
	}
	return ub_shadowmap;
}

void Shadowmap::setup()
{
	if (!relatedNodes().empty()) {
		m_drawfunc.points.clear();
		for (auto &node: relatedNodes()) {
			vector_cat(m_drawfunc.points, node->points());
		}
		m_drawfunc.func = [&] {
			for (auto &node: relatedNodes()) {
				node->render();
			}
		};
	}
	takeover();  // need fix
	if (getProperty(e_lazy)) {
		m_cameraComposition = Composition(*this);
		m_cameraComposition.adjustDepth(0, m_drawfunc.points);
	}
	else {
		m_cameraComposition = Composition();
	}
}

Mat4f Shadowmap::retriveShadowFrustum()
{
	auto &light_source = ub_light.sources[0];
	auto type
	        = light_source.type == e_ub_light_point ? ShadowFrustumf::e_point : ShadowFrustumf::e_parallel;

	ShadowFrustumf shadow_frustum(light_source.direction, light_source.position, type);

	auto worldshadow = shadow_frustum.capture(m_drawfunc.points);
	auto new_worldshadow = shadow_frustum.prune(m_cameraComposition.worldscreen(0), worldshadow);
	return new_worldshadow;
}

void Shadowmap::postFilter()
{
	if (m_gaussCanvas) {
		m_gaussCanvas->u_color0 = GsCanvas::getBuffer("depth").id();
		m_gaussCanvas->u_variance = m_variance;
		m_gaussCanvas->u_footstep = Vec4f(m_footstep, 0, 0, m_footstep);

		for (auto layer = 0; layer < m_layerCount; layer++) {
			m_gaussCanvas->ub_connect.sources[0].layer = layer;
			m_gaussCanvas->set("layer", layer);
			m_gaussCanvas->begin();
			m_gaussCanvas->render();
			m_gaussCanvas->end();
		}
	}
}

void Shadowmap::update()
{
	setup();
	if (m_drawfunc.func == nullptr) return;

	uint32_t target_layer;
	get("layer", &target_layer);
	auto source_layer = target_layer == uint32_t(-1) ? 0u : target_layer;

	m_shadowComposition.getWorldviews().at(source_layer) = retriveShadowFrustum();
	m_shadowComposition.getViewscreen() = Mat4f();

	Composition::takeover(m_shadowComposition, source_layer, 0);
	begin();
	clear();
	m_drawfunc.func();
	end();
	postFilter();
	GsCanvas::update();
}

void Shadowmap::set(const Attrs &attrs)
{
	attrs.peek("poly_offset", "use 'poly_offset.factor/poly_offset.units' instead");
	attrs.peek("u_variance", "use 'capture_variance' instead");
	attrs.peek("u_footstep", "use 'capture_footstep' instead");

	attrs.apply("poly_offset.factor", m_polyOffset.factor);
	attrs.apply("poly_offset.units", m_polyOffset.units);
	attrs.apply("blocker_pattern", m_blockerPattern);
	attrs.apply("receiver_pattern", m_receiverPattern);
	attrs.apply("capture_variance", m_variance);
	attrs.apply("capture_footstep", m_footstep);

	GsCanvas::set(attrs);
}

void Shadowmap::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new ShadowmapInspector(this);
	}
}

}  // namespace spu::gs_canvas
