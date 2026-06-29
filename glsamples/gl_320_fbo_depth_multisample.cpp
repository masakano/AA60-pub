//
// App :
//
#include "base_app.h"
#include "gl_320_texture_2d.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert_blit = {
    "#version 330                                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(4*(gl_VertexID%2)-1, 4*(gl_VertexID/2)-1, 0, 1);                \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 330                                                                           \n"
    "uniform sampler2DMS u_diffuse;                                                         \n"
    "out vec4 color;                                                                        \n"
    "float LinearizeDepth(float depth)                                                      \n"
    "{                                                                                      \n"
    "    float n = 0.1; // camera z near                                                    \n"
    "    float f = 8.0; // camera z far                                                     \n"
    "    return (2.0 * n) / (f + n - depth * (f - n));                                      \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    float depth = texelFetch(u_diffuse, ivec2(gl_FragCoord.xy), 0).r;                  \n"
    "    for(int i = 1; i < 4; ++i)                                                         \n"
    "        depth = min(depth, texelFetch(u_diffuse, ivec2(gl_FragCoord.xy), i).r);        \n"
    "    float linear_depth = LinearizeDepth(depth);                                        \n"
    "    color = vec4(linear_depth, linear_depth, linear_depth, 1.0);                       \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[2];
	SpuShader m_shaders[2];

	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	uint32_t m_frameId;
	uint32_t m_arrayIds[2];

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// shader
		{
			const char *verts[2] = {c_vert, c_vert_blit};
			const char *frags[2] = {c_frag, c_frag_blit};

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
			        {"a.0",   1},
                                {"nelem", 3}, // one triangle
			};
			m_arrayIds[1] = spu_array_new(attrs);
		}

		// texture #0
		{
			Attrs attrs = {
			        {"min_filter", GL_NEAREST_MIPMAP_NEAREST},
			        {"mag_filter", GL_NEAREST               },
			        {"wrap_s",     GL_CLAMP_TO_EDGE         },
			        {"wrap_t",     GL_CLAMP_TO_EDGE         },
			};
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_srgb.dds", attrs);
		}

		// testure #1
		{
			Attrs attrs = {

			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_DEPTH_COMPONENT24     },
			        {"multisample", 4                        },
			        {"width",       int32_t(viewport(0).sx)  },
			        {"height",      int32_t(viewport(0).sy)  },
			        {"base_level",  0                        },
			        {"max_level",   0                        },
			        {"auto_mipmap", 0                        },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// frame buffer
		{
			Attrs attrs = {
			        {"viewport0", viewport(0)    },
			        {"depth",     m_textureIds[1]},
			        {"bgcolor0",  Vec4f(-1)      },
			        {"bgdepth",   1.0            },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 8.0);
		auto model = Mat4f().scale(5.0);

		u_worldscreen = getCamera().worldscreen() * model;

		{
			MultisampleControl mcontrol(m_frameId, getGesture());

			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.depth_func = GL_LESS;
			renderstate.use();

			u_diffuse = m_textureIds[0];
			m_shaders[0].use();

			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);  // with index
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = false;
			renderstate.use();

			u_diffuse = m_textureIds[1];
			m_shaders[1].use();
			spu_array_draw(m_arrayIds[1], GL_TRIANGLES, 0, 3);  // one triangle only
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo_depth_multisample");
}  // namespace
}  // namespace spu
