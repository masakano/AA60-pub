//
// App :
//
#include "base_app.h"
#include "gl_330_texture_2d.h"
namespace spu {
namespace {

class App : public BaseApp {
public:
	uint32_t m_swizzleR[4];
	uint32_t m_swizzleG[4];
	uint32_t m_swizzleB[4];
	uint32_t m_swizzleA[4];

	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;

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

			m_swizzleR[0] = GL_RED;
			m_swizzleG[0] = GL_GREEN;
			m_swizzleB[0] = GL_BLUE;
			m_swizzleA[0] = GL_ALPHA;

			m_swizzleR[1] = GL_BLUE;
			m_swizzleG[1] = GL_GREEN;
			m_swizzleB[1] = GL_RED;
			m_swizzleA[1] = GL_ALPHA;

			m_swizzleR[2] = GL_ONE;
			m_swizzleG[2] = GL_GREEN;
			m_swizzleB[2] = GL_BLUE;
			m_swizzleA[2] = GL_ALPHA;

			m_swizzleR[3] = GL_ZERO;
			m_swizzleG[3] = GL_GREEN;
			m_swizzleB[3] = GL_BLUE;
			m_swizzleA[3] = GL_ALPHA;
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();

		for (auto i = 0; i < 4; i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			Attrs attrs = {
			        {"swizzle_r", m_swizzleR[i]},
			        {"swizzle_g", m_swizzleG[i]},
			        {"swizzle_b", m_swizzleB[i]},
			        {"swizzle_a", m_swizzleA[i]},
			};
			spu_texture_set(u_diffuse, attrs);
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_texture_swizzle");
}  // namespace
}  // namespace spu
