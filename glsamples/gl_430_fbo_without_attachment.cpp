//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
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

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "#extension GL_ARB_shader_image_size : require                                          \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "layout(binding = 0, rgba8) uniform coherent image2D u_color;                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 texcoord = gl_FragCoord.xy / vec2(imageSize(u_color));                        \n"
    "    imageStore(u_color, ivec2(gl_FragCoord.xy), textureLod(u_diffuse, texcoord, 2));   \n"
    "    //imageStore(u_color, ivec2(gl_FragCoord.xy), vec4(1.0)); // debug                 \n"
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
    "    vec2 size = vec2(textureSize(u_diffuse, 0));                                       \n"
    "    color = texelFetch(u_diffuse, ivec2(gl_FragCoord.x, size.y - gl_FragCoord.y), 0);  \n"
    "    //u_color = texelFetch(u_diffuse, ivec2(gl_FragCoord.x, gl_FragCoord.y), 0); // NVIDIA\n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[2];  // 0:diffuse 1:colorbuffer
	uint32_t m_samplerId;

	uint32_t m_pipeIds[2];  // 0:render 1:splash
	SpuShader m_shaders[2];
	uint32_t u_diffuse;
	uint32_t u_sampler;
	uint32_t u_color;  // render output

	uint32_t m_frameId;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

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
				        {"u_diffuse", &u_diffuse},
				        {"sampler",   &u_sampler},
				        {"u_color",   &u_color  },
				};
				loadShader(m_shaders[i], shader_attrs, unif_attrs);
			}
		}
		// array
		{
			Attrs attrs = {
			        {"a.0",   1},
                                {"nelem", 3}, // large single triangle
			};
			m_arrayId = spu_array_new(attrs);
		}
		// texture #0
		{
			Attrs attrs = {
			        {"mag_filter",  GL_NEAREST               },
                                {"min_filter",  GL_NEAREST_MIPMAP_NEAREST},
			        {"base_level",  0                        },
                                {"max_level",   0                        }, // necessary
			        {"auto_mipmap", 0                        },
			};
			m_textureIds[0] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}

		// texture #1
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
			        {"iformat",     GL_RGBA8               },
			        {"width",       int32_t(viewport(0).sx)},
			        {"height",      int32_t(viewport(0).sy)},
			        {"base_level",  0                      },
			        {"max_level",   0                      },
			        {"auto_mipmap", 0                      },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// sampler
		{
			Vec4f border = {0, 0, 0, 0};
			float min_lod = -1000;
			float max_lod = 1000;
			float lod_bias = 0;

			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER       },
			        {"min_filter",   GL_NEAREST_MIPMAP_NEAREST},
			        {"mag_filter",   GL_NEAREST               },
			        {"wrap_s",       GL_CLAMP_TO_EDGE         },
			        {"wrap_t",       GL_CLAMP_TO_EDGE         },
			        {"wrap_r",       GL_CLAMP_TO_EDGE         },
			        {"border",       border                   },
			        {"min_lod",      min_lod                  },
			        {"max_lod",      max_lod                  },
			        {"lod_bias",     lod_bias                 },
			        {"compare_mode", GL_NONE                  },
			        {"compare_func", GL_LEQUAL                },
			};
			m_samplerId = spu_texture_new(attrs);
		}
		// framebuffer
		{
			Attrs attrs = {
			        {"default_width",                  int32_t(viewport(0).sx)},
			        {"default_height",                 int32_t(viewport(0).sy)},
			        {"default_layers",                 1                      },
			        {"default_fixed_sample_locations", 1                      },
			};
			m_frameId = spu_frame_new(attrs);
			// spu_frame_report(m_frameId);
		}
	}

	void render() override
	{
		u_sampler = m_samplerId;

		// for debug
		clearImage();

		// Render
		spu_frame_begin(m_frameId);
		spu_frame_clear(m_frameId);

		u_diffuse = m_textureIds[0];
		u_color = m_textureIds[1];

		m_shaders[0].use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
		spu_frame_end();

		// splash
		u_diffuse = m_textureIds[1];
		m_shaders[1].use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}

	void clearImage()
	{
		auto pix = 0xff0000ffu;  // red
		spu_texture_send(m_textureIds[1], &pix, GL_RGBA8, nullptr, nullptr, true);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_fbo_without_attachment");
}  // namespace
}  // namespace spu
