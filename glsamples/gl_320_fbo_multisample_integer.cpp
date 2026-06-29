//
// App :
//
#include "base_app.h"
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
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform bool u_is_texture;                                                             \n"
    "uniform usampler2D u_diffuse;                                                          \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if (u_is_texture) {                                                                \n"
    "        color = texture(u_diffuse, f_texcoord);                                        \n"
    "    }                                                                                  \n"
    "    else {                                                                             \n"
    "        color = vec4(255);                                                             \n"
    "    }                                                                                  \n"
    "}                                                                                      \n"
};

const char *c_vert_blit = {
    "#version 330                                                                           \n"
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
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 pseudo_int_color = texture(u_diffuse, f_texcoord);                            \n"
    "    color = pseudo_int_color / 255.0;                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[3];  // 0:diffuse 1:colorbuffer 2:multisample
	SpuShader m_shaders[2];    // 0:render 1:splash
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_is_texture;

	uint32_t m_frameId;
	uint32_t m_arrayIds[2];

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		const auto fb_scale = 4;
		const Rectf fb_viewport = Rectf(0, 0, viewport(0).sx / fb_scale, viewport(0).sy / fb_scale);
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
				int32_t i = &shader - &m_shaders[0];
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen},
				        {"u_diffuse",     &u_diffuse    },
				        {"u_is_texture",  &u_is_texture },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			}
		}

		// diffuse texture
		{
			Attrs attrs = {
			        {"iformat",     GL_RGB8UI     },
                                {"pformat",     GL_RGB_INTEGER}, // explict
			        {"base_level",  0             },
                                {"max_level",   0             },
			        {"min_filter",  GL_NEAREST    },
                                {"mag_filter",  GL_NEAREST    },
			        {"auto_mipmap", 0             },
			};
			m_textureIds[0] = loadDDS("kueken7_rgb8_unorm.dds", attrs);
		}

		// color bufer
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D},
                                {"iformat",     GL_RGBA16F   },
			        {"width",       width        },
                                {"height",      height       },
			        {"min_filter",  GL_NEAREST   },
                                {"mag_filter",  GL_NEAREST   },
			        {"auto_mipmap", 0            },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// multisample buffer
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_RGBA16F               },
			        {"width",       width                    },
			        {"height",      height                   },
			        {"multisample", 4                        },
			};
			m_textureIds[2] = spu_texture_new(attrs);
		}

		// frame buffer
		{
			// auto bgcolor0 = Vec4i(0, 128, 255, 255);
			Attrs attrs = {
			        {"color0", m_textureIds[2]},
			        {"color0.resolve", m_textureIds[1]},
			        {"viewport0", fb_viewport},
			        {"bgcolor0", Vec4f(0.0f, 0.5f, 1.0f, 1.0f)},
			};
			m_frameId = spu_frame_new(attrs);
		}

		// render array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>(Vec2f(2.0, 1.5));

			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 2                },
			        {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayIds[0] = spu_array_new(attrs);
		}

		// splash array
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

		// render FBO
		{
			MultisampleControl mcontrol(m_frameId, getGesture());

			u_diffuse = m_textureIds[0];

			u_is_texture = true;
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);

			u_is_texture = false;
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_LINE_LOOP);
		}
		// render FB
		{
			u_diffuse = m_textureIds[1];
			m_shaders[1].use();
			spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo_multisample_integer");
}  // namespace
}  // namespace spu
