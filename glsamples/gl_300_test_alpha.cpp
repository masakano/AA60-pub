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
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 vertTexcoord;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vertTexcoord = a_texcoord;                                                         \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "uniform bool u_use_white;                                                              \n"
    "in vec2 vertTexcoord;                                                                  \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if (u_use_white) {                                                                 \n"
    "        color = vec4(1.0);                                                             \n"
    "    }                                                                                  \n"
    "    else {                                                                             \n"
    "        color = texture(u_diffuse, vertTexcoord);                                      \n"
    "    }                                                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, {1.0, 0.5, 0.0, +1.0}) {}

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
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// texture
		{
			Attrs attrs = {
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			};
			u_diffuse = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}

		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>();

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}

		// renderstate
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.blend = true;
			renderstate.blend_func = {
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			};
			// renderstate.use();
		}
	}

	void render() override
	{
		auto &camera = getCamera();

		camera.setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = camera.worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_300_test_alpha");
}  // namespace
}  // namespace spu
