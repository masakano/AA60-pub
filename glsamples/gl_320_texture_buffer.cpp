//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform samplerBuffer u_displacement;                                                  \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "flat out int f_instance;                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_instance = gl_InstanceID;                                                        \n"
    "    gl_Position =                                                                      \n"
    "    u_worldscreen * (vec4(a_position, 0, 0) + texelFetch(u_displacement, gl_InstanceID));\n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform samplerBuffer u_diffuse;                                                       \n"
    "flat in int f_instance;                                                                \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texelFetch(u_diffuse, f_instance);                                         \n"
    "}                                                                                      \n"
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
	uint32_t u_diffuse;
	uint32_t u_displacement;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_white) {}

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
			        {"u_worldscreen",  &u_worldscreen },
			        {"u_diffuse",      &u_diffuse     },
			        {"u_displacement", &u_displacement},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
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
		// texture
		{
			{
				const std::vector<Vec4f> c_displacements = {
				        {+0.1, +0.3, -1.0, +1.0},
				        {-0.5, +0.0, -0.5, +1.0},
				        {-0.2, -0.2, +0.0, +1.0},
				        {+0.3, +0.2, +0.5, +1.0},
				        {+0.1, -0.3, +1.0, +1.0}
                                };

				Attrs attrs = {
				        {"data",    c_displacements.data()                             },
				        {"size",    c_displacements.size() * sizeof(c_displacements[0])},
				        {"target",  GL_TEXTURE_BUFFER                                  },
				        {"iformat", GL_RGBA32F                                         },
				};
				u_displacement = spu_texture_new(attrs);
			}
			{
				const std::vector<vec4ub_t> c_diffuse_datas = {
				        {255, 0,   0,   255},
                                        {255, 127, 0,   255},
                                        {255, 255, 0,   255},
				        {0,   255, 0,   255},
                                        {0,   0,   255, 255},
				};

				Attrs attrs = {
				        {"data",    c_diffuse_datas.data()                             },
				        {"size",    c_diffuse_datas.size() * sizeof(c_diffuse_datas[0])},
				        {"target",  GL_TEXTURE_BUFFER                                  },
				        {"iformat", GL_RGBA8                                           },
				};

				u_diffuse = spu_texture_new(attrs);
			}
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 5);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_texture_buffer");
}  // namespace
}  // namespace spu
