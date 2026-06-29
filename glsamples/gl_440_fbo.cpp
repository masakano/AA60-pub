//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

#define e_fb_scale "2"  // string

uint8_t c_clear_color[][4] = {
        {0xff, 0x7f, 0x00, 0xff},
        {0x00, 0xff, 0x7f, 0xff},
        {0x7f, 0x00, 0xff, 0xff},
        {0x00, 0x7f, 0xff, 0xff},
        {0xff, 0x00, 0x7f, 0xff},
        {0x7f, 0xff, 0x00, 0xff},
};

int32_t c_clear_loc[][4] = {
        {0,   0,   0, 0},
        {64,  64,  0, 0},
        {256, 16,  0, 0},
        {128, 384, 0, 0},
        {512, 256, 0, 0},
        {432, 372, 0, 0},
};

uint32_t c_clear_size[][4] = {
        {0,   0,   0, 0},
        {64,  64,  1, 1},
        {128, 128, 1, 1},
        {64,  64,  1, 1},
        {64,  64,  1, 1},
        {64,  64,  1, 1},
};

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
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
    "    gl_Position = u_worldscreen *                                                       \n"
    "     vec4(a_position.xy, a_position.z + float(gl_InstanceID) * 0.5 - 0.25, 1.0);       \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "        color = texture(u_diffuse, f_texcoord.st);                                     \n"
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
//    "    color = vec4(1); "
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_frameId;
	uint32_t m_textureIds[3];  // 0:diffuse 1:colorbuffer 2:renderbuffer
	uint32_t m_arrayIds[2];    // 0:texture 1:splash
	SpuShader m_shaders[2];

	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			const char *vert_src[] = {
			        c_vert,
			        c_vert_blit,
			};

			const char *frag_src[] = {
			        c_frag,
			        c_frag_blit,
			};

			for (auto i = 0; i < 2; i++) {
				Attrs shader_attrs = {
				        {"vert", vert_src[i]},
				        {"frag", frag_src[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen}, // UBO
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
			        {"nelem", 4},
			};
			m_arrayIds[1] = spu_array_new(attrs);
		}

		// texture #0
		{
			Attrs attrs = {
			        {"mag_filter", GL_NEAREST               },
			        {"min_filter", GL_NEAREST_MIPMAP_NEAREST},
			};
			m_textureIds[0] = loadDDS("kueken7_rgb_dxt1_srgb.dds", attrs);
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

		// texture #2
		{
			Attrs attrs = {
			        {"target",      GL_RENDERBUFFER        },
			        {"iformat",     GL_DEPTH_COMPONENT32F  },
			        {"width",       int32_t(viewport(0).sx)},
			        {"height",      int32_t(viewport(0).sy)},
			        {"base_level",  0                      },
			        {"max_level",   0                      },
			        {"auto_mipmap", 0                      },
			};
			m_textureIds[2] = spu_texture_new(attrs);
		}

		// framebuffer
		{
			Attrs attrs = {
			        {"color0",  m_textureIds[1]},
			        {"depth",   m_textureIds[2]},
			        {"bgcolor", c_white.f      },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 180.0 * 0.25F, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.use();

		spu_frame_begin(m_frameId);
		spu_frame_clear(m_frameId);

		for (auto i = 0; i < 6; i++) {
			if (c_clear_loc[i][0] + c_clear_size[i][0] < viewport(0).sx
			    && c_clear_loc[i][1] + c_clear_size[i][1] < viewport(0).sy) {
				spu_texture_send(
				        m_textureIds[1], c_clear_color[i], GL_RGBA8, c_clear_loc[i],
				        c_clear_size[i], true);
			}
		}

		u_diffuse = m_textureIds[0];
		m_shaders[0].use();
		spu_array_draw(m_arrayIds[0], GL_TRIANGLES, 0, 0, 2);

		spu_frame_end();

		renderstate.flags.depth_test = false;
		renderstate.use();
		u_diffuse = m_textureIds[1];
		m_shaders[1].use();
		spu_array_draw(m_arrayIds[1], GL_TRIANGLES, 0, 3);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_440_fbo");
}  // namespace
}  // namespace spu
