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
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0) out float red;                                                    \n"
    "layout(location = 1) out float green;                                                  \n"
    "layout(location = 2) out float blue;                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 color = texture(u_diffuse, f_texcoord);                                       \n"
    "    red = color.r;                                                                     \n"
    "    green = color.g;                                                                   \n"
    "    blue = color.b;                                                                    \n"
    "}                                                                                      \n"
};

const char *c_vert_blit = {
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

const char *c_frag_blit = {
    "#version 330 core                                                                      \n"
    "uniform sampler2D u_diffuse0;                                                          \n"
    "uniform sampler2D u_diffuse1;                                                          \n"
    "uniform vec4 u_color;                                                                  \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0) out vec4 color;                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = (texture(u_diffuse0, f_texcoord) +                                         \n"
    "            texture(u_diffuse1, f_texcoord)) *                                         \n"
    "            u_color;                                                                   \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[4];  // 0:RGB8 1:R 2:G 3:B

	uint32_t m_samplerId;
	SpuShader m_shaders[2];  // 0: single, 1: multiple
	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	uint32_t u_diffuse0;
	uint32_t u_diffuse1;
	Vec4f u_color;

	uint32_t m_arrayIds[2];
	std::vector<Rectf> m_viewports;
	uint32_t m_frameId;

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 2);

		// program #0
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			        {"u_diffuse",     &m_samplerId  },
			};
			loadShader(m_shaders[0], shader_attrs, unif_attrs);
		}

		// program #1
		{
			// note that array index is not allowed.
			Attrs shader_attrs = {
			        {"vert", c_vert_blit},
			        {"frag", c_frag_blit},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
                                {"u_diffuse0",    &u_diffuse0   },
			        {"u_diffuse0",    &m_samplerId  },
                                {"u_diffuse1",    &u_diffuse1   },
			        {"u_diffuse1",    &m_samplerId  },
                                {"u_color",       &u_color      },
			};
			loadShader(m_shaders[1], shader_attrs, unif_attrs);
		}

		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>();

			for (auto i = 0; i < 2; i++) {
				Attrs attrs = {
				        {"shader_id",    m_shaders[i].id()},
				        {"a.a_position", 2                },
				        {"a.a_texcoord", 2                },
				        {"data",         c_vertices.data()},
				        {"nelem",        c_vertices.size()},
				};
				m_arrayIds[i] = spu_array_new(attrs);
			}
		}

		// sampler
		{
			Vec4f border = {0, 0, 0, 0};
			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER},
			        {"min_filter",   GL_NEAREST        },
			        {"mag_filter",   GL_NEAREST        },
			        {"wrap_s",       GL_CLAMP_TO_EDGE  },
			        {"wrap_t",       GL_CLAMP_TO_EDGE  },
			        {"wrap_r",       GL_CLAMP_TO_EDGE  },
			        {"border",       border            },
			        {"min_lod",      -1000.0           },
			        {"max_lod",      +1000.0           },
			        {"lod_bias",     0.0               },
			        {"compare_mode", GL_NONE           },
			        {"compare_func", GL_LEQUAL         },
			};
			m_samplerId = spu_texture_new(attrs);
		}

		// texture & frame
		{
			struct {
				int32_t form;
				int32_t swizzle_r;
				int32_t swizzle_g;
				int32_t swizzle_b;
				int32_t swizzle_a;
			} param[] = {
			        {GL_BGR, GL_RED,  GL_GREEN, GL_BLUE, GL_ALPHA},
			        {GL_R8,  GL_RED,  GL_ZERO,  GL_ZERO, GL_ZERO },
			        {GL_R8,  GL_ZERO, GL_RED,   GL_ZERO, GL_ZERO },
			        {GL_R8,  GL_ZERO, GL_ZERO,  GL_RED,  GL_ZERO },
			};

			const std::string filename = "kueken7_bgra8_srgb.dds";
			dds::Image image(filename.c_str(), true);

			for (auto i = 0; i < 4; i++) {
				Attrs attrs = {
				        {"target",     GL_TEXTURE_2D     },
                                        {"iformat",    param[i].form     },
				        {"width",      image.width()     },
                                        {"height",     image.height()    },
				        {"min_filter", GL_NEAREST        },
                                        {"mag_filter", GL_NEAREST        },
				        {"swizzle_r",  param[i].swizzle_r},
                                        {"swizzle_g",  param[i].swizzle_g},
				        {"swizzle_b",  param[i].swizzle_b},
                                        {"swizzle_a",  param[i].swizzle_a},
				};

				m_textureIds[i] = spu_texture_new(attrs);
				spu_texture_send(m_textureIds[i], image.pixels(0), GL_BGRA);
			}

			auto viewport = Rectf(0, 0, image.width(), image.height());
			Attrs attrs = {
			        {"viewport0", viewport       },
                                {"color0",    m_textureIds[1]},
			        {"color1",    m_textureIds[2]},
                                {"color2",    m_textureIds[3]},
			        {"bgcolor0",  c_white        },
			};
			m_frameId = spu_frame_new(attrs);
		}

		// blend
		{
			auto &renderstate = getRenderstate();
			renderstate.blend_eq = {GL_FUNC_ADD, GL_FUNC_ADD};
			renderstate.blend_func = {GL_SRC_COLOR, GL_SRC_COLOR, GL_ZERO, GL_ZERO};
			renderstate.flags.blend = true;
			// renderstate.use();
		}
	}

	void render() override
	{
		// Pass 1
		{
			getCamera().setViewscreen(-1.0, 1.0, -1.0, 1.0, -1.0, +1.0, false);
			u_worldscreen = getCamera().viewscreen() * Mat4f().trans({0.0, 0.0, 0.0});

			spu_frame_begin(m_frameId);
			spu_frame_clear(m_frameId);
			u_diffuse = m_textureIds[0];

			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);
			spu_frame_end();
		}

		// Pass 2
		{
			getCamera().setViewscreen(-1.0, 1.0, 1.0, -1.0, -1.0, +1.0, false);
			u_worldscreen = getCamera().viewscreen();
			for (auto i = 0; i < 4; i++) {
				spu_frame_set(-1, "viewport0", m_viewports[i]);
				u_diffuse0 = m_textureIds[i];
				u_diffuse1 = m_textureIds[0];
				u_color = Vec4f(0.25);

				m_shaders[1].use();
				spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_blend_rtt");
}  // namespace
}  // namespace spu
