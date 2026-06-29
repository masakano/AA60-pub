//
// App :
//
#include "base_app.h"
#include "sb6multisample.h"

namespace spu::linesmooth {
/* clang-format off */
const char *render_vert = {
    "#version 410 core                                                                      \n"
    "layout (location = 0) in vec4 position;                                                \n"
    "layout (location = 1) in mat4 modelview;                                               \n"
    "uniform mat4 u_modelview; /*debug*/                                                    \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "flat out vec3 f_color;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec3 colors[6] = vec3[6](                                                          \n"
    "        vec3(1,0,0), vec3(0,1,0), vec3(0,0,1),                                         \n"
    "        vec3(0,1,1), vec3(1,0,1), vec3(1,1,0));                                        \n"
    "    gl_Position = u_viewscreen * modelview * position;                                 \n"
    "    f_color = colors[gl_InstanceID % 6];                                               \n"
    "}                                                                                      \n"
};

const char *render_frag = {
    "#version 410 core                                                                      \n"
    "flat in vec3 f_color;                                                                  \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(f_color, 1);                                                          \n"
    "}                                                                                      \n"
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
	uint32_t m_array;
	Mat4f u_viewscreen;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs shader_attrs = {
		        {"frag", render_frag},
		        {"vert", render_vert},
		};
		Attrs unif_attrs = {
		        //{"u_modelview",  &u_modelview },
		        {"u_viewscreen", &u_viewscreen},
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
	}
	// array
	{
		const std::vector<uint16_t> indices = {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7,
		                                       6, 7, 0, 0, 7, 1, 6, 0, 2, 2, 4, 6, 7, 5, 3, 7, 3, 1};
		const float u = 0.25;
		const std::vector<float> positions = {
		        -u, -u, -u, -u, +u, -u, +u, -u, -u, +u, +u, -u,
		        +u, -u, +u, +u, +u, +u, -u, -u, +u, -u, +u, +u,
		};
		Attrs array_attrs0 = {
		        {"a.0", 3}
                };
		m_array = spu_array_new(array_attrs0);
		spu_array_send(m_array, positions.data(), positions.size());
		Attrs array_attrs1 = {
		        {"divisor", 1 },
                        {"a.1",     16}
                };
		spu_array_aux(m_array, array_attrs1, 1);
		spu_array_send(m_array, indices.data(), indices.size(), -1, 2);
	}
	// multisample
	{
		m_multisample.init(viewport(0));
	}
}

void App::menu() { m_multisample.menu(); }

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();

	std::vector<Mat4f> mv_matrices;
	for (auto i = 0; i < 24; i++) {
		auto f = i + t * 0.3f;
		mv_matrices.push_back(c_unit.trans({sinf(2.1 * f) * 2.0f, cosf(1.7 * f) * 2.0f,
		                                    sinf(1.3 * f) * cosf(1.5 * f) * 2.0f})
		                              .rot("XY", t * 21.0, t * 45.0)
		                              .trans({0.0, 0.0, -7.5}));
	}
	m_multisample.begin();
	m_shader.use();
	spu_array_send(m_array, mv_matrices.data(), mv_matrices.size(), 1);
	spu_array_draw(m_array, GL_TRIANGLES, 0, 36, mv_matrices.size());
	m_multisample.end();
	m_multisample.render();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("linesmooth");
}  // namespace spu::linesmooth
