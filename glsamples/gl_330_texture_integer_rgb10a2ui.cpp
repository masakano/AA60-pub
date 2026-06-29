//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};
const char *c_frag = {
    "#version 330 core                                                                      \n"
    "uniform usampler2D u_diffuse;                                                          \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    uvec4 int_color = texture(u_diffuse, f_texcoord);                                  \n"
    "    color = vec4(int_color.rgb, 1023.0) / 1023.0;                                      \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_arrayId;
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture
		{
			Attrs attrs = {
			        {"iformat",    GL_RGB10_A2UI             },
			        //{ "pformat", GL_BGRA_INTEGER },
			        {"pformat",    GL_RGBA_INTEGER           },
			        //{ "ptype", GL_UNSIGNED_INT_2_10_10_10_REV },
			        {"ptype",      GL_UNSIGNED_INT_10_10_10_2},
			        {"min_filter", GL_NEAREST                },
			        {"mag_filter", GL_NEAREST                },
			};
			u_diffuse = loadDDS("kueken7_rgb10a2_unorm.dds", attrs);
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
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_texture_integer_rgb10a2ui");
}  // namespace
}  // namespace spu
