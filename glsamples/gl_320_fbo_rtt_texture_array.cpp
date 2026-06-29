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
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "layout (location = 0) out vec4 red;                                                    \n"
    "layout (location = 1) out vec4 green;                                                  \n"
    "layout (location = 2) out vec4 blue;                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    red = vec4(1.0, 0.0, 0.0, 1.0);                                                    \n"
    "    green = vec4(0.0, 1.0, 0.0, 1.0);                                                  \n"
    "    blue = vec4(0.0, 0.0, 1.0, 1.0);                                                   \n"
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
    "uniform sampler2DArray u_diffuse;                                                      \n"
    "uniform int u_layer;                                                                   \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, vec3(f_texcoord, float(u_layer)));                      \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shaders[2];
	std::vector<Rectf> m_viewports;

	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_layer;
	uint32_t m_frameId;
	uint32_t m_arrayIds[2];

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 2);

		// program
		{
			const char *verts[] = {
			        c_vert,
			        c_vert_blit,
			};

			const char *frags[] = {c_frag, c_frag_blit};

			for (auto &shader: m_shaders) {
				int32_t i = &shader - &m_shaders[0];
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen},
				        {"u_diffuse",     &u_diffuse    },
				        {"u_layer",       &u_layer      },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			}
		}

		// texture
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_ARRAY        },
			        {"iformat",     GL_BGR                     },
			        {"width",       int32_t(viewport(0).sx) / 2},
			        {"height",      int32_t(viewport(0).sy) / 2},
			        {"depth",       3                          },
			        {"base_level",  0                          },
			        {"max_level",   0                          },
			        {"min_filter",  GL_NEAREST                 },
			        {"mag_filter",  GL_NEAREST                 },
			        {"auto_mipmap", 0                          },
			};
			u_diffuse = spu_texture_new(attrs);
		}

		// frame buffer
		{
			auto half_viewport = Rectf(0, 0, viewport(0).sx / 2, viewport(0).sy / 2);

			Attrs attrs = {
			        {"color0",       u_diffuse    },
                                {"color0.layer", 0            },
                                {"color1",       u_diffuse    },
			        {"color1.layer", 1            },
                                {"color2",       u_diffuse    },
                                {"color2.layer", 2            },
			        {"viewport0",    half_viewport},
                                {"bgcolor0",     c_orange     },
			};
			m_frameId = spu_frame_new(attrs);
		}

		// array #0
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>();

			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 2                },
			        {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayIds[0] = spu_array_new(attrs);
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
		// Pass 1
		{
			getCamera().setViewscreen(-1.0, 1.0, -1.0, 1.0, -1.0, +1.0, false);
			u_worldscreen = getCamera().viewscreen();
			m_shaders[0].use();

			spu_frame_begin(m_frameId);
			spu_frame_clear(m_frameId);
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);
			spu_frame_end();
		}

		// Pass 2
		{
			getCamera().setViewscreen(-1.0, 1.0, 1.0, -1.0, -1.0, +1.0, false);
			u_worldscreen = getCamera().viewscreen();
		}

		for (auto i = 0u; i < 3; ++i) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			u_layer = i;
			m_shaders[1].use();
			spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo_rtt_texture_array");
}  // namespace
}  // namespace spu
