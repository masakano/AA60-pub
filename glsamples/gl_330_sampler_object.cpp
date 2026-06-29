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
    "uniform sampler2D u_diffuse[2];                                                        \n"
    "uniform vec4 u_color;                                                                  \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = (texture(u_diffuse[0], f_texcoord) +                                       \n"
    "             texture(u_diffuse[1], f_texcoord) +                                       \n"
    "             u_color) * 0.33;                                                          \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse[2];
	uint32_t u_samplers[2];
	Vec4f m_color;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

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
			        {"u_diffuse",     &u_diffuse[0] },
			        {"u_diffuse",     &u_samplers[0]},
			        {"u_color",       &m_color      },
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
		// texture
		{
			Attrs attrs = {
			        {"swizzle_r", GL_RED  },
			        {"swizzle_g", GL_GREEN},
			        {"swizzle_b", GL_BLUE },
			        {"swizzle_a", GL_ALPHA},
			};
			u_diffuse[0] = loadDDS("kueken7_rgba_dxt5_unorm.dds", attrs);
			u_diffuse[1] = u_diffuse[0];
		}
		// sampler
		{
			float min_lod = -1000;
			float max_lod = +1000;
			float lod_bias = 0.0;
			Vec4f border = {0, 0, 0, 0};

			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER},
                                {"wrap_s",       GL_CLAMP_TO_EDGE  },
			        {"wrap_t",       GL_CLAMP_TO_EDGE  },
                                {"wrap_r",       GL_CLAMP_TO_EDGE  },
			        {"min_lod",      min_lod           },
                                {"max_lod",      max_lod           },
			        {"lod_bias",     lod_bias          },
                                {"border",       border            },
			        {"compare_mode", GL_NONE           },
                                {"compare_func", GL_LEQUAL         },
			};

			Attrs attrs_a = {
			        {"min_filter", GL_NEAREST},
			        {"mag_filter", GL_NEAREST},
			};
			u_samplers[0] = spu_texture_new(attrs + attrs_a);

			Attrs attrs_b = {
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			};
			u_samplers[1] = spu_texture_new(attrs + attrs_b);
		}
		// initVertexArray();
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();

		// Bind the program for useBNN
		m_color = Vec4f(1.0, 0.5, 0.0, +1.0);

		m_shader.use();

		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_sampler_object");
}  // namespace
}  // namespace spu
