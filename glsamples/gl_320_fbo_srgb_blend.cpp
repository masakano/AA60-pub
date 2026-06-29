//
// App :
//
#include "base_app.h"
// #include <spu/config.h>

namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position,0.0,1.0);                             \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord) * vec4(1.0, 1.0, 1.0, 0.02);                \n"
    "}                                                                                      \n"
};

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
    "    color = texture(u_diffuse, gl_FragCoord.xy  / texture_size);                       \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[3];  // 0:diffuse 1:colorbuffer 2:renderbuffer
	SpuShader m_shaders[2];    // 0:texture 1:splash

	uint32_t m_framebufferName;

	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_frameId;
	uint32_t m_arrayIds[2];

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		auto fb_viewport = Rectf(0, 0, viewport(0).sx, viewport(0).sy);

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

		// texture #0
		{
			Attrs attrs = {
			        {"iformat",    GL_SRGB8_ALPHA8        },
                                {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
                                {"wrap_s",     GL_CLAMP_TO_EDGE       },
			        {"wrap_t",     GL_CLAMP_TO_EDGE       },
			};
			m_textureIds[0] = loadDDS("kueken7_rgba8_srgb.dds", attrs);
		}

		// texture #1
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
			        {"iformat",     GL_SRGB8_ALPHA8        },
			        {"width",       int32_t(viewport(0).sx)},
			        {"height",      int32_t(viewport(0).sy)},
			        {"base_level",  0                      },
			        {"max_level",   0                      },
			        {"min_filter",  GL_LINEAR              },
			        {"mag_filter",  GL_LINEAR              },
			        {"auto_mipmap", 0                      },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// frame buffer
		{
			Attrs attrs = {
			        {"color0",    m_textureIds[1]},
			        {"viewport0", fb_viewport    },
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
		}
	}

	void render() override
	{
		{
			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
			u_worldscreen = getCamera().worldscreen();
		}

		// Render a textured quad to a sRGB framebuffer object.
		{
			// spu_frame_set(m_frameId, "srgb", 1);

			spu_frame_begin(m_frameId);
			spu_frame_clear(m_frameId);

			auto &renderstate = getRenderstate();
			renderstate.flags.blend = true;
			renderstate.blend_func = {
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			};
			renderstate.use();

			u_diffuse = m_textureIds[0];
			m_shaders[0].use();

			static auto n_repeat = 4;
			if (++n_repeat > 400) {
				n_repeat = 4;
			}
			spu_printf(0, "alpha add %d times\n", n_repeat / 4);
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES, 0, 0, n_repeat / 4);
			spu_frame_end();
		}

		// Blit the sRGB framebuffer to the default framebuffer back buffer.
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.blend = false;
			renderstate.use();

			u_diffuse = m_textureIds[1];
			m_shaders[1].use();

			spu_array_draw(m_arrayIds[1], GL_TRIANGLES, 0, 3);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo_srgb_blend");
}  // namespace
}  // namespace spu
