//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

#define e_fb_scale "2"  // string

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "#extension GL_ARB_shader_storage_buffer_object : require                               \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "struct vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec2 position;                                                                     \n"
    "    vec2 texcoord;                                                                     \n"
    "};                                                                                     \n"
    "layout(std430, binding = 0) buffer mesh                                                \n"
    "{                                                                                      \n"
    "    vertex vertex[];                                                                   \n"
    "} Mesh;                                                                                \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = Mesh.vertex[gl_VertexID].texcoord;                                    \n"
    "    gl_Position = u_worldscreen * vec4(Mesh.vertex[gl_VertexID].position, 0.0, 1.0);    \n"
    "}                                                                                      \n"
};
	
const char *c_frag = {
    "#version 420 core                                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "layout(binding = 0, rgba8) uniform coherent image2D u_color;                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    imageStore(u_color, ivec2(gl_FragCoord.xy), texture(u_diffuse, f_texcoord));       \n"
    "    //imageStore(u_color, ivec2(gl_FragCoord.xy), vec4(f_texcoord, 1, 1));             \n"
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
        "    color = texture(u_diffuse, gl_FragCoord.xy * " e_fb_scale " / texture_size); "
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[2];  // 0:diffuse 1:colorbuffer
	uint32_t m_samplerIds[2];  // 0:render: 1:splash
	uint32_t m_pipeIds[2];
	SpuShader m_shaders[2];
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_color;
	uint32_t u_sampler;
	uint32_t m_arrayIds[2];
	uint32_t m_frameId;

	int32_t m_supersampling;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_supersampling = atoi(e_fb_scale);
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
				        {"u_worldscreen", &u_worldscreen}, // UBO
				        {"u_diffuse",     &u_diffuse    },
				        {"u_diffuse",     &u_sampler    },
				        {"u_color",       &u_color      },
				};
				loadShader(m_shaders[i], shader_attrs, unif_attrs);
			}
		}
		// array #0
		{
			const std::vector<uint16_t> c_indices = {0, 1, 2, 2, 3, 0};

			Attrs attrs0 = {
			        {"shader_id", m_shaders[0].id()},
			        {"nelem",     4                },
			};

			Attrs attrs1 = {
			        {"a.mesh", 0}, // must be zero : (not good)
			};
			m_arrayIds[0] = spu_array_new(attrs0);

			spu_array_aux(m_arrayIds[0], attrs1, 1);
			spu_array_send(m_arrayIds[0], c_indices.data(), c_indices.size(), -1, 2);

			const std::vector<v2fv2f_t> c_vertices = squareQuads<v2fv2f_t>();
			spu_array_send(
			        m_arrayIds[0], c_vertices.data(), c_vertices.size() * sizeof(c_vertices[0]), 1);
		}

		// array #1
		{
			const std::vector<uint16_t> c_indices = {0, 1, 2, 2, 3, 0};
			Attrs attrs0 = {
			        {"nelem", 4},
			};

			m_arrayIds[1] = spu_array_new(attrs0);
			spu_array_send(m_arrayIds[1], c_indices.data(), c_indices.size(), -1, 2);
		}

		// texture #0
		{
			Attrs attrs = {
			        {"mag_filter", GL_NEAREST               },
			        {"min_filter", GL_NEAREST_MIPMAP_NEAREST},
			};
			m_textureIds[0] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}

		// texture #1
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D                            },
			        {"iformat",     GL_RGBA8                                 },
			        {"width",       int32_t(viewport(0).sx * m_supersampling)},
			        {"height",      int32_t(viewport(0).sy * m_supersampling)},
			        {"base_level",  0                                        },
			        {"max_level",   0                                        },
			        {"auto_mipmap", 0                                        },
			};
			m_textureIds[1] = spu_texture_new(attrs);
		}

		// sampler
		{
			Vec4f border = {0.5, 0.5, 0.5, 0};
			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER     },
			        {"wrap_s",       GL_CLAMP_TO_EDGE       },
			        {"wrap_t",       GL_CLAMP_TO_EDGE       },
			        {"wrap_r",       GL_CLAMP_TO_EDGE       },
			        {"min_filter",   GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",   GL_LINEAR              },
			        {"min_lod",      -1000.0                },
			        {"max_lod",      +1000.0                },
			        {"lod_bias",     0.0                    },
			        {"compare_mode", GL_NONE                },
			        {"compare_func", GL_LEQUAL              },
			        {"border",       border                 },
			        {"max_aniso",    16.0                   },
			};
			m_samplerIds[0] = spu_texture_new(attrs);

			attrs.replace("min_filter", GL_LINEAR);
			attrs.replace("mag_filter", GL_LINEAR);
			m_samplerIds[1] = spu_texture_new(attrs);
		}

		// framebuffer
		{
			auto scaled_viewport = Rectf(
			        0, 0, viewport(0).sx * m_supersampling, viewport(0).sy * m_supersampling);

			Attrs attrs = {
			        {"viewport0",                      scaled_viewport            }, // necessary
			        {"default_width",                  int32_t(scaled_viewport.sx)},
			        {"default_height",                 int32_t(scaled_viewport.sy)},
			        {"default_layers",                 1                          },
			        {"default_fixed_sample_locations", 1                          },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		// for debug
		clearImage();

		// Render
		spu_frame_begin(m_frameId);

		u_diffuse = m_textureIds[0];
		u_sampler = m_samplerIds[0];
		u_color = m_textureIds[1];

		m_shaders[0].use();

		spu_array_draw(m_arrayIds[0], GL_TRIANGLES);

		spu_frame_end();

		// Splash
		u_diffuse = m_textureIds[1];
		u_sampler = m_samplerIds[1];
		m_shaders[1].use();

		spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
	}

	void clearImage()
	{
		auto pix = 0xff007fffu;  // 1.0, 0.5, 0.0, 1.0
		spu_texture_send(m_textureIds[1], &pix, GL_RGBA8, nullptr, nullptr, true);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_440_fbo_without_attachment");
}  // namespace
}  // namespace spu
