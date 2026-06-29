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
    "uniform mat4 u_worldscreen;                                                            \n"
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
    "    int ox =gl_InstanceID/16;                                                          \n"
    "    int oy =gl_InstanceID%16;                                                          \n"
    "    vec4 offset = vec4(ox * 4, oy * 4, 0, 0);                                          \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0) + offset;                 \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = textureLod(u_diffuse, f_texcoord, 0);                                      \n"
    "    //color = vec4(f_texcoord, 1, 1);                                                  \n"
    "}                                                                                      \n"
};

const char *c_vert_blit = {
    "#version 150 core                                                                      \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(4*(gl_VertexID%2)-1, 4*(gl_VertexID/2)-1, 0, 1);                \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 150 core                                                                      \n"
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
	uint32_t m_textureIds[4];  // 0:diffuse 1:colorbuffer 2:depthbuffer 3:stencilbuffer
	SpuShader m_shaders[2];    // 0:texture 1:splash
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_arrayIds[2];
	uint32_t m_frameId;

	App(const char *name) : BaseApp(name, false) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		float fb_scale = float(atoi(e_fb_scale));
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

			for (auto i = 0; i < 2; i++) {
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen}, // UBO
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
			        {"nelem", 3},
			};
			m_arrayIds[1] = spu_array_new(attrs);
		}

		// texture #0
		{
			Attrs attrs = {
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			};
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_unorm.dds", attrs);
		}

		// texture #1
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D},
                                {"iformat",     GL_RGBA8     },
                                {"width",       width        },
			        {"height",      height       },
                                {"min_filter",  GL_LINEAR    },
                                {"mag_filter",  GL_LINEAR    },
			        {"base_level",  0            },
                                {"max_level",   0            },
                                {"auto_mipmap", 0            },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// texture #2
		{
			Attrs attrs = {
			        {"target",  GL_RENDERBUFFER     },
			        {"iformat", GL_DEPTH32F_STENCIL8},
			        {"width",   width               },
			        {"height",  height              },
			};
			m_textureIds[2] = spu_texture_new(attrs);
		}

		// texture #3
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D    },
                                {"iformat",     GL_STENCIL_INDEX8},
			        {"width",       width            },
                                {"height",      height           },
			        {"base_level",  0                },
                                {"max_level",   0                },
			        {"auto_mipmap", 0                },
			};
			m_textureIds[3] = spu_texture_new(attrs);

			// read write test
			bool use_clearimage = false;
			std::vector<uint8_t> pixels0(
			        int32_t(fb_viewport.sx) * int32_t(fb_viewport.sy), rand() % 0xff);
			std::vector<uint8_t> pixels1(int32_t(fb_viewport.sx) * int32_t(fb_viewport.sy), 0xaa);

			if (!use_clearimage) {
				for (auto &pix: pixels0) {
					pix = rand() % 0xff;
				}
			}

			if (use_clearimage) {
				spu_texture_send(
				        m_textureIds[3], pixels0.data(), GL_STENCIL_INDEX8, nullptr, nullptr,
				        true);
			}
			else {
				spu_texture_send(m_textureIds[3], pixels0.data(), GL_STENCIL_INDEX8);
			}

			spu_texture_recv(m_textureIds[3], pixels1.data(), GL_STENCIL_INDEX8);
			assert(memcmp(pixels0.data(), pixels1.data(), pixels0.size()) == 0);
		}

		// frame buffer
		{
			// not work
			Attrs attrs = {
			        {"color0",    m_textureIds[1]},
#if 0				
                                {"depth",     m_textureIds[2]},// cannot use!
#else
			        {"stencil",   m_textureIds[3]},
#endif
			        {"viewport0", fb_viewport    },
			        {"bgcolor0",  c_orange       },
			        {"bgdepth",   0.0            },
			        {"bgstencil", 0              },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		spu_frame_begin(m_frameId);
		spu_frame_clear(m_frameId);
		u_diffuse = m_textureIds[0];

		// draw stencil
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.stencil_test = true;

			renderstate.stencil_func = {
			        GL_ALWAYS, 0x1, 0xff, GL_KEEP, GL_KEEP, GL_REPLACE,
			        GL_ALWAYS, 0x1, 0xff, GL_KEEP, GL_KEEP, GL_REPLACE,
			};
			renderstate.write_mask = {
			        0, 0, 0, 1, 1,
			};
			renderstate.use();

			auto worldview = getCamera().worldview().trans({0, 0, 3});
			auto viewscreen = getCamera().viewscreen();
			u_worldscreen = viewscreen * worldview;
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES, 0, 0, 1);
		}

		// draw rectangle
		{
			auto &renderstate = getRenderstate();
			renderstate.stencil_func = {
			        GL_EQUAL, 0x1, 0xff, GL_KEEP, GL_KEEP, GL_REPLACE,
			        GL_EQUAL, 0x1, 0xff, GL_KEEP, GL_KEEP, GL_REPLACE,
			};
			renderstate.write_mask = {
			        1, 1, 1, 1, 1,
			};

			renderstate.use();
			auto worldview = getCamera().worldview().trans({-16, -16, -16});
			auto viewscreen = getCamera().viewscreen();

			u_worldscreen = viewscreen * worldview;
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES, 0, 0, 256);
		}
		spu_frame_end();

		// blit
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.stencil_test = false;
			renderstate.use();

			u_diffuse = m_textureIds[1];
			m_shaders[1].use();
			spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_440_fbo_depth_stencil");
}  // namespace
}  // namespace spu
