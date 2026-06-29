//
// App :
//
#include "base_app.h"
// #include <spu/config.h>

namespace spu {
namespace {

#define e_fb_scale "2"  // string

/* clang-format off */
const char *c_vert = {
    "#version 430                                                                           \n"
    "uniform mat4 u_worldscreen;                                                            \n"
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
    "    gl_Position = u_worldscreen * vec4(a_position, 1.0);                               \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 430                                                                           \n"
    "uniform sampler2DArray u_diffuse;                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, vec3(f_texcoord, 0.0));                                 \n"
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
    "      vec4(4*(gl_VertexID%2)-1, 4*(gl_VertexID/2)-1, 0, 1);                            \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 texture_size = vec2(textureSize(u_diffuse, 0));                               \n"
        "    color = texture(u_diffuse, gl_FragCoord.xy * " e_fb_scale "/ texture_size);"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shaders[2];  // 0:texture 1:splash
	uint32_t m_rgbTextureId;
	uint32_t m_srgbTextureId;
	uint32_t m_colorId;
	uint32_t m_depthId;
	uint32_t m_arrayIds[2];
	uint32_t m_frameId;

	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		const auto fb_scale = std::stof(e_fb_scale);
		auto fb_viewport = Rectf(0.0f, 0.0f, viewport(0).sx * fb_scale, viewport(0).sy * fb_scale);
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
			        {"nelem", 3},
			};
			m_arrayIds[1] = spu_array_new(attrs);
		}

		// texture #0
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_ARRAY    },
                                {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
                                {"wrap_s",      GL_CLAMP_TO_EDGE       },
			        {"wrap_t",      GL_CLAMP_TO_EDGE       },
                                {"auto_mipmap", 0                      },
			};
			m_srgbTextureId = loadDDS("kueken7_rgba8_srgb.dds", attrs);
		}

		// texture #1
		{
			m_rgbTextureId
			        = spu_texture_alias(m_srgbTextureId, GL_TEXTURE_2D_ARRAY, GL_RGBA8, 1, 0);
		}

		// texture #2
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D  },
                                {"iformat",     GL_SRGB8_ALPHA8},
			        {"width",       width          },
                                {"height",      height         },
			        {"base_level",  0              },
                                {"max_level",   0              },
			        {"min_filter",  GL_LINEAR      },
                                {"mag_filter",  GL_LINEAR      },
			        {"auto_mipmap", 0              },
			};
			m_colorId = spu_texture_new(attrs);
		}

		// texture #3
		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D       },
                                {"iformat",    GL_DEPTH_COMPONENT24},
			        {"width",      width               },
                                {"height",     height              },
			        {"base_level", 0                   },
                                {"max_level",  0                   },
			};
			m_depthId = spu_texture_new(attrs);
		}

		// framebuffer
		{
			auto bgcolor0 = Vec4f(1.0, 0.5, 0.0, +1.0);

			Attrs attrs = {
			        {"color0",    m_colorId  },
			        {"depth",     m_depthId  },
			        {"viewport0", fb_viewport},
			        {"bgcolor0",  bgcolor0   },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		{
			// spu_frame_set(m_frameId, "srgb", 1);

			spu_frame_begin(m_frameId);
			spu_frame_clear(m_frameId);
			// Convert linear clear color to sRGB color space, FramebufferName is a
			// sRGB FBO

			// m_textureIds[texture::DIFFUSE] is a sRGB texture which sRGB
			// conversion on fetch has been disabled Hence in the shader, the value
			// is stored as sRGB so we should not convert it to sRGB.
			// getRenderstate().flags.framebuffer_srgb = 0;
			u_diffuse = m_rgbTextureId;

			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);

			// spu_frame_set(m_frameId, "srgb", 0);
			spu_frame_end();
		}

		{
			u_diffuse = m_colorId;
			m_shaders[1].use();
			spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_fbo_srgb_decode");
}  // namespace
}  // namespace spu
