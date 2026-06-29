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
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
    "#extension GL_NV_gpu_shader5 : require                                                 \n"
    "uniform sampler2D u_diffuse[2];                                                        \n"
    "// debug                                                                               \n"
    "uniform sampler2D u_diffuse0;                                                          \n"
    "uniform sampler2D u_diffuse1;                                                          \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    int index = (int(f_texcoord.x * 8) + int(f_texcoord.y * 8)) % 2;                   \n"
    "    color = texture(u_diffuse[index], f_texcoord * 2.0 - 0.5);                         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[2];  // 0:RGB 1:BGR
	uint32_t m_samplerId = 0;

	SpuShader m_shader;
	uint32_t u_diffuse[2];
	uint32_t u_samplers[2];
	Mat4f u_worldscreen;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, {1.0, 0.5, 0.0, +1.0}) {}

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
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// texture
		{
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_srgb.dds");
		}

		// texture #1
		{
			m_textureIds[1] = loadDDS("kueken7_rgb_dxt1_srgb.dds");
		}

		// sampler
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
			m_samplerId = spu_texture_new(attrs);
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
		/*
		// array
		{
		        const std::vector<v2fv2f_t> c_vertices = squareQuads<v2fv2f_t>();
		        const std::vector<uint32_t> c_indices = {0, 1, 2, 2, 3, 0};

		        Attrs attrs = {
		                {"shader_id",    m_shader.id()  },
		                {"a.a_position", 2              },
		                {"a.a_texcoord", 2              },
		                {"data",         c_vertices.data()},
		                {"nelem",        c_vertices.size()},
		        };
		        m_arrayId = spu_array_new(attrs);
		        spu_array_send(m_arrayId, c_indices.data(), c_indices.size(), -1, 4);
		}
		*/
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();

		u_diffuse[0] = m_textureIds[0];
		u_diffuse[1] = m_textureIds[1];

		u_samplers[0] = m_samplerId;
		u_samplers[1] = m_samplerId;

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_sampler_array_nv");
}  // namespace
}  // namespace spu
