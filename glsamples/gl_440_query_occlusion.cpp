//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "#extension GL_ARB_shader_storage_buffer_object : require                               \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "layout(std430, binding = 0) buffer mesh                                                \n"
    "{                                                                                      \n"
    "    vec2 positions[];                                                                  \n"
    "} Mesh;                                                                                \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(Mesh.positions[gl_VertexID], 0.0, 1.0);          \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0, 0.5, 0.0, 1.0);                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	std::vector<Rectf> m_viewports;

	uint32_t m_arrayId;
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t m_queryId[4];

	App(const char *name) : BaseApp(name) {}

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
			        {"u_worldscreen", &u_worldscreen}, // UBO
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<vec2sf_t> c_vertices = squareTriangles();

			Attrs attrs0 = {
			        {"shader_id", m_shader.id()},
			        {"nelem",     6            },
			};
			Attrs attrs1 = {
			        {"a.mesh", 0},
			};

			m_arrayId = spu_array_new(attrs0);
			spu_array_aux(m_arrayId, attrs1, 1);
			spu_array_send(
			        m_arrayId, c_vertices.data(), c_vertices.size() * sizeof(c_vertices[0]), 1);
		}
		// array
		{
			Attrs attrs = {
			        {"target", GL_ANY_SAMPLES_PASSED},
			};
			for (auto &i: m_queryId) {
				i = spu_query_new(attrs);
			}
		}
	}

#define e_buffer_uint_offset(i) ((uint32_t *)NULL + (i))

	void render() override
	{
		uint64_t sample_counts[4];
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		// Clear color buffer with black
		// Samples count query

		m_shader.use();
		for (auto i = 0; i < 4; i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			spu_query_begin(m_queryId[i]);
			spu_array_draw(m_arrayId, GL_TRIANGLES);
			spu_query_end(m_queryId[i], &sample_counts[i], true);
		}
		spu_frame_set(-1, "viewport0", viewport(0));

		spu_printf(
		        0, "Any samples Passed = %d %d %d %d\n", sample_counts[0], sample_counts[1],
		        sample_counts[2], sample_counts[3]);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_440_query_occlusion");
}  // namespace
}  // namespace spu
