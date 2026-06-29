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
	uint32_t m_textureIds[4];  // 0:RGB8, 1:DXT5, 2:RGTC, 3:BPTC
	std::vector<Rectf> m_viewports;

	uint32_t m_samplerId;
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_sampler;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 2);

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
			Attrs attrs = {
			        {"base_level", 0},
			        {"max_level",  0},
			};

			m_textureIds[0] = loadDDS("kueken7_rgba_dxt5_srgb.dds", attrs);
			m_textureIds[1] = loadDDS("kueken7_rgba_dxt5_unorm.dds", attrs);
			m_textureIds[2] = loadDDS("kueken7_rgba8_snorm.dds", attrs);  // need fix
			m_textureIds[3] = loadDDS("kueken7_rgb9e5_ufloat.dds", attrs);
		}
		// sampler
		{
			Vec4f border = {0, 0, 0, 0};
			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER       },
			        {"min_filter",   GL_NEAREST_MIPMAP_NEAREST},
			        {"mag_filter",   GL_NEAREST               },
			        {"wrap_s",       GL_CLAMP_TO_EDGE         },
			        {"wrap_t",       GL_CLAMP_TO_EDGE         },
			        {"wrap_r",       GL_CLAMP_TO_EDGE         },
			        {"border",       border                   },
			        {"min_lod",      -1000.0                  },
			        {"max_lod",      +1000.0                  },
			        {"lod_bias",     0.0                      },
			        {"compare_mode", GL_NONE                  },
			        {"compare_func", GL_LEQUAL                },
			};
			m_samplerId = spu_texture_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();

		// Clear the color buffer
		// Draw each texture in different viewports
		for (auto i = 0; i < 4; i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			u_diffuse = m_textureIds[i];
			u_sampler = m_samplerId;
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_texture_compressed");
}  // namespace
}  // namespace spu
