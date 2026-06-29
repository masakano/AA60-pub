//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert1 = {
    "#version 400                                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(4*(gl_VertexID%2)-1, 4*(gl_VertexID/2)-1, 0, 1);                \n"
    "}                                                                                      \n"
};

const char *c_frag1 = {
    "#version 400                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "uniform float u_framebuffer_size;                                                      \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 texture_size = vec2(textureSize(u_diffuse, 0)) * u_framebuffer_size;          \n"
    "    color = texture(u_diffuse, gl_FragCoord.xy / texture_size);                        \n"
    "    //color = texelFetch(u_diffuse, ivec2(gl_FragCoord.xy), 0);                        \n"
    "}                                                                                      \n"
};

const char *c_vert2 = {
    "#version 400                                                                           \n"
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

const char *c_frag2 = {
    "#version 400                                                                           \n"
    "//#define FUNCTION                                                                     \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "uniform bool u_use_grad;                                                               \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "#ifndef FUNCTION                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if(u_use_grad)                                                                     \n"
    "    {                                                                                  \n"
    "        vec2 texcoord00 = interpolateAtOffset(f_texcoord, vec2(-0.5,-0.5));            \n"
    "        vec2 texcoord10 = interpolateAtOffset(f_texcoord, vec2( 0.5,-0.5));            \n"
    "        vec2 texcoord11 = interpolateAtOffset(f_texcoord, vec2( 0.5, 0.5));            \n"
    "        vec2 texcoord01 = interpolateAtOffset(f_texcoord, vec2(-0.5, 0.5));            \n"
    "        color = textureGrad(                                                           \n"
    "                     u_diffuse,                                                        \n"
    "                     f_texcoord,                                                       \n"
    "                     abs(texcoord10-texcoord00),                                       \n"
    "                     abs(texcoord01-texcoord00));                                      \n"
    "    }                                                                                  \n"
    "    else                                                                               \n"
    "    {                                                                                  \n"
    "        color = texture(u_diffuse, f_texcoord);                                        \n"
    "    }                                                                                  \n"
    "}                                                                                      \n"
    "/*                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(fwidthFine(interpolateAtCentroid(f_texcoord)), 0.0, 1.0);             \n"
    "}                                                                                      \n"
    "*/                                                                                     \n"
    "#else//FUNCTION                                                                        \n"
    "vec4 textureFine(in sampler2D u_sampler, in vec2 a_texcoord)                           \n"
    "{                                                                                      \n"
    "    vec2 texcoord00 = interpolateAtOffset(texcoord, vec2(-0.5,-0.5));                  \n"
    "    vec2 texcoord10 = interpolateAtOffset(texcoord, vec2( 0.5,-0.5));                  \n"
    "    vec2 texcoord11 = interpolateAtOffset(texcoord, vec2( 0.5, 0.5));                  \n"
    "    vec2 texcoord01 = interpolateAtOffset(texcoord, vec2(-0.5, 0.5));                  \n"
    "    return textureGrad(                                                                \n"
    "                  u_sampler,                                                           \n"
    "                  texcoord,                                                            \n"
    "                  abs(texcoord10-texcoord00),                                          \n"
    "                  abs(texcoord01-texcoord00));                                         \n"
    "}                                                                                      \n"
    "vec4 textureCoarse(in sampler2D u_sampler, in vec2 a_texcoord)                         \n"
    "{                                                                                      \n"
    "    return texture(u_sampler, texcoord);                                               \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if(u_use_grad)                                                                     \n"
    "    {                                                                                  \n"
    "        color = textureFine(u_diffuse, f_texcoord);                                    \n"
    "    }                                                                                  \n"
    "    else                                                                               \n"
    "    {                                                                                  \n"
    "        color = textureCoarse(u_diffuse, f_texcoord);                                  \n"
    "    }                                                                                  \n"
    "}                                                                                      \n"
    "#endif//FUNCTION                                                                       \n"
};

const auto c_frame_size_rate = 2;

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[3];  // 0:diffuse 1:colorbuffer 2:renderbuffer
	SpuShader m_shaders[2];    // 0:texture 1:splash
	uint32_t m_arrayIds[2];
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	int32_t u_use_grad;
	float u_framebuffer_size;
	uint32_t m_frameId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			const char *verts[] = {
			        c_vert2,  // TEXTURE
			        c_vert1,  // SPLASH
			};
			const char *frags[] = {
			        c_frag2,  // TEXTURE
			        c_frag1,  // SPLASH
			};

			for (auto &shader: m_shaders) {
				auto i = &shader - &m_shaders[0];
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen",      &u_worldscreen     },
				        {"u_diffuse",          &u_diffuse         },
				        {"u_use_grad",         &u_use_grad        },
				        {"u_framebuffer_size", &u_framebuffer_size},
				};
				loadShader(shader, shader_attrs, unif_attrs);
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
			        {"nelem", 3}, // one triangle
			};
			m_arrayIds[1] = spu_array_new(attrs);
		}

		// texture #0
		{
			const auto c_size = 128;
			const auto c_level_count = 7;

			// use other color..
			const std::vector<uint32_t> c_pixs = [=]() {
				std::vector<uint32_t> pixs(c_level_count * c_size * c_size);
				for (auto i = 0; i < c_size * c_size; i++) {
					pixs[0 * c_size * c_size + i] = 0xff0000ff;
					pixs[1 * c_size * c_size + i] = 0xff7f00ff;
					pixs[2 * c_size * c_size + i] = 0xff00ffff;
					pixs[3 * c_size * c_size + i] = 0xff00ff00;
					pixs[4 * c_size * c_size + i] = 0xffffff00;
					pixs[5 * c_size * c_size + i] = 0xffff0000;
					pixs[6 * c_size * c_size + i] = 0xff0000ff;
				}
				return pixs;
			}();

			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D           },
			        {"iformat",     GL_RGBA8                },
			        {"width",       c_size                  },
			        {"height",      c_size                  },
			        {"base_level",  0                       },
			        {"max_level",   c_level_count - 1       },
			        {"min_filter",  GL_NEAREST_MIPMAP_LINEAR},
			        {"mag_filter",  GL_NEAREST              },
			        {"wrap_s",      GL_CLAMP_TO_EDGE        },
			        {"wrap_t",      GL_CLAMP_TO_EDGE        },
			        {"swizzle_r",   GL_BLUE                 },
			        {"swizzle_g",   GL_GREEN                },
			        {"swizzle_b",   GL_RED                  },
			        {"swizzle_a",   GL_ALPHA                },
			        {"auto_mipmap", 0                       },
			};
			m_textureIds[0] = spu_texture_new(attrs);

			for (auto i = 0; i < c_level_count; i++) {
				int32_t locs[4] = {0, 0, 0, i};
				spu_texture_send(
				        m_textureIds[0], c_pixs.data() + i * c_size * c_size, GL_RGBA8, locs);
			}
		}

		// texture 1 & 2
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D                              },
			        {"iformat",     0                                          }, // place holder
			        {"width",       int32_t(viewport(0).sx) / c_frame_size_rate},
			        {"height",      int32_t(viewport(0).sy) / c_frame_size_rate},
			        {"base_level",  0                                          },
			        {"max_level",   0                                          },
			        {"min_filter",  GL_NEAREST                                 },
			        {"mag_filter",  GL_NEAREST                                 },
			        {"auto_mipmap", 0                                          },
			};
			attrs.replace("iformat", GL_RGBA8);
			m_textureIds[1] = spu_texture_new(attrs);

			attrs.replace("iformat", GL_DEPTH_COMPONENT24);
			m_textureIds[2] = spu_texture_new(attrs);
		}

		// framebuffer
		{
			auto scaled_viewport = Rectf(
			        0, 0, viewport(0).sx / c_frame_size_rate, viewport(0).sy / c_frame_size_rate);

			Attrs attrs = {
			        {"color0",    m_textureIds[1]},
			        {"depth",     m_textureIds[2]},
			        {"viewport0", scaled_viewport},
			};
			m_frameId = spu_frame_new(attrs);
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.depth_func = GL_LESS;
		}
	}

	void render() override
	{
		auto w = float(viewport(0).sx / c_frame_size_rate);
		auto h = float(viewport(0).sy / c_frame_size_rate);

		Rectf scissor[] = {
		        {0,     0, 0,         0}, // disable
		        {0,     0, w / 2 - 1, h},
		        {w / 2, 0, w / 2 - 1, h},
		};

		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		spu_frame_set(m_frameId, "scissor0", scissor[0]);

		spu_frame_begin(m_frameId);
		spu_frame_clear(m_frameId);

		spu_frame_set(m_frameId, "scissor0", scissor[1]);

		{
			u_diffuse = m_textureIds[0];
			u_use_grad = 1;
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES, 0, 0, 2);  // why two times?
		}
		spu_frame_set(m_frameId, "scissor0", scissor[2]);

		{
			u_use_grad = 0;
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES, 0, 0, 2);  // why two times?
		}
		spu_frame_end();

		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = false;
			renderstate.use();

			u_diffuse = m_textureIds[1];
			u_framebuffer_size = c_frame_size_rate;
			m_shaders[1].use();
			spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_texture_derivative");
}  // namespace
}  // namespace spu
