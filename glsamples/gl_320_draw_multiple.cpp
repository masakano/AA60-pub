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
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * a_position;                                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0, 0.5, 0.0, 1.0);                                                  \n"
    "}                                                                                      \n"
};

//      3 +----+ 2
//        |    |
//        |    |
//      0 +----+ 1

bool use_restart = true;

#define e_restart 0x7fff
const std::vector<int32_t> c_indices = {
        0,       3,     1,     2,
        e_restart,  // restart
        0 + 4,   3 + 4, 1 + 4, 2 + 4,
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	uint32_t m_arrayId;
	Mat4f u_worldscreen;

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

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
			        {"a.a_position", 3                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
			spu_array_send(m_arrayId, c_indices.data(), c_indices.size(), -1);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		m_shader.use();

		if (use_restart) {
			spu_array_set(m_arrayId, "restart", e_restart);  // don't use -1 for restart token
			spu_array_draw(m_arrayId, GL_TRIANGLE_STRIP);
		}
		else {
			spu_array_set(m_arrayId, "base_vertex", 0);
			spu_array_draw(m_arrayId, GL_TRIANGLE_STRIP, 0, 4);
			spu_array_set(m_arrayId, "base_vertex", 4);
			spu_array_draw(m_arrayId, GL_TRIANGLE_STRIP, 0, 4);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_draw_multiple");
}  // namespace
}  // namespace spu
