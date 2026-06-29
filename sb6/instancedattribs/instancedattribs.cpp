//
// App :
//
#include "base_app.h"
namespace spu::instancedattribs {
/* clang-format off */
const char *square_vs_source = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "layout (location = 0) in vec4 position;                                                \n"
    "layout (location = 1) in vec4 instance_color;                                          \n"
    "layout (location = 2) in vec4 instance_position;                                       \n"
    "                                                                                       \n"
    "out Fragment                                                                           \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} fragment;                                                                            \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = (position + instance_position) * vec4(0.25, 0.25, 1.0, 1.0);         \n"
    "    fragment.color = instance_color;                                                   \n"
    "}                                                                                      \n"
};

const char *square_fs_source = {
    "#version 410 core                                                                      \n"
    "precision highp float;                                                                 \n"
    "                                                                                       \n"
    "in Fragment                                                                            \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} fragment;                                                                            \n"
    "                                                                                       \n"
    "out vec4 color;                                                                        \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = fragment.color;                                                            \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	uint32_t m_arrayId;
	SpuShader m_shader;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	const float square_vertices[]
	        = {-1.0, -1.0, 0.0, 1.0, 1.0, -1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, -1.0, 1.0, 0.0, 1.0};
	const float instance_colors[]
	        = {1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0};
	const float instance_positions[]
	        = {-2.0, -2.0, 0.0, 0.0, +2.0, -2.0, 0.0, 0.0, +2.0, +2.0, 0.0, 0.0, -2.0, +2.0, 0.0, 0.0};
	Attrs attr0 = {
	        {"a.0", 4}
        };
	Attrs attr1 = {
	        {"divisor", 1},
                {"a.1",     4}
        };
	Attrs attr2 = {
	        {"divisor", 1},
                {"a.2",     4}
        };

	m_arrayId = spu_array_new(attr0);
	spu_array_aux(m_arrayId, attr1, 1);
	spu_array_aux(m_arrayId, attr2, 2);
	spu_array_send(m_arrayId, square_vertices, 4, 0);
	spu_array_send(m_arrayId, instance_colors, 4, 1);
	spu_array_send(m_arrayId, instance_positions, 4, 2);
	Attrs shader_attrs = {
	        {"frag", square_fs_source},
	        {"vert", square_vs_source},
	};
	loadShader(m_shader, shader_attrs, Attrs());
	// m_shader.addUniforms(unif_attrs);
}

void App::render()
{
	m_shader.use();
	spu_array_draw(m_arrayId, GL_TRIANGLE_FAN, 0, 4, 4);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("instancedattribs");
}  // namespace spu::instancedattribs
