//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * a_position;                                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330 core                                                                      \n"
    "uniform vec4 u_diffuse;                                                                \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = u_diffuse;                                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_queryId0;
	uint32_t m_queryId1;
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Vec4f u_diffuse;
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
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<vec2sf_t> c_vertices = squareTriangles();

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
			        {"a.a_position", 2                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// query
		{
			m_queryId0 = spu_query_new((Attrs){
			        {"target", GL_TIMESTAMP}
                        });
			m_queryId1 = spu_query_new((Attrs){
			        {"target", GL_TIMESTAMP}
                        });
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		u_diffuse = Vec4f(1.0, 0.5, 0.0, +1.0);
		m_shader.use();

		spu_query_begin(m_queryId0);
		spu_array_draw(m_arrayId, GL_TRIANGLES);
		spu_query_begin(m_queryId1);

		uint64_t time_begin;
		uint64_t time_end;

		spu_query_end(m_queryId0, &time_begin, true);
		spu_query_end(m_queryId1, &time_end, true);

		spu_printf(
		        0, "Time stamp: %08x - %08x = %d us \n", time_end, time_begin, (time_end - time_begin));
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_query_counter");
}  // namespace
}  // namespace spu
