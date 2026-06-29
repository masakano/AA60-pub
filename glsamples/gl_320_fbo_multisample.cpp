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
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if (u_is_texture) {                                                                \n"
    "        color = texture(u_diffuse, f_texcoord);                                        \n"
    "    }                                                                                  \n"
    "    else {                                                                             \n"
    "        color = vec4(1.0);                                                             \n"
    "    }                                                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[3];
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_is_texture;
	uint32_t m_frameId;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		auto fb_viewport = Rectf(0, 0, 160, 120);
		auto width = int32_t(fb_viewport.sx);
		auto height = int32_t(fb_viewport.sy);

		// shader
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			        {"u_is_texture",  &u_is_texture },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// diffuse texture
		{
			Attrs attrs = {
			        {"base_level",  0         },
                                {"max_level",   0         },
                                {"min_filter",  GL_NEAREST},
			        {"mag_filter",  GL_NEAREST},
                                {"auto_mipmap", 0         },
			};
			m_textureIds[0] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}

		// color texture
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D},
                                {"iformat",     GL_RGBA8     },
			        {"width",       width        },
                                {"height",      height       },
			        {"min_filter",  GL_NEAREST   },
                                {"mag_filter",  GL_NEAREST   },
			        {"auto_mipmap", 0            },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// resolve texture
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_RGBA8                 },
			        {"width",       width                    },
			        {"height",      height                   },
			        {"min_filter",  GL_NEAREST               },
			        {"mag_filter",  GL_NEAREST               },
			        {"multisample", 4                        },
			        {"auto_mipmap", 0                        },
			};
			m_textureIds[2] = spu_texture_new(attrs);
		}

		// frame buffer
		{
			Attrs attrs = {
			        {"color0",         m_textureIds[2]},
			        {"color0.resolve", m_textureIds[1]},
			        {"viewport0",      fb_viewport    },
			        {"bgcolor0",       c_sky          },
			};
			m_frameId = spu_frame_new(attrs);
		}

		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>(Vec2f(4.0, 3.0));

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
	}

	void render() override
	{
		// render FBO
		{
			MultisampleControl mcontrol(m_frameId, getGesture());

			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

			u_diffuse = m_textureIds[0];
			u_worldscreen = getCamera().worldscreen() * Mat4f().scale(0.3);
			u_is_texture = true;
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);

			u_is_texture = false;
			m_shader.use();
			spu_array_draw(m_arrayId, GL_LINE_LOOP);

			u_is_texture = true;  // for safety
		}
		// render FB
		{
			getCamera().setViewscreen(-4.0, 4.0, -3.0, 3.0, 0.0, 100.0, false);
			u_worldscreen = getCamera().viewscreen() * Mat4f().trans(getCamera().position() * 2.0);
			u_diffuse = m_textureIds[1];
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo_multisample");
}  // namespace
}  // namespace spu
