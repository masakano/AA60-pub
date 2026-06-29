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
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * a_position;                                           \n"
    "}                                                                                      \n"
};
const char *c_frag = {
    "#version 330                                                                           \n"
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
	uint32_t m_queryId0;
	uint32_t m_queryId1;
	uint32_t m_arrayId;

	ConditionalDraw m_cdraw0;  // any sample passed
	ConditionalDraw m_cdraw1;  // samples

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
			m_queryId0 = spu_query_new((Attrs){
			        {"target", GL_ANY_SAMPLES_PASSED}
                        });
			m_queryId1 = spu_query_new((Attrs){
			        {"target", GL_SAMPLES_PASSED}
                        });
		}

		// cond draw
		m_cdraw0.init(m_arrayId, m_queryId0);
		m_cdraw1.init(m_arrayId, m_queryId1);
	}

	void render() override
	{
		// must not auto
		static auto query_mode = 3;
		auto is_nonblock = ((query_mode >> 2) & 0x1) != 0;

		spu_printf(
		        0, "hit \"l\" to toggle query (%d:%d:%d)\n", query_mode & 0x1, (query_mode >> 1) & 0x1,
		        (query_mode >> 2) & 0x1);

		if (getGesture()->pressed('l')) {
			query_mode = (query_mode + 1) & 0x7;
		}

		// Compute the u_worldscreen (Model View getCamera().viewscreen() matrix)
		// getCamera().setViewscreen(viewport(0), 45.0, 0.1, 100.0, 4.0 / 3.0);
		getCamera().setViewscreen(viewport(0), 45.0, 0.1, 100.0);
		auto worldview = getCamera().worldview();

		u_worldscreen = getCamera().worldscreen();

		// Clear color buffer with black

		// Bind program
		u_diffuse = Vec4f(0.0, 0.5, 1.0, +1.0);
		m_shader.use();

		// The first orange quad is not written in the framebuffer.
		{
			m_cdraw0.probe((query_mode & 0x1) != 0, is_nonblock);
			m_cdraw1.probe((query_mode & 0x2) != 0, is_nonblock);

			spu_printf(
			        0, "any_sample_passed = %d sample_passed = %d\n", m_cdraw0.get(),
			        m_cdraw1.get());
		}

		// 2nd draw
		{
			u_worldscreen = getCamera().viewscreen() * worldview * Mat4f().trans({1.5, 0.0, 0.0});
			u_diffuse = Vec4f(0.0, 0.5, 1.0, +1.0);
			m_shader.use();

			// draw only if query0 is passed
			m_cdraw0.begin();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
			m_cdraw0.end();
		}

		{
			u_worldscreen = getCamera().viewscreen() * worldview * Mat4f().trans({-1.5, 0.0, 0.0});
			u_diffuse = Vec4f(0.5, 0.0, 1.0, +1.0);
			m_shader.use();

			m_cdraw1.begin();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
			m_cdraw1.end();
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_query_conditional");
}  // namespace
}  // namespace spu
