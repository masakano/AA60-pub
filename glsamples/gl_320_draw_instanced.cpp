//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen[2];                                                          \n"
    "in vec2 a_position;                                                                    \n"
    "flat out int f_instance;                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen[gl_InstanceID] * vec4(a_position, 0.0, 1.0);            \n"
    "    f_instance = gl_InstanceID;                                                        \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform vec4 u_diffuse[2];                                                             \n"
    "flat in int f_instance;                                                                \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = u_diffuse[f_instance];                                                     \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen[2];
	Vec4f u_diffuse[2];
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_white) {}

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
			        {"u_worldscreen", &u_worldscreen[0]},
			        {"u_diffuse",     &u_diffuse[0]    },
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

		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			// renderstate.use();
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		Mat4f model_a = Mat4f().trans({-1.1, 0.0, 0.0});
		Mat4f model_b = Mat4f().trans({+1.1, 0.0, 0.0});

		u_worldscreen[0] = getCamera().worldscreen() * model_a;
		u_worldscreen[1] = getCamera().worldscreen() * model_b;

		u_diffuse[0] = Vec4f(1.0, 0.5, 0.0, +1.0);
		u_diffuse[1] = Vec4f(0.0, 0.5, 1.0, +1.0);

		m_shader.use();

		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 2 /*5*/);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_draw_instanced");
}  // namespace
}  // namespace spu
