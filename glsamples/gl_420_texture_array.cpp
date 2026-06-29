//
// App :
//
#include "base_app.h"
#include <ssys/random_generator.h>
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out float f_instance;                                                                  \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 offset = vec2(gl_InstanceID % 5, gl_InstanceID / 5) - vec2(2, 1);             \n"
    "    f_instance = gl_InstanceID;                                                        \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position + offset, 0.0, 1.0);                  \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2DArray u_diffuse;                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "in float f_instance;                                                                   \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, vec3(f_texcoord, f_instance));                          \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_sampler;

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
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			        {"u_diffuse",     &u_sampler    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// texture
		{
			const auto width = 4u;
			const auto height = 4u;
			const auto depth = 15;

			RandomGenerator<float> frand = {0.0, 1.0};
			frand.seed(0);

			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_ARRAY},
			        {"iformat",     GL_RGBA8           },
			        {"width",       width              },
			        {"height",      height             },
			        {"depth",       depth              },
			        {"auto_mipmap", 0                  },
			};
			u_diffuse = spu_texture_new(attrs);

			for (auto i = 0; i < depth; i++) {
				const std::vector<float> c_color = {
				        frand(),
				        frand(),
				        frand(),
				        1.0,
				};

				int32_t dst_loc[4] = {0, 0, i, 0};
				uint32_t size[4] = {width, height, 1, 0};
				spu_texture_send(u_diffuse, c_color.data(), GL_RGBA32F, dst_loc, size, true);
			}
		}
		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>(Vec2f(0.4));

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// sampler
		{
			Vec4f border = {0, 0, 0, 0};
			float min_lod = -1000;
			float max_lod = 1000;
			float lod_bias = 0;

			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER},
			        {"min_filter",   GL_NEAREST        },
			        {"mag_filter",   GL_NEAREST        },
			        {"wrap_s",       GL_CLAMP_TO_EDGE  },
			        {"wrap_t",       GL_CLAMP_TO_EDGE  },
			        {"wrap_r",       GL_CLAMP_TO_EDGE  },
			        {"border",       border            },
			        {"min_lod",      min_lod           },
			        {"max_lod",      max_lod           },
			        {"lod_bias",     lod_bias          },
			        {"compare_mode", GL_NONE           },
			        {"compare_func", GL_LEQUAL         },
			};
			u_sampler = spu_texture_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 15);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_texture_array");
}  // namespace
}  // namespace spu
