//
// DuiNode :
//
#include <spu++/dds/dds.h>
#include <gsys/node/dui.h>
#include <gsys/painter/ft_text.h>

namespace {
#include "assetdata.h"

const char *s_vert
        = "#version 430							\n"
          "uniform mat4 u_nodescreen;					\n"
          "in vec2 a_position;						\n"
          "out vec2 f_texcoord;						\n"
          "void main()							\n"
          "{								\n"
          "    gl_Position = u_nodescreen * vec4(a_position, 0, 1);	\n"
          "    f_texcoord = a_position * vec2(+0.5, -0.5)  + 0.5 ;	\n"
          "}								\n";

const char *s_frag
        = "#version 430							\n"
          "in vec2 f_texcoord;						\n"
          "uniform sampler2D u_texture;					\n"
          "uniform vec4 u_color;					\n"
          "uniform vec2 u_border;					\n"
          "uniform uint u_debug;					\n"
          "out vec4 final_color;					\n"
          "float border_step(float x, float b) {			\n"
          "	if (x < b) return x / (b * 2.0);			\n"
          "	if (x < 1.0 - b) return 0.5;				\n"
          "	return 1.0 - (1.0 - x) / (b * 2.0);			\n"
          "}								\n"
          "void main() {						\n"
          "	float u = border_step(f_texcoord.x, u_border.x);	\n"
          "	float v = border_step(f_texcoord.y, u_border.y);	\n"
          "	final_color = texture(u_texture, vec2(u, v)) * u_color; \n"
          "	if (u_debug != 0) final_color = vec4(1); \n"
          "}								\n";
}  // namespace

namespace spu::gs_node::dui {

void DuiNode::startup()
{
	// shader
	{
		Attrs shader_attrs = {
		        {"vert", s_vert},
		        {"frag", s_frag},
		};
		ms_shader.init(shader_attrs);

		Attrs unif_attrs = {
		        {"u_nodescreen", &u_nodescreen},
                        {"u_color",      &u_color     },
                        {"u_debug",      &u_debug     },
		        {"u_texture",    &u_texture   },
                        {"u_border",     &u_border    },
		};
		ms_shader.addUniforms(unif_attrs);
	}

	// array
	{
		Attrs array_attrs = {
		        {"shader_id",    ms_shader.id()},
		        {"a.a_position", 2             },
		};
		ms_array.init(array_attrs);
		std::vector<float> vertices = {
		        -1, -1, +1, -1, +1, +1, -1, +1,
		};
		ms_array.send(vertices.data(), vertices.size() / 2);
	}

	// gesture
	{
		ms_gesture.init(nullptr);
	}

	// file system
	{
		for (auto i = 0u; i < c_assetCount; i++) {
			File::embed(c_assetNames[i], c_assetDatas[i], c_assetSizes[i]);
		}
	}

	// temporary
	{
		gs_painter::FTText::allocGlobalAtlas(Vec4i(512, 512, 1, 1));
	}
}

void DuiNode::shutdown()
{
	gs_painter::FTText::freeGlobalAtlas();
	for (const auto &object: GsObject::aliveObjects()) {
		auto *element = dynamic_cast<DuiNode *>(object);
		if (element) {
			aux_printf(
			        "dui::shutdown: %p %s \"%s\"\n", element, element->typeName().c_str(),
			        element->name().c_str());
			delete element;
		}
	}

	ms_shader.dispose();
	ms_array.dispose();
}

void DuiNode::init(const Attrs &attrs)
{
	attrs.peek("range", "use 'span' instead");

	// set via override
	setColor(attrs.get<vec4f_t>("color", eone<Vec4f>()));
	setBorder(attrs.get<vec4f_t>("border", ezero<Vec4f>()));
	GsObject::setProperty(e_render, attrs.get<int32_t>("is_visible", 1) != 0);

	auto *texture_path = attrs.get("texture_path", "");
	auto span = Vec2f(0);

	if (*texture_path) {
		int32_t width;
		int32_t height;
		if (std::string(texture_path).find(".dds") == std::string::npos) {
			m_texture.init(texture_path, Attrs());
			m_texture.get("width", &width);
			m_texture.get("height", &height);
		}
		else {
			dds::Image image(texture_path, false);
			width = image.width();
			height = image.height();

			Attrs tex_attrs = {
			        {"min_filter", GL_LINEAR       },
			        {"mag_filter", GL_LINEAR       },
			        {"wrap_s",     GL_CLAMP_TO_EDGE},
			        {"wrap_t",     GL_CLAMP_TO_EDGE},
			};
			m_texture.reset(image.upload(tex_attrs));
		}
		span = Vec2f(width, height);
	}
	span = attrs.get<vec4f_t>("span", span);
	getARange() = Range2f(Vec2f(0), span);
}

void DuiNode::add(DuiNode *element, bool is_expand)
{
	assert(element != this);
	if (is_expand) {
		auto &range = getARange();
		range.expand(element->getARange());
	}
	addChildren({element});
}

void DuiNode::squash()
{
	auto &children = getChildren();
	auto &range = getARange();

	auto span = getARange().span().y;
	for (auto &child: children) {
		auto *element = dynamic_cast<DuiNode *>(child);
		span = std::max(span, element->getARange().span().y);
	}
	range.p0.y = range.p1.y - span;

	auto to = range.center().y;
	for (auto &child: children) {
		auto *element = dynamic_cast<DuiNode *>(child);
		auto &element_range = element->getARange();
		auto from = element_range.center().y;
		element_range = element_range + Vec2f(0, to - from);
	}
}

void DuiNode::pile(DuiNode *element, const DuiNode *upper, float indent)
{
	if (upper == nullptr) upper = this;

	auto &element_range = element->getARange();
	auto &upper_range = upper->getARange();
	auto from = upperLeftOf(element_range);
	auto to = lowerLeftOf(upper_range) + Vec2f(indent, 0);
	element->move(to - from);
	add(element);
}

void DuiNode::move(const Vec2f &delta)
{
	for (auto &child: getChildren()) {
		auto *element = dynamic_cast<DuiNode *>(child);
		element->move(delta);
	}
	getARange() = getARange() + delta;
}

void DuiNode::setColor(const Vec4f &color, uint32_t mask) { m_color = select(mask, color, m_color); }

// Note: update children
void DuiNode::update()
{
	// for (auto &element: selectChildren<DuiNode *>()) {
	for (auto &element: select_objects<DuiNode *>(getChildren())) {
		if (element->getProperty(e_render)) {
			element->update();
		}
	}

	auto curr = Vec4f(Vec2f(ms_gesture.curr().cursor), 0, 1);  // assume ortho
	m_insideState.curr = getARange().inside(curr);
	if (!ms_gesture.prev().mouse_L && ms_gesture.curr().mouse_L) {
		m_insideState.anchor = m_insideState.curr;
	}
}

// Note: don't draw children
void DuiNode::doRender()
{
	if (m_texture.id() != 0) {
		coreDraw();
	}
}

void DuiNode::doDebugRender()
{
	SpuScopedRenderstate renderstate(true);
	renderstate.flags.fill = false;
	renderstate.use();
	u_debug = 1;
	coreDraw();
	u_debug = 0;
}

float DuiNode::highlightRate(float active_rate) const
{
	if (m_highlight == e_inactive) {
		return 0.5;
	}
	if (m_highlight == e_active) {
		if (insideState().curr) {
			return active_rate;
		}
	}
	return 1.0;
}

void DuiNode::coreDraw()
{
	auto highlight_rate = highlightRate(1.5);
	auto range = getARange();
	auto *current = GsCanvas::getCurrent();

	if (!range.valid()) {
		aux_error(
		        true, "%s : invalid range (%3.1f,%3.1f) - (%3.1f,%3.1f)\n", prettyName().c_str(),
		        range.p0.x, range.p0.y, range.p1.x, range.p1.y);
	}
	u_nodescreen = current->worldscreen(0) * Mat4f(range, 0x0ff).inverse();
	u_color = color() * highlight_rate;
	u_border = select(m_border == Vec2f(0.0), Vec2f(0.5), m_border / Vec2f(range.span() * 0.5));

	u_texture = m_texture.id();
	ms_shader.use();
	ms_array.draw(GL_QUADS);
}
}  // namespace spu::gs_node::dui
