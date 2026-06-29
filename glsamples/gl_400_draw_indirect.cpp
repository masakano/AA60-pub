//
// DrawElementsIndirectCommand :
//
#include "base_app.h"
namespace spu {
namespace {

struct DrawElementsIndirectCommand {
	DrawElementsIndirectCommand()

	        = default;

	DrawElementsIndirectCommand(
	        uint32_t primitiveCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex,
	        uint32_t baseInstance)
	        : m_primitiveCount(primitiveCount), m_instanceCount(instanceCount), m_firstIndex(firstIndex),
	          m_baseVertex(baseVertex), m_baseInstance(baseInstance)
	{
	}

	uint32_t m_primitiveCount{0};
	uint32_t m_instanceCount{0};
	uint32_t m_firstIndex{0};
	int32_t m_baseVertex{0};
	uint32_t m_baseInstance{0};
};

/* clang-format off */
const char *c_vert = {
    "#version 400 core                                                                      \n"
    "#define ATTR_POSITION    0                                                             \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
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
			const std::vector<vec2sf_t> c_vertices = squareQuads();
			const std::vector<uint32_t> c_indices = {0, 1, 2, 0, 2, 3};

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
			        {"a.a_position", 2                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
			spu_array_send(m_arrayId, c_indices.data(), c_indices.size(), -1, 4);

			DrawElementsIndirectCommand command(c_indices.size(), 1, 0, 0, 0);
			spu_array_send(m_arrayId, &command, 1, -2);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();
		u_diffuse = Vec4f(1.0, 0.5, 0.0, +1.0);
		m_shader.use();

		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_draw_indirect");
}  // namespace
}  // namespace spu
