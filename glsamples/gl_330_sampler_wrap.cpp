//
// App :
//
#include "base_app.h"
#include "gl_330_texture_2d.h"
namespace spu {
namespace {

class App : public BaseApp {
public:
	uint32_t m_samplerIds[4];
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_sampler;

	std::vector<Rectf> m_viewports;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

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
			        {"u_diffuse",     &u_sampler    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<v2fv2f_t> c_vertices = {
			        {-1.5, -1.5, -2.0, +2.0},
                                {+1.5, -1.5, +2.0, +2.0},
                                {+1.5, +1.5, +2.0, -2.0},
			        {+1.5, +1.5, +2.0, -2.0},
                                {-1.5, +1.5, -2.0, -2.0},
                                {-1.5, -1.5, -2.0, +2.0}
                        };

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
			int32_t wrap[] = {
			        GL_MIRRORED_REPEAT,
			        GL_CLAMP_TO_BORDER,
			        GL_REPEAT,
			        GL_CLAMP_TO_EDGE,
			};

			for (auto i = 0; i < 4; i++) {
				Attrs attrs = {
				        {"target",     GL_TEXTURE_SAMPLER     },
				        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
				        {"mag_filter", GL_LINEAR              },
				        {"wrap_s",     wrap[i]                },
				        {"wrap_t",     wrap[i]                },
				        {"wrap_r",     wrap[i]                },
				        {"border",     border                 },
				};
				m_samplerIds[i] = spu_texture_new(attrs);
			}
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();

		for (auto i = 0; i < 4; i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);
			u_sampler = m_samplerIds[i];
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_sampler_wrap");
}  // namespace
}  // namespace spu
