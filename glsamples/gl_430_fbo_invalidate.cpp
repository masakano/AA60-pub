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
    "in vec2 a_position;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen *                                                       \n"
    "     vec4(a_position.x - 1.0 + gl_InstanceID, a_position.y, 0.0, 1.0);                 \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0);                                                                 \n"
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
    "     vec4(mix(vec2(-1), vec2(3), bvec2(gl_VertexID == 1, gl_VertexID == 2)), 0, 1);    \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texelFetch(u_diffuse, ivec2(gl_FragCoord.xy * 0.125), 0);                  \n"
    "}                                                                                      \n"
};
	

	
std::vector<vec2sf_t> vertices;

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[2];  // 0:multisample 1:color
	uint32_t m_pipeIds[2];     // 0:multisample, 1:splash

	SpuShader m_shaders[2];
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_frameId;
	uint32_t m_arrayIds[2];

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		auto width = int32_t(viewport(0).sx / 8);
		auto height = int32_t(viewport(0).sy / 8);
		const auto fb_viewport = Rectf(0, 0, width, height);

		// initState();
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
		// array multisample
		{
			const uint32_t c_count(360);
			const float c_step(2.0 * M_PI / c_count);

			vertices.resize(c_count);
			for (auto i = 0u; i < c_count; ++i) {
				vertices[i] = {
				        sinf(c_step * i),
				        cosf(c_step * i),
				};
			}

			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 2                },
			        {"data",         vertices.data()  },
			        {"nelem",        vertices.size()  },
			};
			m_arrayIds[0] = spu_array_new(attrs);
		}
		// array splash
		{
			Attrs attrs = {
			        {"shader_id",    m_shaders[1].id()},
			        {"a.a_position", 1                },
			        {"nelem",        6                },
			};
			m_arrayIds[1] = spu_array_new(attrs);
		}

		// texture #0
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"width",       width                    },
			        {"height",      height                   },
			        {"multisample", 8                        },
			        {"min_filter",  GL_NEAREST               },
			        {"mag_filter",  GL_NEAREST               },
			        {"auto_mipmap", 0                        },
			};

			m_textureIds[0] = spu_texture_new(attrs);
		}

		// texture #1
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D},
                                {"width",       width        },
                                {"height",      height       },
			        {"min_filter",  GL_NEAREST   },
                                {"mag_filter",  GL_NEAREST   },
                                {"auto_mipmap", 0            },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// framebuffer
		{
			Attrs attrs = {
			        {"color0",         m_textureIds[0]},
			        {"color0.resolve", m_textureIds[1]},
			        {"viewport0",      fb_viewport    },
			        {"bgcolor0",       c_orange       },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		spu_frame_begin(m_frameId);
		spu_frame_clear(m_frameId);

		m_shaders[0].use();

		auto &renderstate = getRenderstate();
		// renderstate.flags.multisample = true;
		renderstate.use();
		spu_array_draw(m_arrayIds[0], GL_LINE_LOOP, 0, vertices.size(), 3);

		// renderstate.flags.multisample = false;
		renderstate.use();
		spu_frame_end();

		u_diffuse = m_textureIds[1];
		m_shaders[1].use();
		spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_fbo_invalidate");
}  // namespace
}  // namespace spu
