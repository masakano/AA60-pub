//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 440 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, float(gl_InstanceID) * 0.25 - 0.5, 1.0);\n"
    "}                                                                                      \n"
};
	
const char *c_frag = {
    "#version 440 core                                                                      \n"
    "layout(binding = 0) buffer a_atomic { uint ui; } b_atomic;                             \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    uint counter = atomicAdd(b_atomic.ui, 1);                                          \n"
    "    color = vec4(                                                                      \n"
    "	    float((counter >> 0) & 255) / 255.0,                                              \n"
    "	    float((counter >> 8) & 255) / 255.0,                                              \n"
    "	    float((counter >>16) & 255) / 5.0,                                                \n"
    "	    0.5);                                                                             \n"
    "}                                                                                      \n"
};

/* clang-format on */
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
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareQuads<v2fv2f_t>();
			const std::vector<uint16_t> c_indices = {0, 1, 2, 2, 3, 0};

			{
				Attrs attrs = {
				        {"shader_id",    m_shader.id()    },
                                        {"a.a_position", 2                },
				        {"a.a_texcoord", 2                },
                                        {"data",         c_vertices.data()},
				        {"nelem",        c_vertices.size()},
				};
				m_arrayId = spu_array_new(attrs);
			}
			{
				Attrs attrs = {
				        {"a.+0",  0}, // shader buffer
				        {"nelem", 4},
				};
				spu_array_aux(m_arrayId, attrs, 1);
			}

			spu_array_send(m_arrayId, c_indices.data(), c_indices.size(), -1, 2);
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.blend = true;
			renderstate.blend_eq = {GL_FUNC_ADD, GL_FUNC_ADD};
			renderstate.blend_func = {
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			};
			// renderstate.use();
		}
	}

	void render() override
	{
		// Setup blending

		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		auto *ptr = static_cast<uint32_t *>(
		        spu_array_map(m_arrayId, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, 1));
		spu_printf(0, "atomic counter = %d\n", *ptr);
		*ptr = 0;
		spu_array_unmap(m_arrayId, 1);

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 5);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_atomic_counter");
}  // namespace
}  // namespace spu
