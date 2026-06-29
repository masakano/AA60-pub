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
    "in vec4 a_position;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * a_position;                                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform vec4 u_diffuse;                                                                \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = u_diffuse;                                                                 \n"
    "    //color = vec4(1.0);                                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Vec4f u_diffuse;
	uint32_t m_queryId;
	uint32_t m_arrayId;
	ConditionalDraw m_cdraw;

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
			auto vertices = squareTriangles();

			Attrs attrs = {
			        {"shader_id",    m_shader.id()  },
			        {"a.a_position", 2              },
			        {"data",         vertices.data()},
			        {"nelem",        vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// query
		{
			m_queryId = spu_query_new((Attrs){
			        {"target", GL_ANY_SAMPLES_PASSED_CONSERVATIVE}
                        });
			m_cdraw.init(m_arrayId, m_queryId);
		}
	}

	void render() override
	{
		// change mode
		static auto query_mode = 1;
		spu_printf(0, "hit \"l\" to toggle query (%d)\n", query_mode);
		if (getGesture()->pressed('l')) {
			query_mode = (query_mode + 1) & 0x1;
		}

		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		Mat4f model0 = Mat4f().trans({0.0, 0.0, 8.0});
		Mat4f model1 = Mat4f();

		Mat4f u_worldscreen0 = getCamera().worldscreen() * model0;  // out of the scope
		Mat4f u_worldscreen1 = getCamera().worldscreen() * model1;  // in the scope

		auto u_diffuse0 = Vec4f(0.5);
		Vec4f u_diffuse1 = c_orange;
		Vec4f u_diffuse2 = c_sky;

		// first draw
		{
			spu_frame_set(-1, "bgcolor", Vec4f(0.0, 0.0, 0.0, 1.0).f);

			spu_frame_clear(-1);

			if (query_mode == 0) {
				u_worldscreen = u_worldscreen0;
			}
			else {
				u_worldscreen = u_worldscreen1;
			}
			u_diffuse = u_diffuse0;
			m_shader.use();

			m_cdraw.probe();
		}

		// 2nd draw
		{
			u_worldscreen = u_worldscreen1;
			u_diffuse = u_diffuse1;
			m_shader.use();

			m_cdraw.begin(GL_QUERY_WAIT);

			spu_frame_set(-1, "bgcolor", Vec4f(0.25).f);
			spu_frame_clear(-1);

			spu_array_draw(m_arrayId, GL_TRIANGLES);
			m_cdraw.end();
		}

		// 3rd draw
		{
			u_worldscreen = u_worldscreen1;
			u_diffuse = u_diffuse2;
			m_shader.use();

			// Clear color buffer with black
			m_cdraw.begin(GL_QUERY_WAIT_INVERTED);

			spu_frame_set(-1, "bgcolor", ezero<Vec4f>().f);
			spu_frame_clear(-1);

			spu_array_draw(m_arrayId, GL_TRIANGLES);
			m_cdraw.end();
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_450_query_conditional");
}  // namespace
}  // namespace spu
