//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform int u_instance;                                                                \n"
    "uniform mat4 u_worldscreen[2];                                                          \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "in vec2 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen[u_instance] * vec4(a_position, 0.0, 1.0);               \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
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
	Mat4f u_worldscreen[2];
	Vec4f u_diffuse;
	int32_t u_instance;

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
			        {"u_worldscreen", &u_worldscreen[0]},
			        {"u_diffuse",     &u_diffuse       },
			        {"u_instance",    &u_instance      },
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
	}

	void render() override
	{
		{
			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
			{
				Mat4f rotation = Mat4f().rot("y", M_PI * 0.25);
				u_worldscreen[0] = getCamera().worldscreen() * rotation;
			}
			{
				Mat4f rotation = Mat4f().rot("y", M_PI * 0.50 + M_PI * 0.25);
				u_worldscreen[1] = getCamera().worldscreen() * rotation;
			}
		}

		u_diffuse = Vec4f(1.0, 0.5, 0.0, +1.0);

		for (auto i = 0; i < 2; ++i) {
			u_instance = i;
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_buffer_uniform");
}  // namespace
}  // namespace spu
