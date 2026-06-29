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
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0, 0.5, 0.0, 1.0);                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_arrayId;
	uint32_t m_queryId;
	SpuShader m_shader;
	Mat4f u_worldscreen;

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
			Attrs attrs = {
			        {"target", GL_ANY_SAMPLES_PASSED},
			};
			m_queryId = spu_query_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		// Set the display viewport

		// Clear color buffer with black

		m_shader.use();

		spu_query_begin(m_queryId);
		spu_array_draw(m_arrayId, GL_TRIANGLES);

		uint64_t sample_count;
		spu_query_end(m_queryId, &sample_count, true);
		spu_printf(0, "Any Sample Passed =  %d\n", sample_count);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_query_occlusion");
}  // namespace
}  // namespace spu
