//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "#extension GL_ARB_cull_distance : require                                              \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec3 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_CullDistance[1];                                                          \n"
    "};                                                                                     \n"
    "out vec4 f_position;                                                                   \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 position = u_worldscreen * vec4(a_position, 1.0);                              \n"
    "    f_position = position;                                                             \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = position;                                                            \n"
    "    gl_CullDistance[0] = mix(-1.0, 1.0, position.z > 2);                               \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec4 f_position;                                                                    \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "uniform bool u_is_cull;                                                                \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if(u_is_cull && int(f_position.z) % 2 == 0)                                        \n"
    "        discard;                                                                       \n"
    "    color = texture(u_diffuse, f_texcoord.st);                                         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_is_cull;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, {1.0, 0.5, 0.0, 1.0}) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture
		{
			Attrs attrs = {
			        {"mag_filter", GL_LINEAR              },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			};
			u_diffuse = loadDDS("kueken7_rgba8_srgb.dds", attrs);
		}
		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			        {"u_is_cull",     &u_is_cull    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareQuads<v2fv2f_t>();
			const std::vector<uint16_t> c_indices = {0, 1, 2, 2, 3, 0};

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_texcoord", 2                },
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

		u_worldscreen = getCamera().worldscreen();

		auto &renderstate = getRenderstate();
		renderstate.flags.fill = true;
		renderstate.use();
		u_is_cull = 1;
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);

		renderstate.flags.fill = false;
		renderstate.use();
		u_is_cull = 0;
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_450_culling");
}  // namespace
}  // namespace spu
