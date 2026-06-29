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
    "out vec2 vertTexcoord;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vertTexcoord = a_texcoord;                                                         \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "uniform bool u_use_white;                                                              \n"
    "in vec2 vertTexcoord;                                                                  \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if (u_use_white) {                                                                 \n"
    "        color = vec4(1.0);                                                             \n"
    "    }                                                                                  \n"
    "    else {                                                                             \n"
    "        color = texture(u_diffuse, vertTexcoord);                                      \n"
    "    }                                                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_use_white;

	uint32_t m_textureId;
	uint32_t m_colorMultiId;
	uint32_t m_colorId;
	uint32_t m_frameId;
	uint32_t m_arrayId;

	explicit App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		auto fb_viewport = Rectf(0, 0, 80, 60);
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
			        {"u_use_white",   &u_use_white  },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// texture
		{
			Attrs attrs = {
			        {"min_filter", GL_LINEAR},
			        {"mag_filter", GL_LINEAR},
			};
			m_textureId = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}

		// color buffer
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
			m_colorId = spu_texture_new(attrs);
		}

		// resolve color buffer
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_RGBA8                 },
			        {"multisample", 4                        },
			        {"width",       width                    },
			        {"height",      height                   },
			};
			m_colorMultiId = spu_texture_new(attrs);
		}

		// frame buffer
		{
			Attrs attrs = {
			        {"viewport0",      fb_viewport   },
			        {"color0",         m_colorMultiId},
			        {"color0.resolve", m_colorId     },
			        {"bgcolor0",       c_sky         },
			};
			m_frameId = spu_frame_new(attrs);
		}

		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>(Vec2f(2.0, 1.5));

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
		auto &camera = getCamera();

		// render FBO
		{
			MultisampleControl mcontrol(m_frameId, getGesture());

			camera.setViewscreen(viewport(0), 45, 0.1, 100.0);
			u_worldscreen = camera.worldscreen();
			u_diffuse = m_textureId;
			u_use_white = false;
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);

			u_use_white = true;
			m_shader.use();
			spu_array_draw(m_arrayId, GL_LINE_LOOP);
		}
		// render FB
		{
			camera.setViewscreen(viewport(0), 45, 0.1, 100.0);
			u_worldscreen = camera.worldscreen();
			u_diffuse = m_colorId;
			u_use_white = false;
			m_shader.use();
			// printf("----------- shader ----------\n");
			// m_shader.report("shader");
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_300_fbo_multisample");

}  // namespace
}  // namespace spu
