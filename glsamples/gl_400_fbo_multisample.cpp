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
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, interpolateAtSample(f_texcoord, gl_SampleID));          \n"
    "}                                                                                      \n"
};

const auto c_fb_viewport = Rectf(0, 0, 60, 60);
const auto c_width = int32_t(c_fb_viewport.sx);
const auto c_height = int32_t(c_fb_viewport.sy);

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureDiffuseId;
	uint32_t m_textureMultiId;
	uint32_t m_textureId;
	uint32_t m_samplerId;

	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_sampler;
	uint32_t m_arrayId;
	uint32_t m_frameId;

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
		// sampler
		{
			float min_lod = -1000;
			float max_lod = 1000;
			float lod_bias = 0;

			Attrs sampler_attrs = {
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
			};
			m_samplerId = spu_texture_new(sampler_attrs);
		}

		// array
		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs, Vec2f(1.5, 1.0));
		}

		// texture #0
		{
			m_textureDiffuseId = loadDDS("kueken7_bgra8_srgb.dds", Attrs());
		}

		// texture #1
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D},
                                {"iformat",     GL_RGBA8     },
                                {"width",       c_width      },
			        {"height",      c_height     },
                                {"width",       c_width      },
                                {"height",      c_height     },
			        {"min_filter",  GL_NEAREST   },
                                {"mag_filter",  GL_NEAREST   },
                                {"auto_mipmap", 0            },
			};
			m_textureId = spu_texture_new(attrs);
		}

		// texture #2
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_RGBA8                 },
			        {"multisample", 4                        },
			        {"width",       c_width                  },
			        {"height",      c_height                 },
			        {"width",       c_width                  },
			        {"height",      c_height                 },
			};
			m_textureMultiId = spu_texture_new(attrs);
		}

		// frame
		{
			Attrs attrs = {
			        {"color0",         m_textureMultiId},
			        {"color0.resolve", m_textureId     },
			        {"viewport0",      c_fb_viewport   },
			        {"bgcolor0",       c_sky           },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		// render FBO
		{
			MultisampleControl mcontrol(m_frameId, getGesture());

			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

			u_worldscreen = getCamera().worldscreen();
			u_diffuse = m_textureDiffuseId;
			u_sampler = m_samplerId;
			m_shader.use();

			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
		// render FB
		{
			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

			u_worldscreen = getCamera().worldscreen();
			u_diffuse = m_textureId;
			u_sampler = m_samplerId;
			m_shader.use();

			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_fbo_multisample");
}  // namespace
}  // namespace spu
