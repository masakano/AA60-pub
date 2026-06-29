//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "out vec4 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position, float(gl_InstanceID) * 0.125 - 0.5, 1.0);\n"
    "    f_color = a_color;                                                                 \n"
    "}                                                                                      \n"
};
const char *c_frag = {
    "#version 330 core                                                                      \n"
    "in vec4 f_color;                                                                       \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = f_color;                                                                   \n"
    "}                                                                                      \n"
};


const auto c_instance_count = 5u;

const std::vector<Vec4f> c_colors = {
	{1.0, 0.0, 0.0, +1.0}, 
	{1.0, 0.5, 0.0, +1.0},
	{1.0, 1.0, 0.0, +1.0}, 
	{0.0, 1.0, 0.0, +1.0},
	{0.0, 0.0, 1.0, +1.0}
};

/* clang-format on */
class App : public BaseApp {
public:
	void initTest()
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		// renderstate.use();
	}

	SpuShader m_shader;
	Mat4f u_worldscreen;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		initTest();
		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			{
				const std::vector<vec2sf_t> c_vertices = squareTriangles();
				Attrs attrs = {
				        {"shader_id",    m_shader.id()    },
				        {"a.a_position", 2                },
				        {"data",         c_vertices.data()},
				        {"nelem",        c_vertices.size()},
				};
				m_arrayId = spu_array_new(attrs);
			}
			{
				Attrs attrs = {
				        {"shader_id", m_shader.id()   },
				        {"divisor",   2               },
				        {"a.a_color", 4               },
				        {"data",      c_colors.data() },
				        {"nelem",     c_instance_count},
				};
				spu_array_aux(m_arrayId, attrs, 1);
			}
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 10);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_draw_instanced_array");
}  // namespace
}  // namespace spu
