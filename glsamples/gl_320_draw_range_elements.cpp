//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position,0.0,1.0);                             \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0, 0.5, 0.0, 1.0);                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	std::vector<uint16_t> m_indices = {
	        0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4,
	};

	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// shader
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
			const std::vector<vec2sf_t> c_vertices = {
			        {-0.4, -0.6},
                                {+0.5, -0.4},
                                {+0.6, +0.4},
                                {-0.5, 0.5 },
			        {-0.5, -0.5},
                                {+0.5, -0.5},
                                {+0.5, +0.5},
                                {-0.5, 0.5 },
			};

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
			        {"a.a_position", 2                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
			spu_array_send(m_arrayId, m_indices.data(), m_indices.size(), -1, 2);
		}

		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			// renderstate.use();
		}
	}

	void render() override
	{
		std::vector<Rectf> m_viewports = makeViewports(3, 1);

		spu_frame_set(-1, "viewport0", m_viewports[0]);

		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();
		m_shader.use();

		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, m_indices.size() / 2);

		spu_frame_set(-1, "viewport0", m_viewports[1]);

		spu_array_draw(m_arrayId, GL_TRIANGLES, m_indices.size() / 2, m_indices.size() / 2);

		spu_frame_set(-1, "viewport0", m_viewports[2]);
		spu_array_set(m_arrayId, "base_vertex", 4);

		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, m_indices.size() / 2);
		spu_array_set(m_arrayId, "base_vertex", 0);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_draw_range_elements");
}  // namespace
}  // namespace spu
