//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "#define COUNT 24                                                                       \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec3 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 1.0);                                \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
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
			int32_t width;
			int32_t height;
			uint32_t src_texture_id;
			{
				Attrs attrs = {
				        {"iformat",     GL_COMPRESSED_RGBA_S3TC_DXT1_EXT},
				        {"mag_filter",  GL_NEAREST                      },
				        {"min_filter",  GL_NEAREST                      },
				        {"max_level",   0                               },
				        {"auto_mipmap", 0                               },
				};
				// src_texture_id =
				// loadDDS("kueken7_rgb_dxt1_srgb.dds", attrs);
				src_texture_id = loadDDS("kueken7_rgb_dxt1_unorm.dds", attrs);
				spu_texture_get(src_texture_id, "width", &width);
				spu_texture_get(src_texture_id, "height", &height);
			}
			uint32_t dst_texture_id;
			{
				Attrs attrs = {
				        {"target",      GL_TEXTURE_2D                   },
				        {"iformat",     GL_COMPRESSED_RGBA_S3TC_DXT1_EXT},
				        {"width",       width / 2                       },
				        {"height",      height / 2                      },
				        {"swizzle_r",   GL_RED                          },
				        {"swizzle_g",   GL_GREEN                        },
				        {"swizzle_b",   GL_BLUE                         },
				        {"swizzle_a",   GL_ALPHA                        },
				        {"mag_filter",  GL_NEAREST                      },
				        {"min_filter",  GL_NEAREST                      },
				        {"max_level",   0                               },
				        {"auto_mipmap", 0                               },
				};
				dst_texture_id = spu_texture_new(attrs);
			}

			int32_t src_loc[4] = {width / 4, height / 4, 0, 0};

			spu_texture_copy(dst_texture_id, src_texture_id, nullptr, src_loc, nullptr);
			spu_texture_delete(src_texture_id);
			u_diffuse = dst_texture_id;
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
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_texture_pixel_store");
}  // namespace
}  // namespace spu
