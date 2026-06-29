//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                          \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture2D(u_diffuse, f_texcoord);                                          \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	std::vector<uint32_t> m_samplerIds;
	std::vector<Rectf> m_viewports;

	uint32_t m_arrayId;

	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_sampler;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(3, 2);
		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen}, // UBO
			        {"u_diffuse",     &u_diffuse    },
			        {"u_diffuse",     &u_sampler    },
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
			u_diffuse = loadDDS("kueken7_rgba_dxt5_unorm.dds", Attrs());
		}
		// sampler
		{
			Vec4f border = {0.0, 0.5, 1.0, 1.0};
			const std::vector<int32_t> c_wraps = {
			        GL_REPEAT,          GL_CLAMP_TO_EDGE,        GL_CLAMP_TO_BORDER,
			        GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, GL_MIRROR_CLAMP_TO_BORDER_EXT,
			};

			for (const auto &wrap: c_wraps) {
				Attrs attrs = {
				        {"target",     GL_TEXTURE_SAMPLER     },
				        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
				        {"mag_filter", GL_LINEAR              },
				        {"wrap_s",     wrap                   },
				        {"wrap_t",     wrap                   },
				        {"wrap_r",     wrap                   },
				        {"border",     border                 },
				};
				m_samplerIds.push_back(spu_texture_new(attrs));
			}
		}
	}

	void render() override
	{
		for (auto i = 0u; i < m_samplerIds.size(); i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
			u_worldscreen = getCamera().worldscreen() * Mat4f();
			u_sampler = m_samplerIds[i];
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_440_sampler_wrap");
}  // namespace
}  // namespace spu
