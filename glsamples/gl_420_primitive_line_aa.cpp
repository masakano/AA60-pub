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
    "     vec4 gl_Position;                                                                 \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen *                                                       \n"
    "      vec4(a_position.x - 1.0 + gl_InstanceID, a_position.y, 0.0, 1.0);                \n"
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
    "        vec4(mix(vec2(-1), vec2(3), bvec2(gl_VertexID == 1, gl_VertexID == 2)), 0, 1); \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texelFetch(u_diffuse, ivec2(gl_FragCoord.xy * 1.0/16.0), 0);               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	std::vector<vec2sf_t> m_vertices;
	uint32_t m_textureIds[2];  // 0:multisample 1:color
	SpuShader m_shaders[2];
	uint32_t m_arrayIds[2];
	uint32_t m_frameId;

	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		const int32_t width = viewport(0).sx / 16;
		const int32_t height = viewport(0).sy / 16;
		const Rectf fb_viewport = Rectf(0, 0, width, height);

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
			const uint32_t count = 1000;
			const float step = M_PI * 2.0 / count;

			m_vertices.resize(count);
			for (auto i = 0u; i < count; ++i) {
				m_vertices[i] = {
				        sinf(step * i),
				        cosf(step * i),
				};
			}

			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 2                },
			        {"data",         m_vertices.data()},
			        {"nelem",        m_vertices.size()},
			};
			m_arrayIds[0] = spu_array_new(attrs);
		}

		// array splash
		{
			Attrs attrs = {
			        {"shader_id", m_shaders[1].id()},
			        {"a.0",       1                },
			        {"nelem",     6                },
			};
			m_arrayIds[1] = spu_array_new(attrs);
		}

		// texture
		{
			Attrs color_multisample_attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_RGBA8                 },
			        {"multisample", 8                        },
			        {"width",       width                    },
			        {"height",      height                   },
			        {"mag_filter",  GL_NEAREST               },
			        {"min_filter",  GL_NEAREST               },
			        {"auto_mipmap", 0                        },
			};

			Attrs color_attrs = {
			        {"target",      GL_TEXTURE_2D},
                                {"iformat",     GL_RGBA8     },
			        {"width",       width        },
                                {"height",      height       },
			        {"mag_filter",  GL_NEAREST   },
                                {"min_filter",  GL_NEAREST   },
			        {"auto_mipmap", 0            },
			};

			m_textureIds[0] = spu_texture_new(color_multisample_attrs);
			m_textureIds[1] = spu_texture_new(color_attrs);
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

		// Render multisampled texture
		spu_frame_begin(m_frameId);
		spu_frame_clear(m_frameId);

		m_shaders[0].use();
		spu_array_draw(m_arrayIds[0], GL_LINE_LOOP, 0, m_vertices.size(), 3);

		// Resolving multisampling
		spu_frame_end();

		// getRenderstate().flags.multisample = 0;
		u_diffuse = m_textureIds[1];
		m_shaders[1].use();
		spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_primitive_line_aa");
}  // namespace
}  // namespace spu
