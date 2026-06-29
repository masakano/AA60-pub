//
// GsCanvas :
//
#include "environ.h"
#include <gsys/canvas.h>
#include <gsys/drawcall.h>

#define pretty_name prettyName().c_str()

namespace spu {

namespace {
constexpr const char *c_viewport_keys[] = {
        "viewport0", "viewport1", "viewport2", "viewport3", "viewport4", "viewport5", "viewport6", "viewport7",
};
constexpr const char *c_scissor_keys[] = {
        "scissor0", "scissor1", "scissor2", "scissor3", "scissor4", "scissor5", "scissor6", "scissor7",
};
}  // namespace

void GsCanvas::startup(const Attrs &attrs)
{
	assert(ms_default == nullptr);

	Rectf viewport0;
	spu_frame_get(0, "viewport0", viewport0.f);

	Attrs init_attrs = {
	        {"name",      "[default]"},
	        {"frame_id",  0          }, // make real
	        {"viewport0", viewport0  }  // use full viewport
	};
	ms_default = new GsCanvas(init_attrs);

	auto set_attrs = (GsObject::getAttrs() + attrs).select("canvas.");
	ms_default->set(set_attrs);
	assert(ms_stack().size() < 32);
	ms_stack().push_back(ms_default);
}

void GsCanvas::shutdown()
{
	ms_stack().clear();
	delete ms_default;
	ms_default = nullptr;
}

GsCanvas::GsCanvas(const char *name) : GsObject(name)
{
	if (ms_default) {
		m_renderstate = ms_default->getRenderstate();
	}
	else {
		m_renderstate = *GsDrawcall::getDefault();
	}
	for (auto &source: ub_connect.sources) {
		source = {0, 0, 1.0f, 0.0f};
	}
}

GsCanvas::~GsCanvas() { dispose(); }

void GsCanvas::init(const Attrs &attrs)
{
	attrs.subpeek({"u_", "ub_"}, "old convention. use set() with shader. prefix");
	attrs.subpeek({"frame.", "light.", "shader."}, "old convention. use set()");

	// parent
	GsObject::init(attrs);

	// frame
	{
		std::vector<const char *> real_canvas_list = {
		        "color0.", "depth.", "stencil.", "depth_stencil.", "default_", "frame_id",
		};
		if (attrs.subpeek(real_canvas_list)) {
			SpuFrame::init(attrs);
		}
		else {
			SpuFrame::reset(-1, false);  // not owner
		}
	}

	// light & matrix
	if (!ms_stack().empty()) {
		auto *canvas = ms_stack().back();
		takeover(canvas);

		// background
		{
			Vec4f bgcolor0 = Vec4f(0);
			float bgdepth = 1.0;
			uint bgstencil = 1;
			canvas->SpuFrame::get("bgcolor0", &bgcolor0);
			canvas->SpuFrame::get("bgdepth", &bgdepth);
			canvas->SpuFrame::get("bgstencil", &bgstencil);
			Attrs set_attrs = {
			        {"bgcolor0",  bgcolor0 },
			        {"bgdepth",   bgdepth  },
			        {"bgstencil", bgstencil},
			};
			SpuFrame::set(set_attrs);
		}
		if (equal(getViewports().at(0), ezero<Rectf>())) {
			getViewports().at(0) = ms_stack().back()->getViewports().at(0);
		}
		adjustAspect(0);
	}

	// shader type
	{
		attrs.apply("shader_type", m_shaderType);
	}

	// shader
	{
		auto shader_attrs = attrs;
		auto shader_id = shader_attrs.pick("shader_id", 0);
		auto *path = shader_attrs.pick("path", "");

		if (shader_id) {
			m_shader.reset(shader_id, false);  // not owner
		}
		else if (*path) {
			auto *shadowmap = shader_attrs.get<GsCanvas *>("shadowmap", nullptr);
			if (shadowmap) {
				shader_attrs += shadowmap->getShaderAttrs();
			}
			//shader_attrs.prepend("use_unif_block", true); // EXPERIMENTAL
			m_shader.init(path, shader_attrs);
			m_shader.addUniforms(uniformAttrs());
		}
	}
	// set
	{
		set(attrs);
	}
}

void GsCanvas::dispose()
{
	if (vector_is_find(ms_stack(), this)) {
		aux_message(0, "disposing cnvase is on the stack\n");
		for (auto &canvas: ms_stack()) {
			aux_message(0, "    %c %s\n", canvas == this ? '*' : ' ', canvas->pretty_name);
		}
		auto &stack = ms_stack();
		vector_remove(stack, this);
	}
	m_shader.dispose();
	m_array.dispose();
	SpuFrame::dispose();
	GsObject::dispose();
}

void GsCanvas::begin()
{
	assert(!ms_stack().empty());
	aux_error(
	        isReal() && isInStack(), "%s: re-entered canvas. might cause viewport corruption\n",
	        pretty_name);

	ms_stack().push_back(this);
	syncComposition();
	if (isReal()) {
		SpuFrame::begin();
	}
}

void GsCanvas::end()
{
	ms_stack().pop_back();
	if (isReal()) {
		SpuFrame::end();
	}
	else {
		auto curr = ms_stack().back();
		curr->syncComposition();
	}
}

bool GsCanvas::doSync(bool /*is_nonblock*/)
{
	syncComposition();  // always sync
	return false;
}

void GsCanvas::setLight(const Attrs &attrs)
{
	attrs.apply("ambient", ub_light.ambient);
	attrs.apply("sources[0].position", ub_light.sources[0].position);
	attrs.apply("sources[0].direction", ub_light.sources[0].direction);
	attrs.apply("sources[0].emission", ub_light.sources[0].emission);
	attrs.apply("sources[0].radius", ub_light.sources[0].radius);
	attrs.apply("sources[0].decay", ub_light.sources[0].decay);
	attrs.apply("sources[0].exponent", ub_light.sources[0].exponent);
	attrs.apply("sources[0].type", ub_light.sources[0].type);
}

void GsCanvas::setWorldview(const Attrs &attrs)
{
	if (attrs.peek({"eye", "dir", "up"})) {
		auto viewworld = this->viewworld(0);  // copy
		Vec3f eye, dir, up;
		if (viewworld.get_orientation(&eye, &dir, &up) == false) {
			aux_message(0, "setWorldview: cannot set 'eye', 'dir' nor 'up' (ignored)\n");
			return;
		}
		attrs.apply<vec4f_t>("eye", eye);
		attrs.apply<vec4f_t>("dir", dir);
		attrs.apply<vec4f_t>("up", up);
		if (viewworld.set_orientation(&eye, &dir, &up) == false) {
			aux_message(0, "setWorldview: cannot set 'eye', 'dir' nor 'up' (ignored)\n");
			return;
		}
		getWorldviews().front() = viewworld.unitary_inverse();
	}
}

void GsCanvas::setViewscreen(const Attrs &attrs)
{
	auto &viewscreen = getViewscreen();
	float fovy, aspect, near, far;

	viewscreen.get_projection(&fovy, &aspect, &near, &far);
	attrs.apply("fovy", fovy);
	attrs.apply("aspect", aspect);
	attrs.apply("near", near);
	attrs.apply("far", far);

	viewscreen.set_projection(&fovy, &aspect, &near, &far);
}

void GsCanvas::set(const Attrs &attrs)
{
	attrs.subpeek({"u_", "ub_"}, "use 'shader.' prefix");

	// composition
	{
		if (attrs.peek({"eye", "dir", "up"})) {
			setWorldview(attrs);
		}
		if (attrs.peek({"fovy", "aspect", "near", "far"})) {
			setViewscreen(attrs);
		}
		for (auto i = 0; i < 8; i++) {
			attrs.apply<vec4f_t>(c_viewport_keys[i], getViewports().at(i));
			attrs.apply<vec4f_t>(c_scissor_keys[i], getScissors().at(i));
		}
	}
	// frame
	{
		if (isReal()) {
			SpuFrame::set(attrs);
		}
		else {
			auto curr = ms_stack().back();
			curr->SpuFrame::set(attrs);
		}
	}

	// others
	{
		setLight(attrs.select("shader.ub_light."));
		m_shader.setUniforms(attrs.select("shader."));
		m_renderstate.set(attrs.select("renderstate."));
		GsObject::set(attrs);
	}
}

void GsCanvas::render()
{
	if (m_shader.id()) {
		SpuScopedRenderstate renderstate = m_renderstate;
		renderstate.use();
		m_shader.use();
		if (m_array.id()) {
			m_array.draw(GL_TRIANGLE_STRIP);
		}
		else {
			spu_array_draw(0, GL_TRIANGLE_STRIP);
		}
	}
}

void GsCanvas::clear()
{
	syncComposition();
	if (isReal()) {
		SpuFrame::clear();
	}
	else {
		auto curr = ms_stack().back();
		curr->SpuFrame::clear();  // tricky
		curr->syncComposition();
	}
}

void GsCanvas::takeover(const GsCanvas *canvas)
{
	if (canvas == nullptr) {
		canvas = ms_stack().back();
	}
	if (canvas && this != canvas) {
		ub_light = canvas->ub_light;
		Composition::takeover(*canvas);
	}
}

const std::vector<GsCanvas *> &GsCanvas::getStack() { return ms_stack(); }

GsCanvas *GsCanvas::getCurrent()
{
	assert(ms_stack().size() >= 1);
	return ms_stack().back();
}

GsCanvas *GsCanvas::getPrevious()
{
	assert(ms_stack().size() >= 2);
	return ms_stack().at(ms_stack().size() - 2);
}

GsCanvas *GsCanvas::getDefault() { return ms_default; }

bool GsCanvas::peek(int32_t x, int32_t y, const hash32_t &slot, void *value) const
{
	auto &viewport = getViewports().at(0);
	auto &texture = getBuffer(slot);

	uint32_t target = 0;
	uint32_t iformat = 0;

	texture.get("target", &target);
	texture.get("iformat", &iformat);

	if (target == GL_RENDERBUFFER || x < viewport.ox || y < viewport.oy || x > viewport.ox + viewport.sx
	    || y > viewport.oy + viewport.sy) {
		return false;
	}
	else {
		if (iformat == GL_DEPTH32F_STENCIL8) {
			iformat = GL_DEPTH_COMPONENT32F;  // hotfix
		}
		int32_t loc[4] = {x, y, 0, 0};
		uint32_t size[4] = {1, 1, 0, 0};
		texture.recv(value, iformat, loc, size);
		return true;
	}
}

void GsCanvas::syncComposition()
{
	Attrs attrs;
	for (auto i = 0; i < 8; i++) {
		attrs.emplace_back(c_viewport_keys[i], viewport(i));
		attrs.emplace_back(c_scissor_keys[i], scissor(i));
	}
	SpuFrame::set(attrs);
}

bool GsCanvas::isInStack() { return vector_is_find(ms_stack(), this); }
bool GsCanvas::isInShadow() { return isInStack() && this != ms_stack().back(); }

Attrs GsCanvas::uniformAttrs() const
{
	Attrs unif_attrs = {
	        {"ub_light",      &ub_light     },
                {"ub_connect",    &ub_connect   },
	        {"u_color0",      &u_color0     },
                {"u_color1",      &u_color1     },
	        {"u_color2",      &u_color2     },
                {"u_color3",      &u_color3     },
	        {"u_color4",      &u_color4     },
                {"u_color5",      &u_color5     },
	        {"u_color6",      &u_color6     },
                {"u_color7",      &u_color7     },
	        {"u_color8",      &u_color8     },
                {"u_color9",      &u_color9     },
	        {"u_depth",       &u_depth      },
                {"u_depth_ms",    &u_depth      },
	        {"u_stencil",     &u_stencil    },
                {"u_multisample", &u_multisample},
	};
	return unif_attrs;
}

bool GsCanvas::isReal() const { return SpuFrame::id() != uint32_t(-1); }

void GsCanvas::report(const char *str) const
{
	GsObject::report(str);

	aux_printf("    shader type:\n\t '%s'\n", m_shaderType.c_str());
	aux_printf("    frame:\n\t %d\n", SpuFrame::id());
	aux_printf("    uniform block (summery):\n");

	prt_f3("ub_light.ambient", ub_light.ambient);
	prt_f3("ub_light.sources[0].position", ub_light.sources[0].position);
	prt_f3("ub_light.sources[0].direction", ub_light.sources[0].direction);
	prt_f3("ub_light.sources[0].emission", ub_light.sources[0].emission);
	prt_f("ub_light.sources[0].radius", ub_light.sources[0].radius);
	prt_f("ub_light.sources[0].decay", ub_light.sources[0].decay);
	prt_f("ub_light.sources[0].exponent", ub_light.sources[0].exponent);
	prt_i("ub_light.sources[0].type", ub_light.sources[0].type);
	prt_i("ub_connect.sources[0].level", ub_connect.sources[0].level);
	prt_i("ub_connect.sources[0].layer", ub_connect.sources[0].layer);
	prt_f("ub_connect.sources[0].gain", ub_connect.sources[0].gain);
	prt_f("ub_connect.sources[0].bias", ub_connect.sources[0].bias);
	prt_h("u_color0", u_color0);

	aux_printf("    stack:\n");
	for (auto &stack: ms_stack()) {
		auto &viewport = stack->getViewports().at(0);
		aux_printf(
		        "\t%2d %4.0f %4.0f %4.0f %4.0f %s\n", stack->id(), viewport.ox, viewport.oy,
		        viewport.sx, viewport.sy, stack->prettyName().c_str());
	}
	SpuFrame::report("frame");
}
GsObject::ClassCreator<GsCanvas> GsCanvas::ms_classCreator;
}  // namespace spu
