//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 410 core                                                                      \n"
    "uniform dmat4 u_worldscreen;                                                            \n"
    "in dvec3 a_position;                                                                   \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "    float gl_ClipDistance[];                                                           \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(u_worldscreen * dvec4(a_position, 1.0));                         \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 410 core                                                                      \n"
    "uniform dvec4 u_diffuse;                                                               \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(u_diffuse);                                                           \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	mat4d_t u_worldscreen;
	vec4d_t u_diffuse;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name) {}

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
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<vec3d_t> c_vertices = {
			        {-1.0, -1.0, 0.0},
                                {+1.0, -1.0, 0.0},
                                {+1.0, +1.0, 0.0},
                                {-1.0, +1.0, 0.0}
                        };

			const std::vector<uint16_t> c_indices = {0, 1, 2, 0, 2, 3};

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"format",       GL_DOUBLE        },
			        {"oformat",      GL_DOUBLE        },
                                {"a.a_position", 3                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
			spu_array_send(m_arrayId, c_indices.data(), c_indices.size(), -1, 2);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = mat4d_t(getCamera().worldscreen());
		u_diffuse = {1.0, 0.5, 0.0, +1.0};

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_410_program_64");
}  // namespace
}  // namespace spu
