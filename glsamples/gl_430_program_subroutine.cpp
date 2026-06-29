//
// App :
//
#include "base_app.h"
namespace spu {
// #define USE_PIPELINE  // not work

namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 g_texcoord;                                                                   \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    g_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};
const char *c_geom = {
    "#version 420 core                                                                      \n"
    "layout(triangles, invocations = 1) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 3) out;                                          \n"
    "in gl_PerVertex                                                                        \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "} gl_in[];                                                                             \n"
    "in vec2 g_texcoord[];                                                                  \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "flat out int f_instance;                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        gl_Position = gl_in[i].gl_Position;                                            \n"
    "        f_texcoord = g_texcoord[i];                                                    \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "#extension GL_ARB_explicit_uniform_location : require                                  \n"
    "subroutine vec4 diffuse();                                                             \n"
    "layout(location = 0) subroutine uniform diffuse u_diffuse;                             \n"
    "uniform sampler2D u_diffuse_DXT1;                                                      \n"
    "uniform sampler2D u_diffuse_RGB8;                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "flat in int f_instance;                                                                \n"

    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "layout(index = 0) subroutine(diffuse)                                                  \n"
    "vec4 diffuseLQ()                                                                       \n"
    "{                                                                                      \n"
    "    return texture(u_diffuse_DXT1, f_texcoord);                                        \n"
    "}                                                                                      \n"
    "layout(index = 1) subroutine(diffuse)                                                  \n"
    "vec4 diffuseHQ()                                                                       \n"
    "{                                                                                      \n"
    "    return texture(u_diffuse_RGB8, f_texcoord);                                        \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = u_diffuse();                                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[2];  // 0:DXT1 1:RGB8

	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse_RGB8 = 0;
	uint32_t u_diffuse_DXT1 = 0;
	uint32_t u_diffuse = 0;
	int32_t m_indices[2] = {0, 0};

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert}, // m_shaders[0] only
			        {"geom", c_geom},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen",  &u_worldscreen }, // UBO
			        {"u_diffuse_DXT1", &u_diffuse_DXT1},
			        {"u_diffuse_RGB8", &u_diffuse_RGB8},
			        {"u_diffuse",      &u_diffuse     },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);

			std::vector<const char *> func_names = {
			        "diffuseLQ",  // sampleing::DXT1
			        "diffuseHQ",  // sampleing::RGB8
			};
			spu_shader_loc(
			        m_shader.id(), func_names.data(), m_indices, nullptr, nullptr,
			        func_names.size());
		}
		// array
		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs, Vec2f(2.0, 1.0));
		}

		// texture #0
		{
			Attrs attrs = {
			        {"swizzle_r", GL_BLUE },
			        {"swizzle_g", GL_GREEN},
			        {"swizzle_b", GL_RED  },
			};
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_srgb.dds", attrs);
		}

		// texture #1
		{
			Attrs attrs = {
			        {"swizzle_r", GL_RED  },
			        {"swizzle_g", GL_GREEN},
			        {"swizzle_b", GL_BLUE },
			};
			m_textureIds[1] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}

		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
		}
	}

	void render() override
	{
		std::vector<Rectf> m_viewports = makeViewports(2, 1);
		spu_frame_set(-1, "viewport0", m_viewports[0]);

		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		u_diffuse_RGB8 = m_textureIds[1];
		u_diffuse_DXT1 = m_textureIds[0];

		{
			u_diffuse = m_indices[0];
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}

		{
			spu_frame_set(-1, "viewport0", m_viewports[1]);
			u_diffuse = m_indices[1];
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};  // namespace spu

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_program_subroutine");
}  // namespace
}  // namespace spu
