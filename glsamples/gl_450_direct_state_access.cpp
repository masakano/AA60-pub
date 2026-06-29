//
// App :
//
#include "base_app.h"
// #include "gl_aux.h"
namespace spu {

namespace {

/* clang-format off */
const char *c_vert = {
    "#version 450 core                                                                      \n"
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
    "#version 450 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, interpolateAtSample(f_texcoord, gl_SampleID));          \n"
    "}                                                                                      \n"
};

const auto c_fb_viewport = Rectf(0, 0, 160, 160);
const auto c_width = int32_t(c_fb_viewport.sx);
const auto c_height = int32_t(c_fb_viewport.sy);

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;

	uint32_t m_textureIds[3];  // 0:texture 1:multisample 2:colorbuffer
	uint32_t m_arrayId;
	uint32_t m_samplerId;
	uint32_t m_frameId;
	uint32_t u_diffuse;
	uint32_t u_sampler;
	Mat4f u_worldscreen;
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
			        {"u_diffuse",     &u_diffuse    },
			        {"u_diffuse",     &u_sampler    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
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
		// array
		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs);
		}

		// texture diffuse
		{
			Attrs attrs = {
			        {"min_filter", GL_NEAREST},
			        {"mag_filter", GL_NEAREST},
			};
			m_textureIds[0] = loadDDS("kueken7_rgba8_srgb.dds", attrs);
		}

		// texture multisample color
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_RGBA8                 },
			        {"width",       c_width                  },
			        {"width",       c_width                  },
			        {"height",      c_height                 },
			        {"base_level",  0                        },
			        {"max_level",   0                        },
			        {"multisample", 4 /*1*/                  },
			        {"auto_mipmap", 0                        },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// texture color
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D},
                                {"iformat",     GL_RGBA8     },
                                {"width",       c_width      },
			        {"height",      c_height     },
                                {"width",       c_width      },
                                {"height",      c_height     },
			        {"base_level",  0            },
                                {"max_level",   0            },
                                {"auto_mipmap", 0            },
			};
			m_textureIds[2] = spu_texture_new(attrs);
		}

		// framebuffer
		{
			auto viewport = Rectf(0, 0, c_fb_viewport.sx, c_fb_viewport.sy);
			auto bgcolor0 = Vec4f(0.0, 0.5, 1.0, +1.0);

			Attrs attrs = {
			        {"color0",         m_textureIds[1]},
			        {"color0.resolve", m_textureIds[2]},
			        {"viewport0",      viewport       },
			        {"bgcolor0",       bgcolor0       },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(c_fb_viewport, 180 * 0.25F, 0.1, 100.0);
		Mat4f u_worldscreen0 = getCamera().worldscreen() * Mat4f();
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		Mat4f u_worldscreen1 = getCamera().viewscreen() * getCamera().worldview().scale(2);

		// Step 1, render the scene in a multisampled framebufferZD
		u_worldscreen = u_worldscreen0;
		u_diffuse = m_textureIds[0];
		u_sampler = m_samplerId;
		m_shader.use();

		// render FBO
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.multisample = true;
			renderstate.flags.sample_shading = true;
			renderstate.min_sample_shading = 1.0;
			renderstate.use();

			spu_frame_begin(m_frameId);
			spu_frame_clear(m_frameId);
			spu_array_draw(m_arrayId, GL_TRIANGLES);
			spu_frame_end();

			renderstate.flags.multisample = false;
			renderstate.use();
		}

		// Step 2, render the colorbuffer from the multisampled framebuffer
		u_worldscreen = u_worldscreen1;
		u_diffuse = m_textureIds[2];
		u_sampler = (GL_TEXTURE_SAMPLER << 16) | 0;
		m_shader.use();

		// render FB
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_450_direct_state_access");
}  // namespace
}  // namespace spu
