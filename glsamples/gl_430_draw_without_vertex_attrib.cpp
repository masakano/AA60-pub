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
    "struct vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec2 position;                                                                     \n"
    "    vec2 texcoord;                                                                     \n"
    "};                                                                                     \n"
    "layout(std430, binding = 0) buffer mesh                                                \n"
    "{                                                                                      \n"
    "    vertex vertex[];                                                                   \n"
    "} Mesh;                                                                                \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = Mesh.vertex[gl_VertexID].texcoord;                                    \n"
    "    gl_Position = u_worldscreen * vec4(Mesh.vertex[gl_VertexID].position, 0.0, 1.0);    \n"
    "}                                                                                      \n"
};
	
const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2DArray u_albedo;                                                       \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_albedo, vec3(f_texcoord.st, 0.0));                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_albedo;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_white) {}

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
			        {"u_worldscreen", &u_worldscreen}, // UBO
			        {"u_albedo",      &u_albedo     },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareQuads<v2fv2f_t>();
			const std::vector<uint16_t> c_indices = {0, 1, 2, 2, 3, 0};

			Attrs attrs0 = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};

			Attrs attrs1 = {
			        {"shader_id", m_shader.id()},
			        {"a.mesh",    0            },
			};

			m_arrayId = spu_array_new(attrs0);
			spu_array_aux(m_arrayId, attrs1, 1);

			spu_array_send(m_arrayId, c_indices.data(), c_indices.size(), -1, 2);
			spu_array_send(
			        m_arrayId, c_vertices.data(), c_vertices.size() * sizeof(c_vertices[0]), 1);
			spu_array_link(m_arrayId, m_arrayId, 0, 1);  // share vertex buffer
		}
		// texture
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_ARRAY    },
			        {"mag_filter",  GL_LINEAR              },
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"auto_mipmap", 0                      },
			};
			u_albedo = loadDDS("kueken7_rgba8_srgb.dds", attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen() * Mat4f();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_draw_without_vertex_attrib");
}  // namespace
}  // namespace spu
