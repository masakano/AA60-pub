//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 400 core                                                                      \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(a_position, 0.0, 1.0);                                          \n"
    "}                                                                                      \n"
};

const char *c_geom = {
    "#version 400 core                                                                      \n"
    "layout(triangles, invocations = 4) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 3) out;                                          \n"
    "flat out int geomInstance;                                                             \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Layer = gl_InvocationID;                                                        \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        gl_Position = u_worldscreen * gl_in[i].gl_Position;                             \n"
    "        geomInstance = gl_InvocationID;                                                \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
    "const vec4 colors[4] = vec4[]                                                          \n"
    "(                                                                                      \n"
    "    vec4(1.0, 0.0, 0.0, 1.0),                                                          \n"
    "    vec4(1.0, 1.0, 0.0, 1.0),                                                          \n"
    "    vec4(0.0, 1.0, 0.0, 1.0),                                                          \n"
    "    vec4(0.0, 0.0, 1.0, 1.0)                                                           \n"
    ");                                                                                     \n"
    "flat in int geomInstance;                                                              \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = colors[geomInstance];                                                      \n"
    "}                                                                                      \n"
};

const char *c_vert_blit = {
    "#version 400 core                                                                      \n"
    "const int vertexCount = 3;                                                             \n"
    "const vec2 positions[vertexCount] = vec2[](                                            \n"
    "    vec2(-1.0,-1.0),                                                                   \n"
    "    vec2( 3.0,-1.0),                                                                   \n"
    "    vec2(-1.0, 3.0));                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);                              \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 400 core                                                                      \n"
    "uniform sampler2DArray u_diffuse;                                                      \n"
    "uniform int u_layer;                                                                   \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 texture_size = vec2(textureSize(u_diffuse, 0).xy);                            \n"
    "    color = texture(u_diffuse, vec3(gl_FragCoord.xy / texture_size, u_layer));         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_samplerId = 0;

	SpuShader m_shaders[2];  // 0:layering 1:image2D
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_sampler;
	uint32_t u_layer;

	std::vector<Rectf> m_viewports;

	uint32_t m_arrayIds[2];
	uint32_t m_frameId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 2, 2);

		// program
		{
			const char *verts[] = {
			        c_vert,
			        c_vert_blit,
			};
			const char *geoms[] = {
			        c_geom,
			        nullptr,
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
				        {"u_layer",       &u_layer      },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			}
		}
		// array
		{
			// layering
			{
				Attrs attrs = {
				        {"shader_id",    m_shaders[0].id()},
				        {"a.a_position", 2                },
				        {"a.a_texcoord", 2                },
				};
				m_arrayIds[0] = squareQuadsArray<v2fv2f_t>(attrs);
			}

			// image2D
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
		// texture
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_ARRAY        },
			        {"base_level",  0                          },
			        {"max_level",   0                          },
			        {"auto_mipmap", 0                          },
			        {"min_filter",  GL_NEAREST                 },
			        {"mag_filter",  GL_NEAREST                 },
			        {"iformat",     GL_RGB8                    },
			        {"width",       int32_t(viewport(0).sx) / 2},
			        {"height",      int32_t(viewport(0).sy) / 2},
			        {"depth",       4                          },
			};
			u_diffuse = spu_texture_new(attrs);
		}
		// framebuffer
		{
			Attrs attrs = {
			        {"color0",    u_diffuse  },
			        {"viewport0", viewport(0)},
			};
			m_frameId = spu_frame_new(attrs);
		}
		// sampler
		{
			auto min_lod = -1000.0f;
			auto max_lod = 1000.0f;
			auto lod_bias = 0.0f;
			auto max_aniso = 16.0f;
			auto border = Vec4f(0.5, 0.5, 0.5, 0);

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
                                {"border",       border            },
			        {"max_aniso",    max_aniso         },
			};
			m_samplerId = spu_texture_new(attrs);
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
			u_sampler = m_samplerId;

			for (auto i = 0; i < 4; ++i) {
				u_layer = i;
				m_shaders[1].use();
				spu_frame_set(-1, "viewport0", m_viewports[i]);
				spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_fbo_layered");
}  // namespace
}  // namespace spu
