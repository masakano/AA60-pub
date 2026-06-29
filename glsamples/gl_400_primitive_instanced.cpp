//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 400 core                                                                      \n"
    "in vec3 a_position;                                                                    \n"
    "out vec3 g_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(a_position, 1.0);                                               \n"
    "    g_color = vec3(1.0, 0.5, 0.0);                                                     \n"
    "}                                                                                      \n"
};

const char *c_geom = {
    "#version 400 core                                                                      \n"
    "layout(triangles, invocations = 6) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "in vec3 g_color[];                                                                     \n"
    "out vec3 f_color;                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        f_color = (vec3(gl_InvocationID + 1) / 6.0 +                                   \n"
    "              g_color[i]) / 2.0;                                                       \n"
    "        gl_Position = u_worldscreen * (gl_in[i].gl_Position +                           \n"
    "               vec4(vec2(0.0), - 0.3 + float(0.1) * float(gl_InvocationID), 0.0));     \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
    "uniform vec4 u_diffuse;                                                                \n"
    "in vec3 f_color;                                                                       \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(f_color, 1.0) * u_diffuse;                                            \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Vec4f u_diffuse;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"geom", c_geom},
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
			m_arrayId = squareQuadsArray(attrs);
		}
		// renderstate
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			// renderstate.use();
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

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_primitive_instanced");
}  // namespace
}  // namespace spu
