//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec3 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "flat out int f_index;                                                                  \n"
    "out vec4 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 1.0);                                \n"
    "    f_color = a_color;                                                                 \n"
    "    f_index = gl_VertexID / 4;                                                         \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "const float luminances[2] = float[2](1.0, 0.2);                                        \n"
    "flat in int f_index;                                                                   \n"
    "in vec4 f_color;                                                                       \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = f_color * luminances[f_index];                                             \n"
    "}                                                                                      \n"
};

std::vector<vec4ub_t> const c_colors = {
	{255,   0,   0, 255}, {255, 255,   0, 255},
	{  0, 255,   0, 255}, {  0,   0, 255, 255},
	{255, 128, 128, 255}, {255, 255, 128, 255},
	{128, 255, 128, 255}, {128, 128, 255, 255},
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_black) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// shader
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
			const std::vector<vec3sf_t> c_vertices = {
			        {-1.0, -1.0, +0.5},
                                {+1.0, -1.0, +0.5},
                                {+1.0, +1.0, +0.5},
                                {-1.0, +1.0, +0.5},
			        {-0.5, -1.0, -0.5},
                                {+0.5, -1.0, -0.5},
                                {+1.5, +1.0, -0.5},
                                {-1.5, +1.0, -0.5},
			};

			const std::vector<uint16_t> c_indices = {0, 1, 2, 0, 2, 3};

			Attrs attrs0 = {
			        {"shader_id",    m_shader.id()    },
			        {"a.a_position", 3                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};

			Attrs attrs1 = {
			        {"shader_id", m_shader.id()   },
			        {"format",    GL_UNSIGNED_BYTE},
			        {"normalize", 1               },
			        {"a.a_color", 4               },
			        {"data",      c_colors.data() },
			        {"nelem",     c_colors.size() },
			};

			m_arrayId = spu_array_new(attrs0);
			spu_array_aux(m_arrayId, attrs1, 1);

			spu_array_send(m_arrayId, c_indices.data(), c_indices.size(), -1, 2);
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

		spu_array_set(m_arrayId, "base_vertex", 0);
		spu_array_draw(m_arrayId, GL_TRIANGLES);
		spu_array_set(m_arrayId, "base_vertex", 4);
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_draw_base_vertex");
}  // namespace
}  // namespace spu
