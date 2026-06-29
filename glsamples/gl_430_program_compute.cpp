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
    "struct vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 position;                                                                     \n"
    "    vec4 texcoord;                                                                     \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "layout(binding = 0) buffer mesh                                                        \n"
    "{                                                                                      \n"
    "    vertex vertex[];                                                                   \n"
    "} Mesh;                                                                                \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec4 f_texcoord;                                                                   \n"
    "out vec4 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = Mesh.vertex[gl_VertexID].texcoord;                                    \n"
    "    if(gl_VertexID % 2 != 0)                                                           \n"
    "        f_color = vec4(1.0);                                                           \n"
    "    else                                                                               \n"
    "        f_color = Mesh.vertex[gl_VertexID].color;                                      \n"
    "    gl_Position = Mesh.vertex[gl_VertexID].position;                                   \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_albedo;                                                            \n"
    "in vec4 f_texcoord;                                                                    \n"
    "in vec4 f_color;                                                                       \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_albedo, f_texcoord.st) * f_color;                                \n"
    "}                                                                                      \n"
};

const char *c_comp = {
    "#version 420 core                                                                      \n"
    "#extension GL_ARB_compute_shader : require                                             \n"
    "#extension GL_ARB_shader_storage_buffer_object : require                               \n"
    "layout (local_size_x = 4) in;                                                          \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "struct vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 position;                                                                     \n"
    "    vec4 texcoord;                                                                     \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "layout(binding = 0) readonly buffer iBuffer                                            \n"
    "{                                                                                      \n"
    "    vertex input[];                                                                    \n"
    "} In;                                                                                  \n"
    "layout(binding = 1) writeonly buffer oBuffer                                           \n"
    "{                                                                                      \n"
    "    vertex ouput[];                                                                    \n"
    "} Out;                                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    Out.ouput[gl_LocalInvocationIndex].position =                                      \n"
    "           u_worldscreen * In.input[gl_LocalInvocationIndex].position;                  \n"
    "    Out.ouput[gl_LocalInvocationIndex].texcoord =                                      \n"
    "           In.input[gl_LocalInvocationIndex].texcoord;                                 \n"
    "    Out.ouput[gl_LocalInvocationIndex].color =                                         \n"
    "           In.input[gl_LocalInvocationIndex].color;                                    \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	std::vector<v4fv4fv4f_t> m_vertices = {
	        {-1.0, -1.0, 0.0, +1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, +1.0},
	        {+1.0, -1.0, 0.0, +1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, +1.0},
	        {+1.0, +1.0, 0.0, +1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, +1.0},
	        {-1.0, +1.0, 0.0, +1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, +1.0},
	        {-1.0, -1.0, 0.0, +1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.5, 0.5, +1.0},
	        {+1.0, -1.0, 0.0, +1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.5, +1.0},
	        {+1.0, +1.0, 0.0, +1.0, 1.0, 0.0, 0.0, 0.0, 0.5, 1.0, 0.0, +1.0},
	        {-1.0, +1.0, 0.0, +1.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.5, 1.0, +1.0},
	};

	std::vector<uint16_t> m_indices = {0, 1, 2, 2, 3, 0};

	uint32_t m_computeId;
	uint32_t m_arrayId;

	SpuShader m_compShader;
	SpuShader m_drawShader;

	Mat4f u_worldscreen;
	uint32_t u_albedo;

	uint32_t m_iBufferId;  // debug
	uint32_t m_oBufferId;  // debug

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program draw
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_albedo",      &u_albedo     },
			};
			loadShader(m_drawShader, shader_attrs, unif_attrs);
		}

		// program compute
		{
			Attrs shader_attrs = {
			        {"comp", c_comp},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			};
			loadShader(m_compShader, shader_attrs, unif_attrs);
		}

		// array compute
		{
			Attrs attrs = {
			        {"shader_id", m_compShader.id()                        },
			        {"a.iBuffer", 0                                        },
			        {"data",      m_vertices.data()                        },
			        {"nelem",     m_vertices.size() * sizeof(m_vertices[0])},
			};

			m_computeId = spu_array_new(attrs);

			Attrs buffer_attrs = {
			        {"a.oBuffer", 0                                        },
			        {"nelem",     m_vertices.size() * sizeof(m_vertices[0])},
			};
			spu_array_aux(m_computeId, buffer_attrs, 1);
		}

		// array draw
		{
			Attrs attrs = {
			        {"shader_id", m_drawShader.id()},
			        {"nelem",     1                },
			};

			m_arrayId = spu_array_new(attrs);

			Attrs buffer_attrs = {
			        {"a.mesh", 0},
			};

			spu_array_aux(m_arrayId, buffer_attrs, 1);
			spu_array_link(m_arrayId, m_computeId, 1, 1);
			spu_array_send(m_arrayId, m_indices.data(), m_indices.size(), -1, 2);
		}

		spu_array_get(m_computeId, "0.buffer_id", &m_iBufferId);
		spu_array_get(m_arrayId, "1.buffer_id", &m_oBufferId);

		// texture
		{
			Attrs attrs = {
			        {"mag_filter", GL_LINEAR              },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			};
			u_albedo = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(
		        viewport(0), 180.0 * 0.25F /*, viewport(0).sx / viewport(0).sy*/, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		m_compShader.use();

		spu_array_draw(m_computeId, 0xffff, m_vertices.size(), 1, 1);

		m_drawShader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_program_compute");
}  // namespace
}  // namespace spu
