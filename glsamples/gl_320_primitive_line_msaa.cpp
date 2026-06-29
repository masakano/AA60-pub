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
    "    gl_Position = u_worldscreen * vec4(a_position,0.0,1.0);                             \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0, 0.5, 0.0, 1.0);                                                  \n"
    "}                                                                                      \n"
};

const char *c_vert_blit = {
    "#version 330                                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(4*(gl_VertexID%2)-1, 4*(gl_VertexID/2)-1, 0, 1);                \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 texture_size = vec2(textureSize(u_diffuse, 0));                               \n"
    "    color = texture(u_diffuse, gl_FragCoord.xy * (1.0 / 8.0) / texture_size);          \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[2];  // 0:colorbuffer 1:renderbuffer
	SpuShader m_shaders[2];    // 0:texture 1:splash
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_frameId;
	uint32_t m_arrayIds[2];

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		auto fb_viewport = Rectf(0.0f, 0.0f, viewport(0).sx / 8.0F, viewport(0).sy / 8.0F);
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
				auto i = &shader - &m_shaders[0];
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen},
				        {"u_diffuse",     &u_diffuse    },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			};
		}

		// array #0
		{
			std::array<vec2sf_t, 36> data;
			for (auto i = 0u; i < data.size(); ++i) {
				auto angle = float(M_PI * 2.0) * i / data.size();
				auto v = normalize(Vec2f(sin(angle), cos(angle)));
				data[i] = {v.x, v.y};
			}

			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 2                },
			        {"data",         data.data()      },
			        {"nelem",        36               },
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
			m_arrayIds[1] = squareQuadsArray<v2fv2f_t>(attrs);
		}

		// texture #0
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D},
                                {"iformat",     GL_RGBA8     },
                                {"width",       width        },
			        {"height",      height       },
                                {"base_level",  0            },
                                {"max_level",   0            },
			        {"min_filter",  GL_NEAREST   },
                                {"mag_filter",  GL_NEAREST   },
                                {"auto_mipmap", 0            },
			};
			m_textureIds[0] = spu_texture_new(attrs);
		}

		// texture #1
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_RGBA8                 },
			        {"width",       width                    },
			        {"height",      height                   },
			        {"multisample", 8                        },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// array
		{
			Attrs attrs = {
			        {"color0",         m_textureIds[1]},
			        {"color0.resolve", m_textureIds[0]},
			        {"viewport0",      fb_viewport    },
			        {"bgcolor0",       c_white        },
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
		spu_array_draw(m_arrayIds[0], GL_LINE_LOOP);

		spu_frame_end();
		u_diffuse = m_textureIds[0];
		m_shaders[1].use();

		spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_primitive_line_msaa");
}  // namespace
}  // namespace spu
