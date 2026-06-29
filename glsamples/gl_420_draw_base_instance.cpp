//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec4 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen *                                                       \n"
    "       vec4(a_position, float(gl_InstanceID) * 0.25 - 0.5, 1.0);                       \n"
    "    f_color = a_color;                                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "vec4 u_diffuse = vec4(1.0, 0.5, 0.0, 1.0);                                             \n"
    "in vec4 f_color;                                                                       \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = f_color;                                                                   \n"
    "}                                                                                      \n"
};

const uint32_t c_color_count(10);
	
Vec4f const c_colors[c_color_count] = {
	{1.0, 0.5, 0.0, +1.0}, {0.8, 0.4, 0.0, +1.0},
	{0.6, 0.3, 0.0, +1.0}, {0.4, 0.2, 0.0, +1.0},
	{0.2, 0.1, 0.0, +1.0}, {0.0, 0.1, 0.2, +1.0},
	{0.0, 0.2, 0.4, +1.0}, {0.0, 0.3, 0.6, +1.0},
	{0.0, 0.4, 0.8, +1.0}, {0.0, 0.5, 1.0, +1.0}
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_arrayId;

	SpuShader m_shader;
	Mat4f u_worldscreen;

	int32_t m_instanceCount = 5;
	std::vector<vec2sf_t> m_vertices = {
	        {0.0,  0.0 },
                {-1.0, -1.0},
                {+1.0, -1.0},
                {+1.0, +1.0},
                {-1.0, +1.0},
	};
	std::vector<uint32_t> m_indices = {0, 1, 2, 0, 2, 3};

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

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
			Attrs vertex_attrs = {
			        {"shader_id",    m_shader.id()    },
			        {"a.a_position", 2                },
			        {"data",         m_vertices.data()},
			        {"nelem",        m_vertices.size()},
			};
			m_arrayId = spu_array_new(vertex_attrs);

			Attrs color_attrs = {
			        //{"shader_id", m_shader.id()},
			        {"divisor",   1            },
			        {"a.a_color", 4            },
			        {"data",      c_colors     },
			        {"nelem",     c_color_count},
			};
			spu_array_aux(m_arrayId, color_attrs, 2);
			spu_array_send(m_arrayId, m_indices.data(), m_indices.size(), -1, 4);

			Attrs set_attrs = {
			        {"base_vertex",   1},
			        {"base_instance", 5},
			};
			spu_array_set(m_arrayId, set_attrs);
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, m_indices.size(), 5);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_draw_base_instance");
}  // namespace
}  // namespace spu
