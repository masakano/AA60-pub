//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "in vec2 a_position;                                                                    \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_geom = {
    "#version 330                                                                           \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 12) out;                                         \n"
    "flat out int f_instance;                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int layer = 0; layer < 4; ++layer)                                             \n"
    "    {                                                                                  \n"
    "        gl_Layer = layer;                                                              \n"
    "        for(int i = 0; i < gl_in.length(); ++i)                                        \n"
    "        {                                                                              \n"
    "            gl_Position = gl_in[i].gl_Position;                                        \n"
    "            f_instance = layer;                                                        \n"
    "            EmitVertex();                                                              \n"
    "        }                                                                              \n"
    "        EndPrimitive();                                                                \n"
    "    }                                                                                  \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "const vec4 colors[4] = vec4[]                                                          \n"
    "(                                                                                      \n"
    "    vec4(1.0, 0.0, 0.0, 1.0),                                                          \n"
    "    vec4(1.0, 1.0, 0.0, 1.0),                                                          \n"
    "    vec4(0.0, 1.0, 0.0, 1.0),                                                          \n"
    "    vec4(0.0, 0.0, 1.0, 1.0)                                                           \n"
    ");                                                                                     \n"
    "flat in int f_instance;                                                                \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = colors[f_instance];                                                        \n"
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
	std::vector<Rectf> m_viewports;
	SpuShader m_shaders[2];  // 0:layering, 1:splash
	uint32_t m_arrayIds[2];
	uint32_t m_frameId;

	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_layer;

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		auto fb_viewport = Rectf(0, 0, 320, 240);
		auto width = int32_t(fb_viewport.sx);
		auto height = int32_t(fb_viewport.sy);

		// 4x4 viewport
		m_viewports = makeViewports(2, 2, 6);

		// shaders
		{
			const char *verts[2] = {
			        c_vert,
			        c_vert_blit,
			};

			const char *geoms[2] = {
			        c_geom,
			        nullptr,  // no geometry shader
			};
			const char *frags[2] = {
			        c_frag,
			        c_frag_blit,
			};

			for (auto &shader: m_shaders) {
				int32_t i = &shader - &m_shaders[0];
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"geom", geoms[i]},
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
			        {"target",      GL_TEXTURE_2D_ARRAY},
			        {"iformat",     GL_RGB8            },
			        {"width",       width              },
			        {"height",      height             },
			        {"depth",       4                  },
			        {"base_level",  0                  },
			        {"max_level",   0                  },
			        {"min_filter",  GL_NEAREST         },
			        {"mag_filter",  GL_NEAREST         },
			        {"auto_mipmap", 0                  },
			};
			u_diffuse = spu_texture_new(attrs);
		}
		// frame
		{
			Attrs attrs = {
			        {"color0",    u_diffuse  },
			        {"viewport0", fb_viewport},
			};
			m_frameId = spu_frame_new(attrs);
		}

		// layering array
		{
			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 2                },
			        {"a.a_texcoord", 2                },
			};
			m_arrayIds[0] = squareQuadsArray<v2fv2f_t>(attrs);
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
			spu_array_link(m_arrayIds[1], m_arrayIds[0], -1, -1);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(-1.0, 1.0, 1.0, -1.0, 1.0, -1.0, false);
		u_worldscreen = getCamera().viewscreen();

		// Pass 1
		{
			spu_frame_begin(m_frameId);
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);
			spu_frame_end();
		}

		// Pass 2
		{
			for (auto i = 0; i < 4; ++i) {
				u_layer = i;
				m_shaders[1].use();

				spu_frame_set(-1, "viewport0", m_viewports[i]);

				spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo_layered");
}  // namespace
}  // namespace spu
