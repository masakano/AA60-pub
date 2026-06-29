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
    "uniform sampler2DRect u_diffuse;                                                       \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord);                                            \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;

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
			// half
			const float size = 256;
			const std::vector<v2fv2f_t> c_vertices = {
			        {-2.0, -1.5, 0.0,  size},
                                {+2.0, -1.5, size, size},
                                {+2.0, +1.5, size, 0.0 },
			        {+2.0, +1.5, size, 0.0 },
                                {-2.0, +1.5, 0.0,  0.0 },
                                {-2.0, -1.5, 0.0,  size}
                        };

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// texture
		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_RECTANGLE},
			        {"min_filter", GL_LINEAR           },
			        {"mag_filter", GL_LINEAR           },
			};
			u_diffuse = loadDDS("kueken7_bgra8_srgb.dds", attrs);
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

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_texture_rect");
}  // namespace
}  // namespace spu
