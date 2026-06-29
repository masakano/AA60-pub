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
    "    gl_Position = u_worldscreen * vec4(a_position, float(gl_InstanceID) * 1.0 - 2.0, 1.0);\n"
    "}                                                                                      \n"
};

const char *c_frag_texture = {
    "#version 330                                                                           \n"
    "uniform bool u_is_texture;                                                             \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if (u_is_texture) {                                                                \n"
    "        color = texture(u_diffuse, f_texcoord);                                        \n"
    "    }                                                                                  \n"
    "    else {                                                                             \n"
    "        color = vec4(1);                                                               \n"
    "    }                                                                                  \n"
    "}                                                                                      \n"
};

const char *c_frag_box = {
    "#version 330                                                                           \n"
    "uniform sampler2DMS u_diffuse;                                                         \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
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
    "#version 330                                                                           \n"
    "uniform sampler2DMS u_diffuse;                                                         \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    // integer UV coordinates, needed for fetching multisampled texture                \n"
    "    ivec2 texcoord = ivec2(textureSize(u_diffuse) * f_texcoord);                       \n"
    "    color = texelFetch(u_diffuse, texcoord, 0);                                        \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[3];  // 0:multisampled depth 1:multisampled color 2:diffuse
	SpuShader m_shaders[3];    // 0:through 1:resolve box 2:resolve near
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_is_texture;

	uint32_t m_frameId;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_sky) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		auto fb_viewport = Rectf(0, 0, 160, 120);
		auto width = int32_t(fb_viewport.sx);
		auto height = int32_t(fb_viewport.sy);

		// shader
		{
			const char *frags[] = {
			        c_frag_texture,
			        c_frag_box,
			        c_frag_near,
			};

			for (auto &shader: m_shaders) {
				int32_t i = &shader - &m_shaders[0];
				Attrs shader_attrs = {
				        {"vert", c_vert  },
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen},
				        {"u_diffuse",     &u_diffuse    },
				        {"u_is_texture",  &u_is_texture },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			}
		}

		// buffer
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>(Vec2f(4.0, 3.0));

			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 2                },
			        {"a.a_texcoord", 2                },
			};
			m_arrayId = spu_array_new(attrs);
			spu_array_send(m_arrayId, c_vertices.data(), c_vertices.size());
		}

		// depth
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_DEPTH_COMPONENT24     },
			        {"width",       width                    },
			        {"height",      height                   },
			        {"multisample", 4                        },
			};
			m_textureIds[0] = spu_texture_new(attrs);
		}

		// color
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_MULTISAMPLE},
			        {"iformat",     GL_RGBA8                 },
			        {"width",       width                    },
			        {"height",      height                   },
			        {"multisample", 4                        },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// diffuse
		{
			Attrs attrs = {
			        {"base_level",  0         },
                                {"max_level",   0         },
                                {"min_filter",  GL_NEAREST},
			        {"mag_filter",  GL_NEAREST},
                                {"auto_mipmap", 0         },
			};
			m_textureIds[2] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}

		// frame buffer
		{
			Attrs attrs = {
			        {"color0",    m_textureIds[1]},
			        {"depth",     m_textureIds[0]},
			        {"viewport0", fb_viewport    },
			        {"bgcolor0",  c_orange       },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		// render FBO
		{
			MultisampleControl mcontrol(m_frameId, getGesture());
			SpuScopedRenderstate renderstate(true);  // must be here!

			renderstate.flags.depth_test = true;
			renderstate.use();

			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

			u_worldscreen = getCamera().worldscreen() * Mat4f().scale(0.3);
			u_diffuse = m_textureIds[2];
			u_is_texture = true;
			m_shaders[0].use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);

			u_is_texture = false;
			m_shaders[0].use();
			spu_array_draw(m_arrayId, GL_LINE_LOOP);

			u_is_texture = true;  // for safety
		}
		// render FB
		{
			getCamera().setViewscreen(-4.0, 4.0, -3.0, 3.0, 0.0, 100.0, false);
			u_worldscreen = getCamera().viewscreen() * Mat4f().trans(getCamera().position() * 2.0);
			u_diffuse = m_textureIds[1];

			const std::vector<Rectf> c_scissors = makeViewports(2, 1, 1);

			// Box
			{
				spu_frame_set(-1, "scissor0", c_scissors[0]);
				m_shaders[1].use();
				spu_array_draw(m_arrayId, GL_TRIANGLES);
			}

			// Near
			{
				spu_frame_set(-1, "scissor0", c_scissors[1]);
				m_shaders[2].use();
				spu_array_draw(m_arrayId, GL_TRIANGLES);
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo_multisample_explicit");
}  // namespace
}  // namespace spu
