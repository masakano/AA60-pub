//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

#define e_fb_scale "2"  // string

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
    "    color = texture(u_diffuse, f_texcoord);                                            \n"
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
        "    color = texture(u_diffuse, gl_FragCoord.xy * " e_fb_scale "/ texture_size);"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[2];  // 0:diffuse 1:colorbuffer
	SpuShader m_shaders[2];    // 0:texture 1:splash

	uint32_t m_arrayIds[2];
	uint32_t m_frameId;

	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	int32_t m_paintCount = 0;

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
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_CLAMP_TO_EDGE       },
			        {"wrap_t",     GL_CLAMP_TO_EDGE       },
			};
			m_textureIds[0] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}

		// texture #1
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D},
                                {"iformat",     GL_RGB10     }, // stress test
			        {"width",       width        },
                                {"height",      height       },
                                {"base_level",  0            },
			        {"max_level",   0            },
                                {"min_filter",  GL_LINEAR    },
                                {"mag_filter",  GL_LINEAR    },
			        {"auto_mipmap", 0            },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// frame buffer
		{
			Attrs attrs = {
			        {"color0",    m_textureIds[1]},
			        {"viewport0", fb_viewport    },
			        {"bgcolor0",  c_orange       },
			};
			m_frameId = spu_frame_new(attrs);
		}

		// renderstate
		{
			auto &renderstate = getRenderstate();
			renderstate.blend_func = {
			        GL_CONSTANT_ALPHA,
			        GL_ONE_MINUS_CONSTANT_ALPHA,
			        GL_CONSTANT_ALPHA,
			        GL_ONE_MINUS_CONSTANT_ALPHA,
			};
			renderstate.blend_color = {1.0, 1.0, 1.0, 0.01};
			renderstate.use();
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		spu_frame_begin(m_frameId);
		spu_frame_clear(m_frameId);
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.blend = true;
			renderstate.use();
			m_paintCount = (m_paintCount + 1) % 180;
			u_diffuse = m_textureIds[0];
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES, 0, 0, m_paintCount);
			spu_printf(0, "paint count = %d\n", m_paintCount);
		}
		spu_frame_end();

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

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo_blend");
}  // namespace
}  // namespace spu
