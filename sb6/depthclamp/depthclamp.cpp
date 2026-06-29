//
// App :
//
#include "base_app.h"
namespace spu::depthclamp {
/* clang-format off */
const char *vert = {
    "#version 410 core                                                                      \n"
    "layout (location = 0) in vec4 position;                                                \n"
    "layout (location = 1) in vec3 normal;                                                  \n"
    "out VS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec3 normal;                                                                       \n"
    "    vec4 color;                                                                        \n"
    "} vs_out;                                                                              \n"
    "                                                                                       \n"
    "uniform mat4 u_modelview;                                                              \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_viewscreen * u_modelview * position;                               \n"
    "    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);                          \n"
    "    vs_out.normal = normalize(mat3(u_modelview) * normal);                             \n"
    "}                                                                                      \n"
};

const char *frag = {
    "#version 410 core                                                                      \n"
    "out vec4 color;                                                                        \n"
    "in VS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec3 normal;                                                                       \n"
    "    vec4 color;                                                                        \n"
    "} fs_in;                                                                               \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0) * abs(normalize(fs_in.normal).z);                                \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;
	SpuShader m_shader;
	Mat4f u_viewscreen;
	Mat4f u_modelview;
	bool m_depthClamp = false;
	sb6::Object m_object;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	{
		Attrs shader_attrs = {
		        {"vert", vert},
		        {"frag", frag},
		};
		Attrs unif_attrs = {
		        {"u_modelview",  &u_modelview },
		        {"u_viewscreen", &u_viewscreen},
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
	}
	// object
	{
		m_object.load("bunny_1k.sbm");
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.cull_face = true;
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
	}
}

void App::menu() { ImGui::Checkbox("depth_clamp", &m_depthClamp); }

void App::render()
{
	auto f = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 1.8, 1000.0);
	u_viewscreen = composition.viewscreen();
	// u_modelview = c_unit.rot("XY", -f * 81.0, -f * 45.0).trans({0.0, 0.0, -2.0});
	u_modelview = c_unit.rot("XY", -f * 81.0, -f * 45.0).trans({0.0, 0.0, -1.9});
	auto &renderstate = getRenderstate();
	renderstate.flags.depth_clamp = m_depthClamp;
	renderstate.use();
	m_shader.use();
	m_object.draw();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("depthclamp");
}  // namespace spu::depthclamp
