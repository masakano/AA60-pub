//
// App :
//
#include "base_app.h"
#include "sb6multisample.h"

namespace spu::msaanative {
/* clang-format off */
const char *vs_source = {
    "#version 420 core                                          \n"
    "in vec4 position;                                          \n"
    "out vec3 f_position;                                       \n"
    "uniform mat4 u_modelview;                                  \n"
    "uniform mat4 u_viewscreen;                                 \n"
    "void main()                                                \n"
    "{                                                          \n"
    "    gl_Position = u_viewscreen * u_modelview * position;   \n"
    "    f_position = position.xyz;                             \n"
    "}                                                          \n"
};

const char *fs_source = {
    "#version 420 core						\n"
    "in vec3 f_position;					\n"
    "out vec4 color;						\n"
    "void main()						\n"
    "{								\n"
    "    color.rgb = floor((f_position + 0.25) * 8) / 4;	\n"
    "    color.a = 1.0;						\n"
    "}								\n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;
	Sb6Multisample m_multisample;
	SpuShader m_shader;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	SpuArray m_array;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs shader_attrs = {
		        {"frag", fs_source},
		        {"vert", vs_source},
		};
		Attrs unif_attrs = {
		        {"u_modelview",  &u_modelview },
		        {"u_viewscreen", &u_viewscreen},
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
	}

	// array
	{
		const uint16_t indices[] = {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7,
		                            6, 7, 0, 0, 7, 1, 6, 0, 2, 2, 4, 6, 7, 5, 3, 7, 3, 1};
		const float u = 0.25;
		const float positions[] = {
		        -u, -u, -u, -u, +u, -u, +u, -u, -u, +u, +u, -u,
		        +u, -u, +u, +u, +u, +u, -u, -u, +u, -u, +u, +u,
		};
		Attrs array_attrs = {
		        {"a.0", 3},
		};
		m_array.init(array_attrs);
		m_array.send(positions, 8);
		m_array.send(indices, 36, -1, 2);
	}
	// multisample
	{
		m_multisample.init(viewport(0));
	}
}

void App::menu() { m_multisample.menu(); }

void App::render()
{
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();

	m_multisample.begin();
	auto t = getSeconds().current();

	for (auto i = 0; i < 24; i++) {
		auto f = i + t * 0.3f;
		u_modelview = c_unit.trans({sinf(2.1 * f) * 2.0f, cosf(1.7 * f) * 2.0f,
		                            sinf(1.3 * f) * cosf(1.5 * f) * 2.0f})
		                      .rot("XY", -t * 21.0, -t * 45.0)
		                      .trans({0.0, 0.0, -5.0});
		m_shader.use();
		m_array.draw(GL_TRIANGLES, 0, 36);
	}
	m_multisample.end();
	m_multisample.render();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("msaanative");
}  // namespace spu::msaanative
