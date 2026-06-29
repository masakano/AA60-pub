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
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Vec4f u_diffuse;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_black) {}

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
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			};
			m_arrayId = squareQuadsArray<vec2sf_t>(attrs);
		}
	}

	void render() override
	{
		auto &camera = getCamera();

		camera.setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = camera.worldscreen();
		u_diffuse = Vec4f(1.0, 0.5, 0.0, +1.0);
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};
// canvas::Manager::Creator<App>
// spu_sketch_creator("gl_320_buffer_uniform_shared");
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_buffer_uniform_shared");

}  // namespace
}  // namespace spu
