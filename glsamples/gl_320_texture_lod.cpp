//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

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
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2DArray u_diffuse;                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, vec3(f_texcoord, 0.0));                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[4];
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_arrayId;
	std::vector<Rectf> m_viewports;

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 2);
		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>();

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// texture
		{
			float min_lod[] = {
			        0.0,
			        3.0,
			        0.0,
			        0.0,
			};

			float max_lod[] = {
			        0.0,
			        3.0,
			        10.0,
			        10.0,
			};

			float lod_bias[] = {4.0, 0.0, 4.0, 3.5};

			for (auto i = 0u; i < 4; i++) {
				Attrs attrs = {
				        {"target",     GL_TEXTURE_2D_ARRAY    },
				        {"iformat",    GL_SRGB8_ALPHA8        },
				        {"depth",      1                      },
				        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
				        {"mag_filter", GL_LINEAR              },
				        {"min_lod",    min_lod[i]             },
				        {"max_lod",    max_lod[i]             },
				        {"lod_bias",   lod_bias[i]            },
				};
				m_textureIds[i] = loadDDS("kueken7_rgba8_srgb.dds", attrs);
			}
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();

		for (auto i = 0; i < 4; i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			u_diffuse = m_textureIds[i];
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_texture_lod");
}  // namespace
}  // namespace spu
