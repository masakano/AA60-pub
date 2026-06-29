//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 410 core                                                                      \n"
    "in vec2 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "out vec4 vertcolor;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "    float gl_ClipDistance[];                                                           \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(a_position, 0.0, 1.0);                                          \n"
    "    vertcolor = a_color;                                                               \n"
    "}                                                                                      \n"
};

const char *c_geom = {
    "#version 410 core                                                                      \n"
    "layout(triangles, invocations = 5) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "in gl_PerVertex                                                                        \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "    float gl_ClipDistance[];                                                           \n"
    "} gl_in[];                                                                             \n"
    "in vec4 colors[];                                                                      \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "    float gl_ClipDistance[];                                                           \n"
    "};                                                                                     \n"
    "out vec4 geomcolor;                                                                    \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        gl_Position = u_worldscreen *                                                   \n"
    "             (gl_in[i].gl_Position +                                                   \n"
    "              vec4(vec2(0.0), - 0.5 + 0.25 * float(gl_InvocationID), 0.0));            \n"
    "        geomcolor = (vec4(gl_InvocationID + 1) / 6.0 + colors[i]) / 2.0;               \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 410 core                                                                      \n"
    "in vec4 geomcolor;                                                                     \n"
    "layout(location = 0, index = 0) out vec4 final_color;                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    final_color = geomcolor;                                                           \n"
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
			const std::vector<v2fc4ub_t> c_vertices = {
			        {-1.0, -1.0, 255, 0,   0,   255},
			        {+1.0, -1.0, 255, 255, 0,   255},
			        {+1.0, +1.0, 0,   255, 0,   255},
			        {-1.0, +1.0, 0,   0,   255, 255}
                        };

			const std::vector<uint16_t> c_indices = {0, 1, 2, 2, 3, 0};

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
			        {"a.a_position", 2                },
			        {"format",       GL_UNSIGNED_BYTE },
			        {"normalize",    1                },
			        {"a.a_color",    4                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
			spu_array_send(m_arrayId, c_indices.data(), c_indices.size(), -1, 2);
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
		u_diffuse = Vec4f(1.0, 0.5, 0.0, +1.0);
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_410_primitive_instanced");
}  // namespace
}  // namespace spu
