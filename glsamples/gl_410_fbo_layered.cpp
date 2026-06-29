//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 410 core                                                                      \n"
    "in vec2 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(a_position, 0.0, 1.0);                                          \n"
    "}                                                                                      \n"
};
const char *c_geom = {
    "#version 410 core                                                                      \n"
    "layout(triangles, invocations = 4) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 3) out;                                          \n"
    "in gl_PerVertex                                                                        \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "} gl_in[];                                                                             \n"
    "flat out int f_instance;                                                               \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        gl_Position = u_worldscreen * gl_in[i].gl_Position;                             \n"
    "        gl_Layer = gl_InvocationID;                                                    \n"
    "        f_instance = gl_InvocationID;                                                  \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};
const char *c_frag = {
    "#version 410 core                                                                      \n"
    "const vec4 colors[4] = vec4[]                                                          \n"
    "(                                                                                      \n"
    "    vec4(1.0, 0.0, 0.0, 1.0),                                                          \n"
    "    vec4(1.0, 1.0, 0.0, 1.0),                                                          \n"
    "    vec4(0.0, 1.0, 0.0, 1.0),                                                          \n"
    "    vec4(0.0, 0.0, 1.0, 1.0)                                                           \n"
    ");                                                                                     \n"
    "flat in int f_instance;                                                                \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = colors[f_instance];                                                        \n"
    "}                                                                                      \n"
};
const char *c_vert_blit = {
    "#version 410 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 g_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    g_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};
const char *c_geom_blit = {
    "#version 410 core                                                                      \n"
    "layout(triangles, invocations = 4) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 3) out;                                          \n"
    "in gl_PerVertex                                                                        \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "} gl_in[];                                                                             \n"
    "in vec2 g_texcoord[];                                                                  \n"
    "out vec2 f_texcoord;                                                                   \n"
    "flat out int f_instance;                                                               \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        gl_Layer = gl_InvocationID;                                                    \n"
    "        gl_ViewportIndex = gl_InvocationID;                                            \n"
    "        gl_Position = u_worldscreen * gl_in[i].gl_Position;                             \n"
    "        f_instance = gl_InvocationID;                                                  \n"
    "        f_texcoord = g_texcoord[i];                                                    \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 410 core                                                                      \n"
    "uniform sampler2DArray u_diffuse;                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "flat in int f_instance;                                                                \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, vec3(f_texcoord, f_instance));                          \n"
    "}                                                                                      \n"
};

const auto c_fb_viewport = Rectf(0, 0, 640, 480);
const auto c_width = int32_t(c_fb_viewport.sx);
const auto c_height = int32_t(c_fb_viewport.sy);

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shaders[2];  // 0:layering 1:viewport
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_sampler;

	uint32_t m_arrayIds[2];

	uint32_t m_frameId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			const char *verts[] = {
			        c_vert,
			        c_vert_blit,
			};
			const char *geoms[] = {
			        c_geom,
			        c_geom_blit,
			};
			const char *frags[] = {
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
				        {"u_diffuse",     &u_sampler    },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			}
		}
		// array
		{
			for (auto i = 0; i < 2; i++) {
				Attrs attrs = {
				        {"shader_id",    m_shaders[i].id()},
				        {"a.a_position", 2                },
				        {"a.a_texcoord", 2                },
				};
				m_arrayIds[i] = squareQuadsArray<v2fv2f_t>(attrs);
			}
		}
		// texture
		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D_ARRAY},
			        {"iformat",    GL_RGB8            },
			        {"width",      c_width            },
			        {"width",      c_width            },
			        {"height",     c_height           },
			        {"depth",      4                  },
			        {"base_level", 0                  },
			        {"max_level",  100                },
			};
			u_diffuse = spu_texture_new(attrs);
		}
		// framebuffer
		{
			Attrs attrs = {
			        {"color0",    u_diffuse    },
			        {"viewport0", c_fb_viewport},
			};
			m_frameId = spu_frame_new(attrs);
		}
		// sampler
		{
			float min_lod = -1000;
			float max_lod = 1000;
			float lod_bias = 0;

			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER},
                                {"wrap_s",       GL_CLAMP_TO_EDGE  },
			        {"wrap_t",       GL_CLAMP_TO_EDGE  },
                                {"wrap_r",       GL_CLAMP_TO_EDGE  },
			        {"min_filter",   GL_NEAREST        },
                                {"mag_filter",   GL_NEAREST        },
			        {"min_lod",      min_lod           },
                                {"max_lod",      max_lod           },
			        {"lod_bias",     lod_bias          },
                                {"compare_mode", GL_NONE           },
			        {"compare_func", GL_LEQUAL         },
			};
			u_sampler = spu_texture_new(attrs);
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
			std::vector<Rectf> m_viewports = makeViewports(2, 2, 2);

			Attrs attrs = {
			        {"viewport0", m_viewports[0]},
			        {"viewport1", m_viewports[1]},
			        {"viewport2", m_viewports[2]},
			        {"viewport3", m_viewports[3]},
			};
			spu_frame_set(-1, attrs);
			m_shaders[1].use();
			spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_410_fbo_layered");
}  // namespace
}  // namespace spu
