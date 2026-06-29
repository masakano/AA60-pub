//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec3 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen *                                                       \n"
    "     vec4(a_position.xy, a_position.z + float(gl_InstanceID) * 0.5 - 0.25, 1.0);       \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "#    ifdef FLAT_COLOR                                                                  \n"
    "        color = vec4(0.0, 0.5, 1.0, 1.0);                                              \n"
    "#    else                                                                              \n"
    "        color = texture(u_diffuse, f_texcoord.st);                                     \n"
    "#endif//                                                                               \n"
    "}                                                                                      \n"
};

const char *c_vert_blit = {
    "#version 420 core                                                                      \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position =                                                                      \n"
    "        vec4(4*(gl_VertexID%2)-1, 4*(gl_VertexID/2)-1, 0, 1);                          \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 texture_size = vec2(textureSize(u_diffuse, 0));                               \n"
    "    color = texture(u_diffuse, gl_FragCoord.xy / texture_size);                        \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[3];  // 0:diffuse 1:colorbuffer 2:renderbuffer
	SpuShader m_shaders[2];    // 0:texture 1:splash
	uint32_t m_frameId;
	uint32_t m_arrayIds[2];

	Mat4f u_worldscreen;
	uint32_t u_diffuse;

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
			        {"shader_id", m_shaders[1].id()},
			        {"a.0",       1                },
			        {"data",      nullptr          }, // this is legal
			        {"nelem",     4                }
                        };
			m_arrayIds[1] = spu_array_new(attrs);
		}

		// texture #3
		{
			Attrs attrs = {
			        {"min_filter", GL_NEAREST_MIPMAP_NEAREST},
			        {"mag_filter", GL_NEAREST               },
			};
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_unorm.dds", attrs);
		}

		// texture #1
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
			        {"iformat",     GL_RGBA8               },
			        {"width",       int32_t(viewport(0).sx)},
			        {"height",      int32_t(viewport(0).sy)},
			        {"base_level",  0                      },
			        {"max_level",   0                      },
			        {"auto_mipmap", 0                      },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// texture #2
		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D          },
			        {"iformat",    GL_DEPTH_COMPONENT24   },
			        {"width",      int32_t(viewport(0).sx)},
			        {"height",     int32_t(viewport(0).sy)},
			        {"base_level", 0                      },
			        {"max_level",  0                      },
			};
			m_textureIds[2] = spu_texture_new(attrs);
		}

		// framebuffer
		{
			Attrs attrs = {
			        {"color0",    m_textureIds[1]},
			        {"depth",     m_textureIds[2]},
			        {"viewport0", viewport(0)    },
			        {"bgcolor0",  c_orange       },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LESS;
		renderstate.use();

		spu_frame_begin(m_frameId);
		spu_frame_clear(m_frameId);

		u_diffuse = m_textureIds[0];
		m_shaders[0].use();
		spu_array_draw(m_arrayIds[0], GL_TRIANGLES);

		renderstate.flags.depth_test = false;
		renderstate.use();
		spu_frame_end();

		u_diffuse = m_textureIds[1];

		m_shaders[1].use();
		spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_fbo");
}  // namespace
}  // namespace spu
