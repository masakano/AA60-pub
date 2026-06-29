//
// Panel :
//
#include <gsys/node/gui/menu.h>
#include <gsys/node/gui/panel.h>
#include <gsys/node/gui/slider.h>

namespace spu::gs_node::gui {

Panel::Panel(const char *name) : Base(name)
{
	Base::init(Attrs());

	getColors()[e_title] = {0.30, 1.00, 1.00, 1.00};

	// background
	auto &graphic = newGraphic(e_bg);
	graphic.drawcall.flags.depth_test = false;
	graphic.drawcall.flags.blend = true;
	graphic.drawcall.ub_material.invisible = false;
	graphic.vertices.resize(3);

	// canvas
	m_canvas.init(Attrs());
	m_canvas.getRenderstate().flags.blend = true;
	// m_canvas.getIsSrgb() = true;  // experimental
	// m_canvas.set("srgb", true);
}

void Panel::autoArrange()
{
	auto modified_count = 0u;
	for (auto &node: getChildren()) {
		auto gui_node = dynamic_cast<Base *>(node);
		modified_count += gui_node->getWindowModifier().count();
	}

	if (modified_count == m_modifiedCount) {
		return;  // cache hit
	}
	m_modifiedCount = modified_count;

	auto max_column = name().length();
	const Base *left = nullptr;
	const Base *upper = nullptr;

	Range3f panel_range = headerRange();
	for (auto &node: getChildren()) {
		auto gui_node = dynamic_cast<Base *>(node);
		if (alignSide(gui_node, left, upper, max_column)) {
			left = gui_node;
		}
		else {
			upper = gui_node;
		}
		panel_range.expand(gui_node->getASubstance() * gui_node->windowRange());
	}
	getARange() = panel_range;
	arrangeBackgroundGraphic();
}

bool Panel::alignSide(Base *node, const Base *left, const Base *upper, int32_t max_column)
{
	auto &transform = node->getASubstance();
	transform.c[3] = {0, 0, 0, 1};  // reset translates

	auto range = transform * node->windowRange();
	if (left) {
		auto left_range = left->getASubstance() * left->windowRange();
		if (left_range.p1.x + range.span().x <= max_column) {  // right side
			transform.c[3].x += left_range.p1.x - range.p0.x;
			transform.c[3].y += left_range.p1.y - range.p1.y;
			return true;
		}
	}
	auto upper_range = upper ? upper->getASubstance() * upper->windowRange() : headerRange();
	transform.c[3].x += upper_range.p0.x - range.p0.x;
	transform.c[3].y += upper_range.p0.y - range.p1.y;
	return false;
}

void Panel::arrangeBackgroundGraphic()
{
	const auto c_matte = 0.25f;
	auto &graphic = getGraphic(e_bg);
	auto &b0 = graphic.vertices[0];
	auto &b1 = graphic.vertices[1];
	auto &b2 = graphic.vertices[2];
	auto &p0 = getARange().p0;
	auto &p1 = getARange().p1;
	auto color = getColors().at(e_background);
	auto side_color = min(color * 32.0f, eone<Vec4f>());
	auto title_color = getColors().at(e_title) * 0.1f;

	// back
	b0.p = {0, 0, 0};
	b0.s = {p0.x, p0.y, p1.x, p1.y};
	b0.c = color;

	// top
	b1.p = {0, 0, 0};
	b1.s = {p0.x, p1.y - c_charheight, p1.x, p1.y}, b1.c = title_color;

	// side
	b2.p = {0, 0, 0};
	b2.s = {p0.x, p0.y, p0.x + c_matte, p1.y};
	b2.c = side_color;
}

void Panel::adjustViewport(float scale)
{
	m_scale = scale;

	auto range = windowRange();
	// auto &c = m_canvas.getComposition();
	auto viewport_span = range.span() * c_charscale * m_scale;

	if (viewport_span.x != m_canvas.viewport(0).sx || viewport_span.y != m_canvas.viewport(0).sy) {
		m_canvas.getViewports().at(0).oy
		        = m_canvas.viewport(0).oy + m_canvas.viewport(0).sy - viewport_span.y;
		m_canvas.getViewports().at(0).sx = viewport_span.x;
		m_canvas.getViewports().at(0).sy = viewport_span.y;
		m_canvas.getWorldviews().front() = Mat4f();
		m_canvas.getViewscreen() = Mat4f(range, 0x00ff);  // x,y only
	}
}

void Panel::update()
{
	if (count() && getGesture()->curr().swap_count == count()) {
		return;  // do nothing
	}

	autoArrange();  // experimental
	m_canvas.begin();

	auto *gesture = getGesture();
	auto cursor = getCursor(getGesture()->curr().cursor);
	auto is_inside = headerRange().inside(cursor);

	// zoom
	if (is_inside) {
		if (gesture->wheel() > 0) {
			m_scale *= 1.1;
		}
		else if (gesture->wheel() < 0) {
			m_scale /= 1.1;
		}
	}
	// pick
	if (is_inside && !gesture->prev().mouse_L && gesture->curr().mouse_L) {
		m_anchor = m_canvas.viewport(0);
		m_isPicking = true;
	}
	// drop
	if (!gesture->curr().mouse_L) {
		m_isPicking = false;
	}

	// drag
	if (m_isPicking) {
		const float c_glue = 16;
		auto delta = Vec2f(gesture->curr().cursor) - Vec2f(gesture->anchorL());
		auto &viewport = m_canvas.getViewports().at(0);

		auto ox = m_anchor.ox + delta.x;
		auto oy = m_anchor.oy + delta.y;

		auto wx = gesture->curr().winsize[0];
		auto wy = gesture->curr().winsize[1];

		viewport.ox = clamp<float>(ox, -viewport.sx + c_glue, wx - c_glue);
		viewport.oy = clamp<float>(oy, -viewport.sy + c_glue, wy - viewport.sy);
	}

	Base::update();
	m_canvas.end();
}

void Panel::doDebugRender()
{
	adjustViewport(m_scale);
	m_canvas.begin();
	Base::doDebugRender();
	m_canvas.end();
}

void Panel::doRender()
{
	auto is_draw = false;
	for (auto &child: getChildren()) {
		if (child->getProperty(e_render)) {
			is_draw = true;
			break;
		}
	}
	if (is_draw) {
		adjustViewport(m_scale);
		m_canvas.begin();
		Base::doRender();
		m_canvas.end();
	}
}
}  // namespace spu::gs_node::gui
