//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 400                                                                           \n"
    "uniform samplerBuffer u_displacement;                                                  \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "in vec2 a_position;                                                                    \n"
    "flat out int f_instance;                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_instance = gl_InstanceID;                                                        \n"
    "    gl_Position = u_worldscreen *                                                      \n"
    "                  (vec4(a_position, 0.0, 0.0) +                                        \n"
    "                   texelFetch(u_displacement, gl_InstanceID));                         \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400                                                                           \n"
    "uniform samplerBuffer u_diffuse;                                                       \n"
    "flat in int f_instance;                                                                \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texelFetch(u_diffuse, f_instance);                                         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_displacement;
	uint32_t m_arrayId;

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
			        {"u_worldscreen",  &u_worldscreen },
			        {"u_diffuse",      &u_diffuse     },
			        {"u_displacement", &u_displacement},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs);
		}
		// displacement
		{
			const std::vector<vec3sf_t> c_displacements = {
			        {+0.1, +0.3, -1.0},
                                {-0.5, +0.0, -0.5},
                                {-0.2, -0.2, +0.0},
			        {+0.3, +0.2, +0.5},
                                {+0.1, -0.3, +1.0},
			};

			Attrs attrs = {
			        {"data",    c_displacements.data()                             },
			        {"size",    c_displacements.size() * sizeof(c_displacements[0])},
			        {"target",  GL_TEXTURE_BUFFER                                  },
			        {"iformat", GL_RGB32F                                          },
			};
			u_displacement = spu_texture_new(attrs);
		}
		// diffuse
		{
			const std::vector<vec3sf_t> c_diffuses = {
			        {1.0, 0.0, 0.0},
                                {1.0, 0.5, 0.0},
                                {1.0, 1.0, 0.0},
			        {0.0, 1.0, 0.0},
                                {0.0, 0.0, 1.0},
			};

			Attrs attrs = {
			        {"data",    c_diffuses.data()                        },
			        {"size",    c_diffuses.size() * sizeof(c_diffuses[0])},
			        {"target",  GL_TEXTURE_BUFFER                        },
			        {"iformat", GL_RGB32F                                },
			};
			u_diffuse = spu_texture_new(attrs);
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
		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 5);
	}
};
}  // namespace
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_texture_buffer_rgb");
}  // namespace spu
