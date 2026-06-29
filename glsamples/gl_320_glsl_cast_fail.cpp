//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "#define COUNT 4                                                                        \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out vec4 f_lumimance[COUNT];                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(lowp int i = 0; i < int(COUNT); ++i)                                           \n"
    "        f_lumimance[i] = vec4(1.0) / vec4(COUNT);                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position,0.0,1.0);                             \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "#define COUNT 4                                                                        \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "in vec4 f_lumimance[COUNT];                                                            \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    highp uint first = uint(0);                                                        \n"
    "    vec4 luminance = vec4(0.0);                                                        \n"
    "    for(uint i = first; i < uint(COUNT); ++i)                                          \n"
    "        luminance += f_lumimance[i];                                                   \n"
    "    color = texture(u_diffuse, f_texcoord) * luminance;                                \n"
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
			        {"min_filter", GL_NEAREST      },
			        {"mag_filter", GL_NEAREST      },
			        {"wrap_s",     GL_CLAMP_TO_EDGE},
			        {"wrap_t",     GL_CLAMP_TO_EDGE},
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
		// array #0
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
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_glsl_cast_fail");
}  // namespace
}  // namespace spu
