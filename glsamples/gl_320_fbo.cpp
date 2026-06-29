//
// App :
//
#include "base_app.h"

#include "gl_320_texture_2d.h"
namespace spu {
namespace {

#define e_fb_scale "2"  // string

/* clang-format off */
const char *c_vert_blit = {
    "#version 330                                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(4*(gl_VertexID%2)-1, 4*(gl_VertexID/2)-1, 0, 1);                \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 texture_size = vec2(textureSize(u_diffuse, 0));                               \n"
    "    color = texture(u_diffuse, gl_FragCoord.xy * " e_fb_scale " / texture_size); "
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_diffuseId;
	uint32_t m_colorId;
	uint32_t m_renderbufferId;
	uint32_t m_frameId;

	SpuShader m_shaders[2];
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_arrayIds[2];

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		const auto fb_scale = std::stof(e_fb_scale);
		auto fb_viewport = Rectf(0.0f, 0.0f, viewport(0).sx * fb_scale, viewport(0).sy * fb_scale);
		auto width = int32_t(fb_viewport.sx);
		auto height = int32_t(fb_viewport.sy);

		// program
		{
			const char *verts[] = {
			        c_vert,
			        c_vert_blit,
			};

			const char *frags[] = {
			        c_frag,
			        c_frag_blit,
			};

			for (auto &shader: m_shaders) {
				auto i = &shader - &m_shaders[0];
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen},
				        {"u_diffuse",     &u_diffuse    },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			}
		}

		// diffuse
		{
			Attrs attrs = {
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_CLAMP_TO_EDGE       },
			        {"wrap_t",     GL_CLAMP_TO_EDGE       },
			};
			m_diffuseId = loadDDS("kueken7_rgb_dxt1_srgb.dds", attrs);
		}

		// color
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D  },
                                {"iformat",     GL_SRGB8_ALPHA8},
			        {"width",       width          },
                                {"height",      height         },
			        {"base_level",  0              },
                                {"max_level",   0              },
			        {"min_filter",  GL_LINEAR      },
                                {"mag_filter",  GL_LINEAR      },
			        {"auto_mipmap", 0              },
			};
			m_colorId = spu_texture_new(attrs);
		}

		// render buffer
		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D        },
                                {"iformat",    GL_DEPTH_COMPONENT32F},
			        {"width",      width                },
                                {"height",     height               },
			        {"base_level", 0                    },
                                {"max_level",  0                    },
			};
			m_renderbufferId = spu_texture_new(attrs);
		}

		// frame buffer
		{
			Attrs attrs = {
			        {"color0",    m_colorId       },
			        {"depth",     m_renderbufferId},
			        {"viewport0", fb_viewport     },
			        {"bgcolor0",  c_orange        },
			};
			m_frameId = spu_frame_new(attrs);
		}

		// array #0
		{
			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 2                },
			        {"a.a_texcoord", 2                },
			};
			m_arrayIds[0] = squareQuadsArray<v2fv2f_t>(attrs);
		}

		// array #1
		{
			Attrs attrs = {
			        {"shader_id",    m_shaders[1].id()},
			        {"a.a_position", 2                },
			        {"a.a_texcoord", 2                },
			};
			m_arrayIds[1] = spu_array_new(attrs);
			spu_array_link(m_arrayIds[1], m_arrayIds[0], 0, 0);
			spu_array_link(m_arrayIds[1], m_arrayIds[0], -1, -1);
		}
	}

	void render() override
	{
		{
			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
			u_worldscreen = getCamera().worldscreen();
		}

		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			// renderstate.flags.depth_test = false;
			renderstate.depth_func = GL_LESS;
			renderstate.use();

			spu_frame_begin(m_frameId);
			spu_frame_clear(m_frameId);

			u_diffuse = m_diffuseId;
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);

			spu_frame_end();
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = false;
			renderstate.use();

			u_diffuse = m_colorId;
			m_shaders[1].use();

			spu_array_draw(m_arrayIds[1], GL_TRIANGLES, 0, 3);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo");
}  // namespace
}  // namespace spu
