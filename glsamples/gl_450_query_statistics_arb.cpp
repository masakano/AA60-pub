//
// App :
//
#include "base_app.h"
namespace spu {
namespace {
/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2DArray u_diffuse;                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, vec3(f_texcoord.st, 0.0));                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	uint32_t m_arrayId;
	uint32_t u_diffuse;
	Mat4f u_worldscreen;

	struct Query {
		uint32_t target;
		uint32_t query_id;
		uint64_t result;
	};

	std::vector<Query> m_queries;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// query
		{
			m_queries = {
			        {GL_VERTICES_SUBMITTED,                 0, 0},
			        {GL_PRIMITIVES_SUBMITTED,               0, 0},
			        {GL_VERTEX_SHADER_INVOCATIONS,          0, 0},
			        {GL_TESS_CONTROL_SHADER_PATCHES,        0, 0},
			        {GL_TESS_EVALUATION_SHADER_INVOCATIONS, 0, 0},
			        {GL_GEOMETRY_SHADER_INVOCATIONS,        0, 0},
			        {GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED, 0, 0},
			        {GL_FRAGMENT_SHADER_INVOCATIONS,        0, 0},
			        {GL_COMPUTE_SHADER_INVOCATIONS,         0, 0},
			        {GL_CLIPPING_INPUT_PRIMITIVES,          0, 0},
			        {GL_CLIPPING_OUTPUT_PRIMITIVES,         0, 0},
			};

			for (auto &q: m_queries) {
				q.query_id = spu_query_new({
				        {"target", q.target}
                                });
			}
		}
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
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs);
		}
		// texture
		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D_ARRAY    },
			        {"mag_filter", GL_LINEAR              },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"swizzle_r",  GL_RED                 },
			        {"swizzle_g",  GL_GREEN               },
			        {"swizzle_b",  GL_BLUE                },
			        {"swizzle_a",  GL_ALPHA               },
			};
			u_diffuse = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(
		        viewport(0), 180.0 * 0.25F, /*float(viewport(0).sx) / viewport(0).sy,*/ 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		m_shader.use();

		for (auto &q: m_queries) {
			spu_query_begin(q.query_id);
		}

		spu_array_draw(m_arrayId, GL_TRIANGLES);

		for (auto &q: m_queries) {
			spu_query_end(q.query_id, &q.result, true /*0*/);
			spu_printf(0, "%-36s:%-5d ", opengl_const(q.target) + 3, q.result);
			if ((&q - &m_queries[0]) % 2 == 1) {
				spu_printf(0, "\n");
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_450_query_statistics_arb");
}  // namespace
}  // namespace spu
