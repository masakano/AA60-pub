//
// App :
//
#include "base_app.h"
#include "gl_320_texture_2d.h"
namespace spu {
namespace {

#define e_fb_scale "2"

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
    "    color = texture(u_diffuse, gl_FragCoord.xy * " e_fb_scale " / texture_size);	    \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[3];  // 0:diffuse 1:colorbuffer 2:renderbuffer
	SpuShader m_shaders[2];    // 0:texture 1:splash

	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_frameId;
	uint32_t m_arrayIds[2];

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		const auto fb_scale = std::stof(e_fb_scale);
		auto fb_viewport = Rectf(0.0f, 0.0f, viewport(0).sx * fb_scale, viewport(0).sy * fb_scale);
		auto width = int32_t(fb_viewport.sx);
		auto height = int32_t(fb_viewport.sy);

		// shader
		{
			const char *verts[2] = {c_vert, c_vert_blit};
			const char *frags[2] = {c_frag, c_frag_blit};

			for (auto i = 0; i < 2; i++) {
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen},
				        {"u_diffuse",     &u_diffuse    },
				};
				loadShader(m_shaders[i], shader_attrs, unif_attrs);
			}
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
			        {"a.0",   1},
                                {"nelem", 3}, // one triangle
			};
			m_arrayIds[1] = spu_array_new(attrs);
		}

		// texture #0
		{
			Attrs attrs = {
			        {"min_filter", GL_NEAREST_MIPMAP_NEAREST},
			        {"mag_filter", GL_NEAREST               },
			        {"wrap_s",     GL_CLAMP_TO_EDGE         },
			        {"wrap_t",     GL_CLAMP_TO_EDGE         },
			};
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_srgb.dds", attrs);
		}

		// texture #1
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
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// texture #2
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D      },
                                {"iformat",     GL_DEPTH24_STENCIL8},
			        {"width",       width              },
                                {"height",      height             },
			        {"base_level",  0                  },
                                {"max_level",   0                  },
			        {"auto_mipmap", 0                  },
			};
			m_textureIds[2] = spu_texture_new(attrs);
		}

		// frame buffer
		{
			Attrs attrs = {
			        {"color0",        m_textureIds[1]},
			        {"depth_stencil", m_textureIds[2]},
			        {"viewport0",     fb_viewport    },
			        {"bgcolor0",      c_orange       },
			        {"bgdepth",       1.0            },
			        {"bgstencil",     0              },
			};
			m_frameId = spu_frame_new(attrs);
			// spu_frame_report(m_frameId);
		}
	}

	void render() override
	{
		{
			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
			auto worldscreen = getCamera().worldscreen();
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = false;
			renderstate.flags.stencil_test = true;

			// spu_frame_report(m_frameId);
			spu_frame_begin(m_frameId);
			spu_frame_clear(m_frameId);

			u_worldscreen = worldscreen * Mat4f().scale(0.75);
			u_diffuse = m_textureIds[0];
			m_shaders[0].use();

			renderstate.write_mask = {1, 0, 0, 0, 0};
			renderstate.stencil_func = {
			        GL_ALWAYS, 1, ~0u, GL_KEEP, GL_KEEP, GL_REPLACE,
			        GL_ALWAYS, 1, ~0u, GL_KEEP, GL_KEEP, GL_REPLACE,
			};
			renderstate.use();

			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);

			u_worldscreen = worldscreen * Mat4f().scale(1.25);
			m_shaders[0].use();

			renderstate.write_mask = {1, 1, 1, 1, 1};

			renderstate.stencil_func = {
			        GL_NOTEQUAL, 1, ~0u, GL_KEEP, GL_KEEP, GL_KEEP,
			        GL_NOTEQUAL, 1, ~0u, GL_KEEP, GL_KEEP, GL_KEEP,
			};
			renderstate.use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);
			spu_frame_end();
		}

		{
			u_diffuse = m_textureIds[1];
			m_shaders[1].use();
			spu_array_draw(m_arrayIds[1], GL_TRIANGLES, 0, 3);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo_depth_stencil");
}  // namespace
}  // namespace spu
