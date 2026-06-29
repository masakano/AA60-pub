//
// App :
//
#include "base_app.h"
#include "sb6cube.h"
namespace spu::indexedcube {
/* clang-format off */
const char *vs_source = {
    "#version 420 core                                                                      \n"
    "                                                                                       \n"
    "in vec4 position;                                                                      \n"
    "                                                                                       \n"
    "out VS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} vs_out;                                                                              \n"
    "                                                                                       \n"
    "uniform mat4 u_modelview;                                                              \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_viewscreen * u_modelview * position;                               \n"
    "    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);                          \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 420 core                                                                      \n"
    "                                                                                       \n"
    "out vec4 color;                                                                        \n"
    "                                                                                       \n"
    "in VS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} fs_in;                                                                               \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = fs_in.color;                                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	SpuShader m_shader;
	Cube m_cube;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	Attrs shader_attrs = {
	        {"frag", fs_source},
	        {"vert", vs_source},
	};
	Attrs unif_attrs = {
	        {"u_modelview",  &u_modelview },
	        {"u_viewscreen", &u_viewscreen},
	};
	loadShader(m_shader, shader_attrs, unif_attrs);
	auto &renderstate = getRenderstate();
	renderstate.flags.cull_face = true;
	renderstate.flags.depth_test = true;
	renderstate.depth_func = GL_LEQUAL;
	// renderstate.use();
}

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	for (auto i = 0; i < 24; i++) {
		auto f = i + t * 0.3f;
		u_modelview = c_unit.trans({sinf(2.1 * f) * 2.0f, cosf(1.7 * f) * 2.0f,
		                            sinf(1.3 * f) * cosf(1.5 * f) * 2.0f})
		                      .rot("XY", -t * 21.0, -t * 45.0)
		                      .trans({0.0, 0.0, -7.5});
		m_shader.use();
		m_cube.draw();
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("indexedcube");
}  // namespace spu::indexedcube
