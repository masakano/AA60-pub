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
    "    gl_Position =                                                                      \n"
    "      vec4(4*(gl_VertexID%2)-1, 4*(gl_VertexID/2)-1, 0, 1);                            \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse,                                                         \n"
    "              vec2(gl_FragCoord.x, 1.0 - gl_FragCoord.y) / vec2(640 - 1, 480 - 1));    \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_sampler;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"ub_transform", &u_worldscreen},
			        {"u_diffuse",    &u_diffuse    },
			        {"u_diffuse",    &u_sampler    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs);
		}

		// texture
		{
			u_diffuse = loadDDS("kueken7_rgb_dxt1_unorm.dds");
		}
		// sampler
		{
			Vec4f border = {0, 0, 0, 0};
			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER},
			        {"min_lod",      -1000.0           },
			        {"max_lod",      +1000.0           },
			        {"lod_bias",     0.0               },
			        {"border",       border            },
			        {"compare_mode", GL_NONE           },
			        {"compare_func", GL_LEQUAL         },
			        {"min_filter",   GL_LINEAR         },
			        {"mag_filter",   GL_LINEAR         },
			};
			u_sampler = spu_texture_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_draw_image_space_rendering");
}  // namespace
}  // namespace spu
