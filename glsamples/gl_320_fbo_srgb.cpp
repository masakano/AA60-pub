//
// App :
//
#include "base_app.h"

namespace spu {
namespace {

#define e_fb_scale "2"  // string

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
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"

    "// convertRgbToSrgb                                                                    \n"
    "vec3 detail_rgbToSrgb(const vec3 colorRGB, const float gammaCorrection)                \n"
    "{                                                                                      \n"
    "    vec3 clampedColorRGB = clamp(colorRGB, 0.0, 1.0);                                  \n"
    "    return mix(                                                                        \n"
    "        pow(clampedColorRGB, vec3(gammaCorrection)) * 1.055 - 0.055,                   \n"
    "        clampedColorRGB * 12.92,                                                       \n"
    "        lessThan(clampedColorRGB, vec3(0.0031308)));                                   \n"
    "}                                                                                      \n"
    "// For all settings: 1.0 = 100% 0.5=50% 1.5 = 150%                                     \n"
    "vec3 contrastSaturationBrightness(vec3 color, float brt, float sat, float con)         \n"
    "{                                                                                      \n"
    "    const vec3 lumCoeff = vec3(0.2125, 0.7154, 0.0721);                                \n"
    "    vec3 brtcolor = color * brt;                                                       \n"
    "    vec3 intensity = vec3(dot(brtcolor, lumCoeff));                                    \n"
    "    vec3 satcolor = mix(intensity, brtcolor, sat);                                     \n"
    "    vec3 concolor = mix(vec3(0.5), satcolor, con);                                     \n"
    "    return concolor;                                                                   \n"
    "}                                                                                      \n"
    "// Main                                                                                \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec3 colorRGB = texture(u_diffuse, f_texcoord).rgb;                                \n"
    "    color = vec4(colorRGB, 1.0);                                                       \n"
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
    "    color = texture(u_diffuse, gl_FragCoord.xy * " e_fb_scale " / texture_size);	    \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[3];  // 0:diffuse 1:colorbuffer 2:renderbuffer
	SpuShader m_shaders[2];    // 0:texture 1:splash

	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	uint32_t m_frameId;
	uint32_t m_arrayIds[2];

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
			for (auto &shader: m_shaders) {
				int32_t i = &shader - &m_shaders[0];
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen},
				        {"u_diffuse",     &u_diffuse    },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			}
		}

		// texture #0
		{
			Attrs attrs = {
			        {"iformat",    GL_RGBA8               }, // linear
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_CLAMP_TO_EDGE       },
			        {"wrap_t",     GL_CLAMP_TO_EDGE       },
			};
			m_textureIds[0] = loadDDS("kueken7_rgba8_srgb.dds", attrs);
		}
		// texture #1
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D  },
                                {"iformat",     GL_SRGB8_ALPHA8}, // SRGB
			        {"width",       width          },
                                {"height",      height         },
			        {"base_level",  0              },
                                {"max_level",   0              },
			        {"min_filter",  GL_LINEAR      },
                                {"mag_filter",  GL_LINEAR      },
			        {"auto_mipmap", 0              },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// texture #2
		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D        },
                                {"iformat",    GL_DEPTH_COMPONENT32F},
			        {"width",      width                },
                                {"height",     height               },
			        {"base_level", 0                    },
                                {"max_level",  0                    },
			};
			m_textureIds[2] = spu_texture_new(attrs);
		}

		// frame
		{
			Attrs attrs = {
			        {"color0",    m_textureIds[1]},
			        {"depth",     m_textureIds[2]},
			        {"viewport0", fb_viewport    },
			};
			m_frameId = spu_frame_new(attrs);
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
			        {"shader_id",    m_shaders[1].id()},
			        {"a.a_position", 2                },
			        {"a.a_texcoord", 2                },
			};
			m_arrayIds[1] = spu_array_new(attrs);
			spu_array_link(m_arrayIds[1], m_arrayIds[0], 0, 0);
		}
	}

	void render() override
	{
		{
			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
			u_worldscreen = getCamera().worldscreen();
		}

		// Render a textured quad to a sRGB framebuffer object.
		{
			spu_frame_begin(m_frameId);
			spu_frame_clear(m_frameId);

			// Explicitly convert linear pixel color to sRGB color space, as
			// FramebufferName is a sRGB FBO
			// Shader execution is done with linear color to get correct linear
			// algebra working.

			u_diffuse = m_textureIds[0];
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES, 0, 0, 2);
			spu_frame_end();
		}

		// Blit the sRGB framebuffer to the default framebuffer back buffer.
		{
			u_diffuse = m_textureIds[1];
			m_shaders[1].use();
			spu_array_draw(m_arrayIds[1], GL_TRIANGLES, 0, 3);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_fbo_srgb");
}  // namespace
}  // namespace spu
