//
// App :
//
#include "base_app.h"
#include "gl_330_texture_2d.h"
namespace spu {
namespace {

class App : public BaseApp {
public:
	uint32_t m_samplerId[4];
	std::vector<Rectf> m_scissors;

	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_sampler;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_scissors = makeViewports(2, 2, 1);

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
			u_diffuse = spu_inventory_new("texture", "kanji.jpg", Attrs());
			spu_inventory_sync(u_diffuse, false);
		}
		// sampler
		{
			int32_t min_filter[] = {
			        GL_NEAREST,
			        GL_LINEAR,
			        GL_LINEAR_MIPMAP_NEAREST,
			        GL_LINEAR_MIPMAP_LINEAR,
			};
			int32_t mag_filter[] = {
			        GL_NEAREST,
			        GL_LINEAR,
			        GL_LINEAR,
			        GL_LINEAR,
			};

			for (auto i = 0; i < 4; i++) {
				Attrs attrs = {
				        {"target",     GL_TEXTURE_SAMPLER},
				        {"min_filter", min_filter[i]     },
				        {"mag_filter", mag_filter[i]     },
				};
				m_samplerId[i] = spu_texture_new(attrs);
			}
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();

		for (auto i = 0; i < 4; i++) {
			spu_frame_set(-1, "scissor0", m_scissors[i]);
			u_sampler = m_samplerId[i];
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
		spu_frame_set(-1, "scissor0", ezero<Vec4f>());
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_sampler_filter");
}  // namespace
}  // namespace spu
