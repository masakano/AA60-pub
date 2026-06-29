//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec3 color;                                        \n"
    "vec3 texelAverage(const vec2 texcoord, in ivec2 offset)                                \n"
    "{                                                                                      \n"
    "    vec4 red = textureGatherOffset(u_diffuse, texcoord, offset, 0);                    \n"
    "    vec4 green = textureGatherOffset(u_diffuse, texcoord, offset, 1);                  \n"
    "    vec4 blue = textureGatherOffset(u_diffuse, texcoord, offset, 2);                   \n"
    "    vec3 texel0 = vec3(red[0], green[0], blue[0]);                                     \n"
    "    vec3 texel1 = vec3(red[1], green[1], blue[1]);                                     \n"
    "    vec3 texel2 = vec3(red[2], green[2], blue[2]);                                     \n"
    "    vec3 texel3 = vec3(red[3], green[3], blue[3]);                                     \n"
    "    return (texel0 + texel1 + texel2 + texel3) * 0.25;                                 \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 size = textureSize(u_diffuse, 0) - 1;                                         \n"
    "    vec2 texcoord = f_texcoord * size;                                                 \n"
    "    ivec2 coord = ivec2(f_texcoord * size);                                            \n"
    "    color = vec3(0);                                                                   \n"
    "    color += texelAverage(f_texcoord, ivec2( 8, 0));                                   \n"
    "    color += texelAverage(f_texcoord, ivec2( 0, 8));                                   \n"
    "    color += texelAverage(f_texcoord, ivec2(-8, 0));                                   \n"
    "    color += texelAverage(f_texcoord, ivec2( 0,-8));                                   \n"
    "    color *= 0.25f;                                                                    \n"
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
			        {"u_diffuse",     &u_diffuse    },
			        {"u_diffuse",     &u_sampler    },
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
		// texture
		{
			u_diffuse = loadDDS("kueken7_rgba_dxt5_unorm.dds");
		}
		// sampler
		{
			float min_lod = -1000;
			float max_lod = 1000;
			float lod_bias = 0;
			float max_aniso = 16;
			Vec4f border = {0.5, 0.5, 0.5, 0};

			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER},
                                {"wrap_s",       GL_CLAMP_TO_EDGE  },
			        {"wrap_t",       GL_CLAMP_TO_EDGE  },
                                {"wrap_r",       GL_CLAMP_TO_EDGE  },
			        {"min_filter",   GL_NEAREST        },
                                {"mag_filter",   GL_NEAREST        },
			        {"min_lod",      min_lod           },
                                {"max_lod",      max_lod           },
			        {"lod_bias",     lod_bias          },
                                {"compare_mode", GL_NONE           },
			        {"compare_func", GL_LEQUAL         },
                                {"border",       border            },
			        {"max_aniso",    max_aniso         },
			};
			u_sampler = spu_texture_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_sampler_gather");
}  // namespace
}  // namespace spu
