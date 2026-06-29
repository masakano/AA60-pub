//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert_feed = {
    "#version 420 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec4 a_position;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out block                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} Out;                                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * (a_position + vec4(vec3(float(gl_InstanceID) * 0.25 - 0.5), 0));\n"
    "    Out.color = vec4(clamp(vec2(a_position), 0.0, 1.0), 0.0, 1.0);                     \n"
    "}                                                                                      \n"
};
	
const char *c_geom_feed = {
    "#version 420 core                                                                      \n"
    "layout(triangles, invocations = 1) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 3) out;                                          \n"
    "in gl_PerVertex                                                                        \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "} gl_in[];                                                                             \n"
    "in block                                                                               \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} In[];                                                                                \n"
    "out block                                                                              \n"
    "{                                                                                      \n"
    "    layout(stream = 0) vec4 color;                                                     \n"
    "} Out;                                                                                 \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        Out.color = In[i].color;                                                       \n"
    "        gl_Position = gl_in[i].gl_Position;                                            \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};
	
const char *c_frag = {
    "#version 420 core                                                                      \n"
    "in block                                                                               \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} In;                                                                                  \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = In.color;                                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	uint32_t m_arrayId;
	Mat4f u_worldscreen;
	uint32_t u_albedo;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert_feed},
			        {"geom", c_geom_feed},
			        {"frag", c_frag     },
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
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
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES, 0, 0, 5);  // nprim = 5
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_transform_feedback_instanced");
}  // namespace
}  // namespace spu
