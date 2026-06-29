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
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "flat out uint f_instance;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    f_instance = uint(gl_InstanceID);                                                  \n"
    "    gl_Position = u_worldscreen *                                                       \n"
    "            vec4(a_position, float(gl_InstanceID) * 0.5, 1.0);                         \n"
    "}                                                                                      \n"
};
const char *c_frag = {
    "#version 400 core                                                                      \n"
    "uniform sampler2DArray u_diffuse[2];                                                   \n"
    "uniform uint u_diffuse_index;                                                          \n"
    "in vec2 f_texcoord;                                                                    \n"
    "flat in uint f_instance;                                                               \n"

    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "vec4 sampling(in sampler2DArray samplers[2], in int layer, in vec2 texcoord)           \n"
    "{                                                                                      \n"
    "    uint index = (u_diffuse_index + f_instance) % 2;                                   \n"
    "    return texture(samplers[index], vec3(texcoord*2, layer));                          \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = sampling(u_diffuse, 0, f_texcoord);                                        \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_samplerIds[2];
	uint32_t m_textureIds[2];  // 0:rgb 1:bgr

	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse[2];
	uint32_t u_samplers[2];
	int32_t u_diffuse_index;

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
			        {"u_worldscreen",   &u_worldscreen  },
			        {"u_diffuse",       &u_diffuse[0]   },
			        {"u_diffuse",       &u_samplers[0]  },
			        {"u_diffuse_index", &u_diffuse_index},
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
		// texture #0
		{
			Attrs attrs = {
			        {"target",    GL_TEXTURE_2D_ARRAY}, // depth = 1
			        {"swizzle_r", GL_RED             },
                                {"swizzle_g", GL_GREEN           },
			        {"swizzle_b", GL_BLUE            },
                                {"swizzle_a", GL_ALPHA           },
			};
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_unorm.dds", attrs);
		}
		{
			Attrs attrs = {
			        {"target",    GL_TEXTURE_2D_ARRAY}, // depth = 1
			        {"swizzle_r", GL_BLUE            },
                                {"swizzle_g", GL_GREEN           },
			        {"swizzle_b", GL_RED             },
                                {"swizzle_a", GL_ALPHA           },
			};
			m_textureIds[1] = loadDDS("kueken7_rgb_dxt1_unorm.dds", attrs);
		}

		// sampler #0
		{
			Vec4f border = {0.5, 0.5, 0.5, 0};
			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER     },
			        {"wrap_s",       GL_REPEAT              },
			        {"wrap_t",       GL_REPEAT              },
			        {"wrap_r",       GL_REPEAT              },
			        {"min_filter",   GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",   GL_LINEAR              },
			        {"min_lod",      -1000.0                },
			        {"max_lod",      +1000.0                },
			        {"lod_bias",     0.0                    },
			        {"compare_mode", GL_NONE                },
			        {"compare_func", GL_LEQUAL              },
			        {"border",       border                 },
			};
			m_samplerIds[0] = spu_texture_new(attrs);
		}
		// sampler #1
		{
			Vec4f border = {0.5, 0.5, 0.5, 0};
			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER     },
			        {"wrap_s",       GL_CLAMP_TO_EDGE       },
			        {"wrap_t",       GL_CLAMP_TO_EDGE       },
			        {"wrap_r",       GL_CLAMP_TO_EDGE       },
			        {"min_filter",   GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",   GL_LINEAR              },
			        {"min_lod",      -1000.0                },
			        {"max_lod",      +1000.0                },
			        {"lod_bias",     0.0                    },
			        {"compare_mode", GL_NONE                },
			        {"compare_func", GL_LEQUAL              },
			        {"border",       border                 },
			};
			m_samplerIds[1] = spu_texture_new(attrs);
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			// renderstate.use();
		}
	}

	void render() override
	{
		static auto frame_count = 0;
		++frame_count;

		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		u_diffuse[0] = m_textureIds[0];
		u_diffuse[1] = m_textureIds[1];
		u_samplers[0] = m_samplerIds[0];
		u_samplers[1] = m_samplerIds[1];
		u_diffuse_index = (frame_count / 30) % 2;
		m_shader.use();

		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 2);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_sampler_array");
}  // namespace
}  // namespace spu
