//
// CascadeShadowmap :
//
#include <gsys/canvas/shadowmap.h>

namespace spu::gs_canvas {

void CascadeShadowmap::init(const Attrs &attrs)
{
	def_use_multi_viewport = attrs.get("def_use_multi_viewport", 0);

	const char *def_shadowmap_path = def_use_multi_viewport ? "<decorator/shadowmap/shadowmap_sel_mv.us>" :
	                                                          "<decorator/shadowmap/shadowmap_sel.us>";
	// parent
	Attrs aux_attrs = {
	        {"depth.depth",        4                 },
	        {"depth.wrap_s",       GL_CLAMP_TO_EDGE  },
	        {"depth.wrap_t",       GL_CLAMP_TO_EDGE  },
	        {"def_shadowmap_path", def_shadowmap_path},
	};
	Shadowmap::init(attrs + aux_attrs);

	m_screenfrag = Mat4f::texcfrag(viewport(0)) * Mat4f::screentexc();
	m_shifts.resize(m_layerCount);

	if (def_use_multi_viewport) {
		Vec2f viewport_dims;
		glGetFloatv(GL_MAX_VIEWPORT_DIMS, viewport_dims.f);
		m_minSpan.x = 2.0f * viewport(0).sx / viewport_dims.x;
		m_minSpan.y = 2.0f * viewport(0).sy / viewport_dims.y;
	}
}

float CascadeShadowmap::splitDepth(float full_near, float full_far, float split_rate) const
{
	auto log_far = full_near * powf(full_far / full_near, split_rate);
	auto linear_far = full_near + split_rate * (full_far - full_near);
	return lerp(linear_far, log_far, m_strength);
}

void CascadeShadowmap::splitShadowViewscreen(const Mat4f &shadow_worldscreen)
{
	auto camera_viewscreen = m_cameraComposition.viewscreen();
	auto camera_worldview = m_cameraComposition.worldview(0);
	auto screen_to_shadow_screen = shadow_worldscreen * m_cameraComposition.worldscreen(0).inverse();

	float full_near;
	float full_far;

	camera_viewscreen.get_projection(nullptr, nullptr, &full_near, &full_far);

	auto shadow_points_view = camera_worldview.ortho3(Mat4f(shadow_worldscreen).points());
	full_far = -huge();
	for (auto &p: shadow_points_view) {
		full_far = max(full_far, -p.z);
	}
	for (auto layer = 0; layer < m_layerCount; layer++) {
		auto near = splitDepth(full_near, full_far, float(layer + 0) / m_layerCount);
		auto far = splitDepth(full_near, full_far, float(layer + 1) / m_layerCount);

		auto near_screen = camera_viewscreen.pers3(Vec3f(0, 0, -near)).z;
		auto far_screen = camera_viewscreen.pers3(Vec3f(0, 0, -far)).z;

		const std::vector<Vec3f> points_screen = {
		        {-1.0f, -1.0f, near_screen},
                        {+1.0f, -1.0f, near_screen},
                        {-1.0f, +1.0f, near_screen},
		        {+1.0f, +1.0f, near_screen},
                        {-1.0f, -1.0f, far_screen },
                        {+1.0f, -1.0f, far_screen },
		        {-1.0f, +1.0f, far_screen },
                        {+1.0f, +1.0f, far_screen },
		};
		auto range_shadow_screen
		        = Range3f(screen_to_shadow_screen.pers3(points_screen));  // use points, not range

		range_shadow_screen.p0 = max(range_shadow_screen.p0, Vec3f(-1));
		range_shadow_screen.p1 = min(range_shadow_screen.p1, Vec3f(+1));

		auto span_shadow_screen = range_shadow_screen.span();
		if (span_shadow_screen.x < m_minSpan.x || span_shadow_screen.y < m_minSpan.y) {
			span_shadow_screen = max(span_shadow_screen, m_minSpan);
			range_shadow_screen.p1 = range_shadow_screen.p0 + span_shadow_screen;
			m_strength *= 0.99f;
		}

		if (m_strength < 0.90f) {
			m_strength /= 0.999f;
		}
		m_strength = std::min(m_strength, 0.99f);

		// xy only
		m_shifts[layer] = Mat4f(range_shadow_screen, 0x0ff);
		m_shadowComposition.getWorldviews().at(layer) = m_shifts[layer] * shadow_worldscreen;
		m_shadowComposition.getViewscreen() = Mat4f();
	}
}

void CascadeShadowmap::standardCapture(const std::function<void()> &drawfunc)
{
	// auto usec = get_microsec();
	for (auto layer = 0; layer < m_layerCount; layer++) {
		set("layer", layer);
		Composition::takeover(m_shadowComposition, layer, 0);
		begin();
		clear();
		drawfunc();
		end();
	}
	// spu_printf(0, "shadowmap: %ld usec\n", get_microsec() - usec);
}

void CascadeShadowmap::multiViewportCapture(
        const Mat4f &shadow_worldscreen, const std::function<void()> &drawfunc)
{
	Attrs attrs;

	constexpr hash32_t c_viewport_names[] = {
	        "viewport0", "viewport1", "viewport2", "viewport3",
	        "viewport4", "viewport5", "viewport6", "viewport7",
	};

	for (auto layer = 0; layer < m_layerCount; layer++) {
		const auto shifted_screenfrag = m_screenfrag * m_shifts[layer];
		const auto p0 = shifted_screenfrag.ortho3(Vec3f(-1, -1, 0));
		const auto p1 = shifted_screenfrag.ortho3(Vec3f(+1, +1, 0));
		const auto viewport = Rectf(p0.x, p0.y, p1.x - p0.x, p1.y - p0.y);
		attrs.emplace_back(c_viewport_names[layer], viewport);
	}
	attrs.emplace_back("layer", -1);  // use all layer
	set(attrs);

	getWorldviews().front() = shadow_worldscreen;
	getViewscreen() = Mat4f();

	begin();
	clear();
	drawfunc();
	end();
}

void CascadeShadowmap::update()
{
	setup();
	if (m_drawfunc.func == nullptr) return;

	auto shadow_worldscreen = retriveShadowFrustum();
	splitShadowViewscreen(shadow_worldscreen);

	if (def_use_multi_viewport) {
		multiViewportCapture(shadow_worldscreen, m_drawfunc.func);
	}
	else {
		standardCapture(m_drawfunc.func);
	}
	postFilter();
	GsCanvas::update();
}
}  // namespace spu::gs_canvas
