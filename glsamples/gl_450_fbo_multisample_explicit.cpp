//
// App :
//
#include "base_app.h"
namespace spu {
namespace {
/* clang-format off */
const char *c_vert = {
    "#version 430 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, float(gl_InstanceID) * 1.0 - 2.0, 1.0);\n"
    "}                                                                                      \n"
};
	
const char *c_frag = {
    "#version 430 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord);                                            \n"
    "}                                                                                      \n"
};

const char *c_frag_box = {
    "#version 430 core                                                                      \n"
    "#extension GL_ARB_shader_texture_image_samples : require                               \n"
    "uniform sampler2DMS u_diffuse;                                                         \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    // integer UV coordinates, needed for fetching multisampled texture                \n"
    "    ivec2 texcoord = ivec2(textureSize(u_diffuse) * f_texcoord);                       \n"
    "    vec4 temp = vec4(0.0);                                                             \n"
    "    float samples = textureSamples(u_diffuse);                                         \n"
    "    // For each of the samples                                                         \n"
    "    for(int i = 0; i < samples; ++i)                                                   \n"
    "        temp += texelFetch(u_diffuse, texcoord, i);                                    \n"
    "    color = temp / samples;                                                            \n"
    "}                                                                                      \n"
};

const char *c_frag_near = {
    "#version 430 core                                                                      \n"
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
const auto c_width = int32_t(c_fb_viewport.sx);
const auto c_height = int32_t(c_fb_viewport.sy);

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shaders[3];  // 0:through 1:resolve box 2:resolve near
	uint32_t m_arrayId;
	uint32_t m_textureIds[4];  // 0:multisample depth 1:multisample color 2:diffuse
	uint32_t m_frameId;

	Mat4f u_worldscreen0;
	Mat4f u_worldscreen1;
	uint32_t u_diffuse;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// gl_aux_init();
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
				};
				loadShader(m_shaders[i], shader_attrs, unif_attrs);
			}
		}
		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>(Vec2f(4.0, 3.0));

			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 2                },
			        {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// texture multisample depth
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_DEPTH_COMPONENT24     },
			        {"width",       c_width                  },
			        {"width",       c_width                  },
			        {"height",      c_height                 },
			        {"multisample", 4                        },
			};
			m_textureIds[0] = spu_texture_new(attrs);
		}

		// texture multisample color
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_RGBA8                 },
			        {"width",       c_width                  },
			        {"width",       c_width                  },
			        {"height",      c_height                 },
			        {"multisample", 4                        },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// texture diffuse
		{
			Attrs attrs = {
			        {"min_filter", GL_NEAREST},
			        {"mag_filter", GL_NEAREST},
			};
			m_textureIds[2] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}

		// framebuffer
		{
			Attrs attrs = {
			        {"depth",     m_textureIds[0]},
			        {"color0",    m_textureIds[1]},
			        {"viewport0", c_fb_viewport  },
			        {"bgcolor0",  c_orange       },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		{
			getCamera().setViewscreen(viewport(0), 180.0 * 0.25, 0.1, 100.0);

			u_worldscreen0 = getCamera().worldscreen() * Mat4f().scale(0.3);
		}
		{
			getCamera().setViewscreen(-4.0, 4.0, -3.0, 3.0, 0.0, 100.0, false);
			u_worldscreen1 = getCamera().viewscreen() * Mat4f().trans(getCamera().position() * 2.0);
		}

		// Pass 1
		{
			// const float c_one = 1.0;

			MultisampleControl mcontrol(m_frameId, getGesture());
			SpuScopedRenderstate renderstate(true);  // must be after mcontrol!

			renderstate.flags.depth_test = true;  // care
			renderstate.use();

			// spu_frame_begin(m_frameId, false);

			u_diffuse = m_textureIds[2];
			m_shaders[0].use();
			spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 5);  // 5 billboards

			// spu_frame_end();
		}

		// Pass 2
		{
			u_diffuse = m_textureIds[1];

			// Box
			{
				auto scissor = Rectf(1, 1, viewport(0).sx / 2 - 2, viewport(0).sy - 2);

				spu_frame_set(-1, "scissor0", scissor);

				m_shaders[1].use();
				spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 5);  // 5 billboards
			}

			// Near
			{
				auto scissor = Rectf(
				        viewport(0).sx / 2 + 1, 1, viewport(0).sx / 2 - 2, viewport(0).sy - 2);

				spu_frame_set(-1, "scissor0", scissor);

				m_shaders[2].use();
				spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 5);  // 5 billboards
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_450_fbo_multisample_explicit");
}  // namespace
}  // namespace spu
