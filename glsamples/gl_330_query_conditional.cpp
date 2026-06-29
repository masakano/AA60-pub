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
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = u_diffuse;                                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Vec4f u_diffuse;

	uint32_t m_queryId;
	uint32_t m_arrayId;
	ConditionalDraw m_cdraw;

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
			m_queryId = spu_query_new((Attrs){
			        {"target", GL_ANY_SAMPLES_PASSED}
                        });
		}

		// conditonal draw
		m_cdraw.init(m_arrayId, m_queryId);
	}

	void render() override
	{
		// change mode
		static auto query_mode = 1;
		spu_printf(0, "hit \"l\" to toggle query (%d)\n", query_mode);
		if (getGesture()->pressed('l')) {
			query_mode = (query_mode + 1) & 0x1;
		}

		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		// Set the display viewport
		spu_frame_set(-1, "bgcolor0", c_black);

		spu_frame_clear(-1);

		// samples count query
		u_diffuse = Vec4f(0.0, 0.5, 1.0, +1.0);
		m_shader.use();
		m_cdraw.probe(query_mode != 0);

		u_diffuse = Vec4f(1.0, 0.5, 0.0, +1.0);
		m_shader.use();

		// Draw only if one sample went through the tests,
		// we don't need to get the query result which prevent the rendering pipeline to
		// stall.
		m_cdraw.begin();

		spu_frame_set(-1, "bgcolor0", c_gray);

		spu_frame_clear(-1);

		spu_array_draw(m_arrayId, GL_TRIANGLES);
		m_cdraw.end();
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_query_conditional");
}  // namespace
}  // namespace spu
