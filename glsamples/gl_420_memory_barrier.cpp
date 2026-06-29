//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "const vec2 positions[3] = vec2[](                                                      \n"
    "    vec2(-1.0f,-1.0f),                                                                 \n"
    "    vec2( 3.0f,-1.0f),                                                                 \n"
    "    vec2(-1.0f, 3.0f));                                                                \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);                              \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texelFetch(u_diffuse, ivec2(gl_FragCoord.xy), 0) - 1.0 / 255;              \n"
    "}                                                                                      \n"
};

const char *c_vert_blit = {
    "#version 420 core                                                                      \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "const vec2 positions[3] = vec2[](                                                      \n"
    "    vec2(-1.0f,-1.0f),                                                                 \n"
    "    vec2( 3.0f,-1.0f),                                                                 \n"
    "    vec2(-1.0f, 3.0f));                                                                \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);                              \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 window_size = vec2(640,480);                                                  \n"
    "    vec2 texcoord = gl_FragCoord.xy / window_size;                                     \n"
    "    color = texture(u_diffuse, texcoord);                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shaders[2];    // 0;update 1:blit
	uint32_t m_textureIds[2];  // 0:diffuse 1:colorbuffer
	uint32_t m_samplerId;
	uint32_t m_diffuseId;
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
				        {"u_diffuse", &m_diffuseId},
				        {"u_diffuse", &m_samplerId},
				};
				loadShader(m_shaders[i], shader_attrs, unif_attrs);
			}
		}
		// array
		{
			Attrs attrs = {
			        {"a.0",   1},
			        {"nelem", 3},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// texture
		{
			auto width = 0;
			auto height = 0;

			Attrs attrs_diffuse = {
			        {"mag_filter", GL_NEAREST},
			        {"min_filter", GL_NEAREST},
			};
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_unorm.dds", attrs_diffuse);

			spu_texture_get(m_textureIds[0], "width", &width);
			spu_texture_get(m_textureIds[0], "height", &height);

			Attrs attrs_colorbuffer = {
			        {"target",      GL_TEXTURE_2D},
                                {"iformat",     GL_RGBA8     },
                                {"width",       width        },
			        {"height",      height       },
                                {"base_level",  0            },
                                {"max_level",   0            },
			        {"auto_mipmap", 0            },
			};
			m_textureIds[1] = spu_texture_new(attrs_colorbuffer);

			auto viewport = Rectf(0, 0, width, height);
			Attrs attrs = {
			        {"color0",    m_textureIds[1]},
			        {"viewport0", viewport       },
			};
			m_frameId = spu_frame_new(attrs);
		}
		// sampler
		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_SAMPLER},
                                {"wrap_s",     GL_CLAMP_TO_EDGE  },
			        {"wrap_t",     GL_CLAMP_TO_EDGE  },
                                {"wrap_r",     GL_CLAMP_TO_EDGE  },
			        {"min_filter", GL_NEAREST        },
                                {"mag_filter", GL_NEAREST        },
			};
			m_samplerId = spu_texture_new(attrs);
		}
	}

	void render() override
	{
		static auto frame_index = 0;

		// Update a colorbuffer bound as a framebuffer attachement and as a texture
		spu_frame_begin(m_frameId);

		m_diffuseId = frame_index != 0 ? m_textureIds[1] : m_textureIds[0];

		m_shaders[0].use();

		// is this working?
		spu_graphics_memory_barrier(GL_TEXTURE_UPDATE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		spu_array_draw(m_arrayId, GL_TRIANGLES);

		// Blit to framebuffer
		spu_frame_end();

		m_diffuseId = m_textureIds[1];
		m_shaders[1].use();
		spu_graphics_memory_barrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
		spu_array_draw(m_arrayId, GL_TRIANGLES);

		frame_index = (frame_index + 1) % 256;

		spu_printf(0, "frame = %d\n", frame_index);
	}

	void clearImage()
	{
		auto pix = 0xff0000ffu;  // red
		spu_texture_send(m_textureIds[1], &pix, 0, nullptr, nullptr, true);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_memory_barrier");
}  // namespace
}  // namespace spu
