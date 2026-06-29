//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert_depth = {
    "#version 400 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "uniform mat4 u_depth_worldscreen;                                                       \n"
    "in vec3 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_depth_worldscreen * vec4(a_position, 1.0);                          \n"
    "}                                                                                      \n"
};

const char *c_vert = {
    "#version 400 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "uniform mat4 u_depth_bias_worldscreen;                                                  \n"
    "in vec3 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "out vec4 f_color;                                                                      \n"
    "out vec4 f_shadowcoord;                                                                \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 1.0);                                \n"
    "    f_shadowcoord = u_depth_bias_worldscreen * vec4(a_position, 1.0);                   \n"
    "    f_color = a_color;                                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
    "uniform sampler2DShadow u_shadow;                                                      \n"
    "in vec4 f_color;                                                                       \n"
    "in vec4 f_shadowcoord;                                                                 \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 shadowcoord = f_shadowcoord;                                                  \n"
    "    shadowcoord.z -= 0.005;                                                            \n"
    "    vec4 gather = textureGather(u_shadow, shadowcoord.xy, shadowcoord.z);              \n"
    "    float texel00 = gather.w;                                                          \n"
    "    float texel10 = gather.z;                                                          \n"
    "    float texel11 = gather.y;                                                          \n"
    "    float texel01 = gather.x;                                                          \n"
    "    vec2 shadow_size = textureSize(u_shadow, 0);                                       \n"
    "    vec2 texelcoord = shadowcoord.xy * shadow_size;                                    \n"
    "    vec2 samplecoord = fract(texelcoord + 0.5);                                        \n"
    "    float texel0 = mix(texel00, texel01, samplecoord.y);                               \n"
    "    float texel1 = mix(texel10, texel11, samplecoord.y);                               \n"
    "    float visibility = mix(texel0, texel1, samplecoord.x);                             \n"
    "    color = vec4(mix(vec4(0.5), vec4(1.0), visibility) * f_color);                     \n"
    "}                                                                                      \n"
};

const Vec4f c_shadow_viewport = {0, 0, 64, 64};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shaders[2];  // 0:depth 1:render
	uint32_t m_arrayIds[2];
	uint32_t m_frameId;
	Mat4f u_worldscreen;
	Mat4f u_depth_worldscreen;
	Mat4f u_depth_bias_worldscreen;
	uint32_t u_shadow;

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
				        {"u_shadow",                 &u_shadow                },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			}
		}
		// array
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

			// DEPTH
			{
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
			// RENDER
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
		}
		// texture
		{
			Attrs attrs = {
			        {"target",       GL_TEXTURE_2D            },
			        {"iformat",      GL_DEPTH_COMPONENT24     },
			        {"base_level",   0                        },
			        {"max_level",    0                        },
			        {"width",        int(c_shadow_viewport.sx)},
			        {"height",       int(c_shadow_viewport.sy)},
			        {"wrap_s",       GL_CLAMP_TO_EDGE         },
			        {"wrap_t",       GL_CLAMP_TO_EDGE         },
			        {"min_filter",   GL_NEAREST               },
			        {"mag_filter",   GL_NEAREST               },
			        {"compare_func", GL_LEQUAL                },
			        {"compare_mode", GL_COMPARE_R_TO_TEXTURE  },
			};
			u_shadow = spu_texture_new(attrs);
		}
		// framebuffer
		{
			Attrs attrs = {
			        {"depth",     u_shadow         },
			        {"viewport0", c_shadow_viewport},
			};
			m_frameId = spu_frame_new(attrs);
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.depth_func = GL_LESS;
			renderstate.use();
		}
	}

	void renderShadow()
	{
		spu_frame_begin(m_frameId);
		spu_frame_clear(m_frameId);

		m_shaders[0].use();
		spu_array_draw(m_arrayIds[0], GL_TRIANGLES);

		spu_frame_end();
	}

	void renderFramebuffer()
	{
		m_shaders[1].use();
		spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.01F, 16.0 /*5.0*/);

		// shadow
		Camera shadow_camera(getGesture());
		shadow_camera.setViewscreen(-1.0, 1.0, -1.0, 1.0, -4.0, 8.0, false);
		shadow_camera.setWorldview(Vec3f(0.5, 1.0, 2.0), ezero(), ez());

		Mat4f bias_matrix(
		        0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);

		u_worldscreen = getCamera().worldscreen();
		u_depth_worldscreen = shadow_camera.worldscreen();
		u_depth_bias_worldscreen = bias_matrix * shadow_camera.worldscreen();

		renderShadow();
		renderFramebuffer();
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_fbo_shadow");
}  // namespace
}  // namespace spu
