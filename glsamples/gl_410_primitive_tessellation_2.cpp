//
// App :
//
#include "base_app.h"
#include "gl_410_primitive_tessellation.h"
namespace spu {
namespace {

class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
                                {"tesc", c_tesc},
                                {"tese", c_tese},
			        {"geom", c_geom},
                                {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<v2fc4f_t> c_vertices = {
			        {-1.0, -1.0, 1.0, 0.0, 0.0, +1.0},
			        {+1.0, -1.0, 1.0, 1.0, 0.0, +1.0},
			        {+1.0, +1.0, 0.0, 1.0, 0.0, +1.0},
			        {-1.0, +1.0, 0.0, 0.0, 1.0, +1.0}
                        };

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_color",    4                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
			spu_array_set(m_arrayId, "patch_vertices", c_vertices.size());
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.fill = false;
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_PATCHES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_410_primitive_tessellation_2");
}  // namespace
}  // namespace spu
