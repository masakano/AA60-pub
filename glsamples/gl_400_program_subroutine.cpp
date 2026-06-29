//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 400 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "uniform float u_displacement;                                                          \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, u_displacement, 1.0);                \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
    "subroutine vec4 diffuse();                                                             \n"
    "subroutine uniform diffuse u_diffuse;                                                  \n"
    "uniform sampler2D u_diffuse_DXT1;                                                      \n"
    "uniform sampler2D u_diffuse_RGB8;                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "subroutine(diffuse)                                                                    \n"
    "vec4 diffuseLQ()                                                                       \n"
    "{                                                                                      \n"
    "    return texture(u_diffuse_DXT1, f_texcoord);                                        \n"
    "}                                                                                      \n"
    "subroutine(diffuse)                                                                    \n"
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
	uint32_t u_diffuse_DXT1 = 0;
	uint32_t u_diffuse_RGB8 = 0;
	float u_displacement = 0;
	int32_t u_diffuse = 0;

	int32_t m_indices[2];
	uint32_t m_arrayId = 0;

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
			        {"u_worldscreen",  &u_worldscreen },
                                {"u_diffuse_DXT1", &u_diffuse_DXT1},
			        {"u_diffuse_RGB8", &u_diffuse_RGB8},
                                {"u_displacement", &u_displacement},
			        {"u_diffuse",      &u_diffuse     },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);

			std::vector<const char *> func_names = {
			        "diffuseLQ",
			        "diffuseHQ",
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
			};
			m_arrayId = squareQuadsArray(attrs);
		}
		// texture
		{
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_srgb.dds");
			m_textureIds[1] = loadDDS("kueken7_bgra8_srgb.dds");
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			// renderstate.use();
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		u_diffuse_DXT1 = m_textureIds[0];
		u_diffuse_RGB8 = m_textureIds[1];

		u_displacement = 0.2;
		u_diffuse = m_indices[0];
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);

		u_displacement = -0.2;
		u_diffuse = m_indices[1];
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_program_subroutine");
}  // namespace
}  // namespace spu
