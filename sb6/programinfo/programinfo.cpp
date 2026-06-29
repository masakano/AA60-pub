//
// App :
//
#include "base_app.h"
namespace spu::programinfo {
/* clang-format off */
const char *vert = {
    "#version 420 core                                                                      \n"
    "                                                                                       \n"
    "in vec4 position;                                                                      \n"
    "                                                                                       \n"
    "uniform mat4 u_modelview;                                                              \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "                                                                                       \n"
    "out VS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec2 tc;                                                                           \n"
    "} vs_out;                                                                              \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_viewscreen * u_modelview * position;                               \n"
    "    vs_out.tc = position.xy;                                                           \n"
    "}                                                                                      \n"
    "                                                                                       \n"
};

const char *frag = {
    "#version 420 core                                                                      \n"
    "                                                                                       \n"
    "out vec4 color;                                                                        \n"
    "layout (location = 2) out ivec2 data;                                                  \n"
    "out float extra;                                                                       \n"
    "                                                                                       \n"
    "in BLOCK0                                                                              \n"
    "{                                                                                      \n"
    "    vec2 tc;                                                                           \n"
    "    vec4 color;                                                                        \n"
    "    flat int foo;                                                                      \n"
    "} fs_in0;                                                                              \n"
    "                                                                                       \n"
    "in BLOCK1                                                                              \n"
    "{                                                                                      \n"
    "    vec3 normal[4];                                                                    \n"
    "    flat ivec3 layers;                                                                 \n"
    "    double bar;                                                                        \n"
    "} fs_in1;                                                                              \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    float val = abs(fs_in0.tc.x + fs_in0.tc.y) * 20.0;                                 \n"
    "    color = vec4(fract(val) >= 0.5 ? 1.0 : 0.25) +                                     \n"
    "fs_in1.normal[3].xyzy;                                                                 \n"
    "    data = ivec2(1, 2);                                                                \n"
    "    extra = 9.0;                                                                       \n"
    "}                                                                                      \n"
    "                                                                                       \n"
};

/* clang-format on */

class App : public BaseApp {
public:
	SpuShader m_shader;
	Message::output_t m_prevOutput;

	App(const char *name) : BaseApp(name)
	{
		// dummy shader
		{
			Attrs shader_attrs = {
			        {"frag", frag},
                                {"vert", vert}
                        };
			loadShader(m_shader, shader_attrs, Attrs());
		}
		// redirect
		{
			m_prevOutput = g_message.output;
			g_message.output = output;
		}
	}
	static void output(int /*level*/, const char * /*time*/, const char * /*label*/, const char *msg)
	{
		spu_printf(0, msg);
	}
	~App() { g_message.output = m_prevOutput; }
	void render() override { m_shader.report("\n\nshader"); }
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("programinfo");
}  // namespace spu::programinfo
