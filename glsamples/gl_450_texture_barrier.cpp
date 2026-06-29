//
// App :
//
#include "base_app.h"
#include <ssys/random_generator.h>
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
    "       vec4(a_position.xy, a_position.z + float(gl_InstanceID) * 0.5 - 0.25, 1);       \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "/*layout(binding = COLORBUFFER)*/ uniform sampler2D u_colorbuffer;                     \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord.st) * 0.75 +                                 \n"
    "            texelFetch(u_colorbuffer, ivec2(gl_FragCoord.xy), 0) * 0.25;               \n"
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
    "    gl_Position = vec4(4*(gl_VertexID%2)-1, 4*(gl_VertexID/2)-1, 0, 1);                \n"
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
	std::vector<Rectf> m_viewports;  // MUST BE FLOAT!

	SpuShader m_shaders[2];  // 0:texture 1:splash
	Mat4f u_worldscreen;
	uint32_t m_arrayIds[2];
	uint32_t m_textureIds[2];  // 0:diffuse 1:colorbuffer
	uint32_t u_diffuse;
	uint32_t u_colorbuffer;
	uint32_t m_frameId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// gl_aux_init();
		m_viewports.resize(1000);

		RandomGenerator<float> frand;

		for (auto &m_viewport: m_viewports) {
			m_viewport = {
			        frand() * 2 * viewport(0).sx - viewport(0).sx,
			        frand() * 2 * viewport(0).sy - viewport(0).sy,
			        frand() * viewport(0).sx,
			        frand() * viewport(0).sy,
			};
		}

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
				        {"colorbuffer",   &u_colorbuffer},
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

		// arry #1
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
			        {"swizzle_r",  GL_RED                   },
			        {"swizzle_g",  GL_GREEN                 },
			        {"swizzle_b",  GL_BLUE                  },
			        {"swizzle_a",  GL_ALPHA                 },
			};
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_srgb.dds", attrs);
		}

		// texture #1
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
			        {"iformat",     GL_RGBA8               },
			        {"width",       int32_t(viewport(0).sx)},
			        {"height",      int32_t(viewport(0).sy)},
			        {"min_level",   0                      },
			        {"max_level",   0                      },
			        {"min_filter",  GL_NEAREST             },
			        {"mag_filter",  GL_NEAREST             },
			        {"auto_mipmap", 0                      },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}
		// framebuffer
		{
			Attrs attrs = {
			        {"viewport0", viewport(0)    },
			        {"color0",    m_textureIds[1]},
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		auto color = Vec4f(1.0, 0.5, 0.0, 1.0);

		spu_texture_send(m_textureIds[1], color.f, GL_RGBA32F, nullptr, nullptr, true);
		spu_frame_begin(m_frameId);

		u_diffuse = m_textureIds[0];
		u_colorbuffer = m_textureIds[1];

		// Bind rendering objects
		m_shaders[0].use();

		for (auto &m_viewport: m_viewports) {
			spu_frame_set(m_frameId, "viewport0", m_viewport);
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);
		}

		spu_frame_end();
		u_diffuse = m_textureIds[1];
		m_shaders[1].use();
		spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_450_texture_barrier");
}  // namespace
}  // namespace spu
