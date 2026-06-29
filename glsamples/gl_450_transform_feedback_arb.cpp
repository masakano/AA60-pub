//
// App :
//
#include "base_app.h"
namespace spu {
namespace {
/* clang-format off */
const char *c_vert_transform = {
    "#version 440 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec4 a_position;                                                                    \n"
    "out block                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} Out;                                                                                 \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * a_position;                                           \n"
    "    Out.color = vec4(clamp(vec2(a_position), 0.0, 1.0), 0.0, 1.0);                     \n"
    "}                                                                                      \n"
};
	
const char *c_frag = {
    "#version 440 core                                                                      \n"
    "in block                                                                               \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} In;                                                                                  \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = In.color;                                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	// static const auto xfb_size = 6;
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t m_arrayId;
	uint32_t m_queryId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert_transform},
			        {"frag", c_frag          },
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// array
		{
			const std::vector<Vec4f> c_vertices = squareTriangles<Vec4f>();
			Attrs attr = {
			        {"shader_id",    m_shader.id()    },
			        {"a.a_position", 4                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attr);
		}

		// query
		{
			Attrs attrs = {
			        {"target", GL_PRIMITIVES_SUBMITTED},
			};
			m_queryId = spu_query_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();

		spu_query_begin(m_queryId);
		spu_array_draw(m_arrayId, GL_TRIANGLES);

		auto primitive_count = uint64_t(0);
		spu_query_end(m_queryId, &primitive_count, false);

		spu_printf(0, "primitve count = %d\n", primitive_count);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_450_transform_feedback_arb");
}  // namespace
}  // namespace spu
