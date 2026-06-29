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
    "in vec3 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_ClipDistance[2];                                                          \n"
    "};                                                                                     \n"
    "out vec4 f_position;                                                                   \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 position = u_worldscreen * vec4(a_position, 1.0);                              \n"
    "    f_position = position;                                                             \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = position;                                                            \n"
    "    gl_ClipDistance[0] = +position.z-4;                                                \n"
    "    gl_ClipDistance[1] = -position.z+6;                                                \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec4 f_position;                                                                    \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord.st);                                         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture
		{
			Attrs attrs = {
			        {"mag_filter", GL_LINEAR              },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			};
			u_diffuse = loadDDS("kueken7_bgra8_srgb.dds", attrs);
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
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();

		auto &renderstate = getRenderstate();
		renderstate.flags.clip_distance0 = true;
		renderstate.flags.clip_distance1 = true;
		renderstate.flags.fill = true;
		renderstate.use();

		spu_array_draw(m_arrayId, GL_TRIANGLES);

		renderstate.flags.clip_distance0 = false;
		renderstate.flags.clip_distance1 = false;
		renderstate.flags.fill = false;
		renderstate.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_clipping");
}  // namespace
}  // namespace spu
