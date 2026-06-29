//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 400 core                                                                      \n"
    "const int vertexCount = 3;                                                             \n"
    "const vec2 positions[vertexCount] = vec2[](                                            \n"
    "    vec2(-1.0,-1.0),                                                                   \n"
    "    vec2( 3.0,-1.0),                                                                   \n"
    "    vec2(-1.0, 3.0));                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);                              \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
    "uniform sampler2DArray u_diffuse;                                                      \n"
    "uniform int u_layer;                                                                   \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 texture_size = vec2(textureSize(u_diffuse, 0).xy);                            \n"
    "    color = texture(u_diffuse, vec3(gl_FragCoord.xy / texture_size, u_layer));         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	std::vector<Rectf> m_viewports;
	uint32_t u_diffuse;
	uint32_t u_layer;
	uint32_t m_arrayId;
	uint32_t m_frameId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		auto fb_viewport = Rectf(0, 0, viewport(0).sx / 2, viewport(0).sy / 2);
		auto width = int32_t(fb_viewport.sx);
		auto height = int32_t(fb_viewport.sy);

		m_viewports = makeViewports(2, 2);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_diffuse", &u_diffuse},
			        {"u_layer",   &u_layer  },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// array
		{
			Attrs attrs = {
			        {"nelem", 3},
			};
			m_arrayId = spu_array_new(attrs);
		}

		// texture
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_ARRAY},
			        {"base_level",  0                  },
			        {"max_level",   0                  },
			        {"auto_mipmap", 0                  },
			        {"min_filter",  GL_NEAREST         },
			        {"mag_filter",  GL_NEAREST         },
			        {"iformat",     GL_RGBA8           },
			        {"width",       width              },
			        {"height",      height             },
			        {"depth",       3                  },
			};

			u_diffuse = spu_texture_new(attrs);
		}

		// frame
		{
			auto half_viewport = Rectf(0, 0, viewport(0).sx / 2, viewport(0).sy / 2);

			const std::vector<Vec4f> c_colors = {
			        {1.0, 0.0, 0.0, 1.0},
			        {0.0, 1.0, 0.0, 1.0},
			        {0.0, 0.0, 1.0, 1.0},
			        {1.0, 1.0, 0.0, 1.0},
			};

			Attrs attrs = {
			        {"color0",       u_diffuse    },
                                {"color0.layer", 0            },
			        {"color1",       u_diffuse    },
                                {"color1.layer", 1            },
			        {"color2",       u_diffuse    },
                                {"color2.layer", 2            },
			        {"viewport0",    half_viewport},
                                {"bgcolor0",     c_colors[0]  },
			        {"bgcolor1",     c_colors[1]  },
                                {"bgcolor2",     c_colors[2]  },
			        {"bgcolor3",     c_colors[3]  },
			};
			m_frameId = spu_frame_new(attrs);
		}
	}

	void render() override
	{
		spu_frame_clear(m_frameId);
		for (auto i = 0; i < 3; i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			u_layer = i;
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_fbo_rtt_texture_array");
}  // namespace
}  // namespace spu
