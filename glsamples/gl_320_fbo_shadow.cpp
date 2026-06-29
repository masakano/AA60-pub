//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert_depth = {
    "#version 330                                                                           \n"
    "uniform mat4 u_depth_worldscreen;                                                       \n"
    "in vec3 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_depth_worldscreen * vec4(a_position,1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_vert = {
    "#version 330                                                                           \n"
    "in vec3 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "uniform mat4 u_depth_bias_worldscreen;                                                  \n"
    "out vec4 f_color;                                                                      \n"
    "out vec4 f_shadowcoord;                                                                \n"
    "out vec4 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 1.0);                                \n"
    "    f_shadowcoord = u_depth_bias_worldscreen * vec4(a_position, 1.0);                   \n"
    "    f_color = a_color;                                                                 \n"
    "    f_texcoord = vec4(a_position, 1.0);                                                \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "uniform sampler2DShadow u_shadow;                                                      \n"
    "in vec4 f_color;                                                                       \n"
    "in vec4 f_shadowcoord;                                                                 \n"
    "in vec4 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 shadowcoord = f_shadowcoord;                                                  \n"
    "    shadowcoord.z -= 0.005;                                                            \n"
    "    vec4 diffuse = mix(f_color, texture(u_diffuse, f_texcoord.xy), 0.5);               \n"
    "    float visibility = mix(0.5, 1.0, texture(u_shadow, shadowcoord.xyz));              \n"
    "    color = visibility * diffuse;                                                      \n"
    "}                                                                                      \n"
};

const Vec4f c_shadow_viewport = {0, 0, 64, 64};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[2];  // 0:diffuse 1:shadowmap
	SpuShader m_shaders[2];    // 0:depth 1:render
	Mat4f u_worldscreen;
	Mat4f u_depth_worldscreen;
	Mat4f u_depth_bias_worldscreen;
	uint32_t u_shadow;
	uint32_t u_diffuse;

	uint32_t m_arrayIds[2];

	uint32_t m_frameId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			const char *verts[] = {
			        c_vert_depth,
			        c_vert,
			};

			const char *frags[] = {
			        nullptr /* no fragment shader */,
			        c_frag,
			};

			for (auto &shader: m_shaders) {
				int32_t i = &shader - &m_shaders[0];
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen",            &u_worldscreen           },
				        {"u_depth_worldscreen",      &u_depth_worldscreen     },
				        {"u_depth_bias_worldscreen", &u_depth_bias_worldscreen},
				        {"u_diffuse",                &u_diffuse               },
				        {"u_shadow",                 &u_shadow                },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			}
		}

		// buffer #0
		{
			const std::vector<v3fv4u8_t> c_vertices = {
			        {-1.0, -1.0, 0.0, 255, 127, 0,   255},
                                {+1.0, -1.0, 0.0, 255, 127, 0,   255},
			        {+1.0, +1.0, 0.0, 255, 127, 0,   255},
                                {-1.0, +1.0, 0.0, 255, 127, 0,   255},
			        {-0.1, -0.1, 0.2, 0,   127, 255, 255},
                                {+0.1, -0.1, 0.2, 0,   127, 255, 255},
			        {+0.1, +0.1, 0.2, 0,   127, 255, 255},
                                {-0.1, +0.1, 0.2, 0,   127, 255, 255},
			};

			const std::vector<uint16_t> c_indices = {
			        0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4,
			};

			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 3                },
			        {"format",       GL_UNSIGNED_BYTE },
			        {"normalize",    1                },
			        {"a.a_color",    4                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayIds[0] = spu_array_new(attrs);
			spu_array_send(m_arrayIds[0], c_indices.data(), c_indices.size(), -1, 2);
		}
		// buffer #1
		{
			Attrs attrs = {
			        {"shader_id",    m_shaders[1].id()},
			        {"a.a_position", 3                },
			        {"format",       GL_UNSIGNED_BYTE },
			        {"normalize",    1                },
			        {"a.a_color",    4                },
			};
			m_arrayIds[1] = spu_array_new(attrs);
			spu_array_link(m_arrayIds[1], m_arrayIds[0], 0, 0);
			spu_array_link(m_arrayIds[1], m_arrayIds[0], -1, -1);
		}

		// texture #0
		{
			Attrs attrs = {
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			};
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_srgb.dds", attrs);
		}

		// texture #1
		{
			Attrs attrs = {
			        {"target",       GL_TEXTURE_2D            },
			        {"iformat",      GL_DEPTH_COMPONENT32F    },
			        {"width",        int(c_shadow_viewport.sx)},
			        {"height",       int(c_shadow_viewport.sy)},
			        {"base_level",   0                        },
			        {"max_level",    0                        },
			        {"min_filter",   GL_LINEAR                },
			        {"mag_filter",   GL_LINEAR                },
			        {"compare_func", GL_LEQUAL                },
			        {"compare_mode", GL_COMPARE_R_TO_TEXTURE  },
			        {"wrap_s",       GL_CLAMP_TO_EDGE         },
			        {"wrap_t",       GL_CLAMP_TO_EDGE         },
			        {"auto_mipmap",  0                        },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// framebuffer
		{
			Attrs frame_attrs = {
			        {"depth",     m_textureIds[1]  },
			        {"viewport0", c_shadow_viewport},
			};
			m_frameId = spu_frame_new(frame_attrs);
			float depth = 1.0;
			spu_frame_set(m_frameId, "bgdepth", depth);
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.depth_func = GL_LESS;
		}
	}

	void render() override
	{
		// Update of the u_worldscreen matrix for the render pass
		{
			getCamera().setViewscreen(viewport(0), 45, 0.1, 10.0);
			u_worldscreen = getCamera().worldscreen();
		}

		// Update of the u_worldscreen matrix for the depth pass
		{
			Camera shadow_camera(getGesture());
			shadow_camera.setViewscreen(-1.0, 1.0, -1.0, 1.0, -4.0, 8.0, false);
			shadow_camera.setWorldview(Vec3f(0.5, 1.0, 2.0), ezero(), ez());

			Mat4f bias_matrix(
			        0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);

			u_depth_worldscreen = shadow_camera.worldscreen();
			u_depth_bias_worldscreen = bias_matrix * shadow_camera.worldscreen();
		}

		// draw shadow
		{
			spu_frame_begin(m_frameId);
			spu_frame_clear(m_frameId);
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);
			spu_frame_end();
		}

		// draw model
		{
			u_diffuse = m_textureIds[0];
			u_shadow = m_textureIds[1];

			m_shaders[1].use();
			spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo_shadow");
}  // namespace
}  // namespace spu
