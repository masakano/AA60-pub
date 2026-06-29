//
// App :
//
#include "base_app.h"
namespace spu {
namespace {
/* clang-format off */
const char *c_vert = {
    "#version 330 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, float(gl_InstanceID) * 1.0 - 2.0, 1.0);\n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord);                                            \n"
    "}                                                                                      \n"
};

const char *c_frag_box = {
    "#version 330 core                                                                      \n"
    "#extension GL_NV_explicit_multisample : require                                        \n"
    "uniform sampler2DMS u_diffuse;                                                         \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    // integer UV coordinates, needed for fetching multisampled texture                \n"
    "    ivec2 texcoord = ivec2(textureSize(u_diffuse) * f_texcoord);                       \n"
    "    vec4 temp = vec4(0.0);                                                             \n"
    "    // For each of the 4 samples                                                       \n"
    "    for(int i = 0; i < 4; ++i)                                                         \n"
    "        temp += texelFetch(u_diffuse, texcoord, i);                                    \n"
    "    color = temp * 0.25;                                                               \n"
    "}                                                                                      \n"
};

const char *c_frag_near = {
    "#version 330 core                                                                      \n"
    "#extension GL_NV_explicit_multisample : require                                        \n"
    "uniform sampler2DMS u_diffuse;                                                         \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    // integer UV coordinates, needed for fetching multisampled texture                \n"
    "    ivec2 texcoord = ivec2(textureSize(u_diffuse) * f_texcoord);                       \n"
    "    color = texelFetch(u_diffuse, texcoord, 0);                                        \n"
    "}                                                                                      \n"
};

const auto c_fb_viewport = Rectf(0, 0, 160, 120);
//const auto width = int32_t(c_fb_viewport.sx);
//const auto height = int32_t(c_fb_viewport.sy);

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shaders[3];  // 0:through 1:resolve box 2:resolve near
	Mat4f u_worldscreen0;
	Mat4f u_worldscreen1;

	uint32_t m_arrayId;
	uint32_t m_textureId;
	uint32_t m_samplerId;
	uint32_t m_frameId;
	uint32_t m_renderId[2];  // 0:color 1:depth

	uint32_t u_diffuse;

	App(const char *name) : BaseApp(name, true, c_gray) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			char const *frags[3] = {
			        c_frag,
			        c_frag_box,
			        c_frag_near,
			};

			for (auto i = 0; i < 3; i++) {
				Attrs shader_attrs = {
				        {"vert", c_vert  },
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", i == 0 ? &u_worldscreen0 : &u_worldscreen1},
				        {"u_diffuse",     &u_diffuse                                },
				        {"u_diffuse",     &m_samplerId                              },
				};
				loadShader(m_shaders[i], shader_attrs, unif_attrs);
			}
		}

		// texture
		{
			Attrs attrs = {
			        {"min_filter", GL_NEAREST},
			        {"mag_filter", GL_NEAREST},
			};
			m_textureId = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}

		// sampler
		{
			Vec4f border = {0, 0, 0, 0};
			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER},
			        {"min_filter",   GL_NEAREST        },
			        {"mag_filter",   GL_NEAREST        },
			        {"wrap_s",       GL_CLAMP_TO_EDGE  },
			        {"wrap_t",       GL_CLAMP_TO_EDGE  },
			        {"wrap_r",       GL_CLAMP_TO_EDGE  },
			        {"border",       border            },
			        {"min_lod",      -1000.0           },
			        {"max_lod",      +1000.0           },
			        {"lod_bias",     0.0               },
			        {"compare_mode", GL_NONE           },
			        {"compare_func", GL_LEQUAL         },
			};
			m_samplerId = spu_texture_new(attrs);
		}

		// color render buffer
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_RGBA8                 },
			        {"width",       int32_t(c_fb_viewport.sx)},
			        {"height",      int32_t(c_fb_viewport.sy)},
			        {"multisample", 4                        },
			};
			m_renderId[0] = spu_texture_new(attrs);
		}

		// depth render buffer
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_DEPTH_COMPONENT24     },
			        {"width",       int32_t(c_fb_viewport.sx)},
			        {"height",      int32_t(c_fb_viewport.sy)},
			        {"multisample", 4                        },
			};
			m_renderId[1] = spu_texture_new(attrs);
		}

		// frame buffer
		{
			Attrs attrs = {
			        {"viewport0", c_fb_viewport},
			        {"color0", m_renderId[0]},
			        {"depth", m_renderId[1]},
			        {"bgcolor0", Vec4f(1.0f, 0.5f, 0.0f, +1.0f)},
			        {"bgdepth", 1.0},
			};
			m_frameId = spu_frame_new(attrs);
		}

		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>(Vec2f(2.0, 1.5));
			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 2                },
			        {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
	}

	void render() override
	{
		// Pass 1
		{
			MultisampleControl mcontrol(m_frameId, getGesture());
			SpuScopedRenderstate renderstate(true);  // must be here

			getCamera().setViewscreen(c_fb_viewport, 180.0 * 0.25f, 0.1, 100.0);

			u_worldscreen0 = getCamera().worldscreen();
			u_diffuse = m_textureId;

			renderstate.flags.depth_test = true;
			renderstate.use();

			m_shaders[0].use();

			spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 5);
		}

		// Pass 2
		{
			getCamera().setViewscreen(
			        viewport(0), 180.0 * 0.25, 0.1,
			        100.0 /*, float(viewport(0).sx) / viewport(0).sy*/);

			u_worldscreen1 = getCamera().worldscreen();

			// Box
			{
				auto scissor = Rectf(1, 1, viewport(0).sx / 2 - 2, viewport(0).sy - 2);
				spu_frame_set(-1, "scissor0", scissor);

				u_diffuse = m_renderId[0];
				m_shaders[1].use();
				spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 5);
			}

			// Near
			{
				auto scissor = Rectf(
				        viewport(0).sx / 2 + 1, 1, viewport(0).sx / 2 - 2, viewport(0).sy - 2);

				spu_frame_set(-1, "scissor0", scissor);

				u_diffuse = m_renderId[0];
				m_shaders[2].use();
				spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 5);
			}
		}

		// recover
		{
			auto scissor = Rectf(0, 0, viewport(0).sx, viewport(0).sy);
			spu_frame_set(-1, "scissor0", scissor);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_fbo_multisample_explicit_nv");
}  // namespace
}  // namespace spu
