//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 400 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "struct vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "in vec2 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "out vertex Vert;                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "    Vert.color = a_color;                                                              \n"
    "}                                                                                      \n"
};

const char *c_cont = {
    "#version 400 core                                                                      \n"
    "layout(vertices = 4) out;                                                              \n"
    "struct vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "in vertex Vert[];                                                                      \n"
    "out vertex Cont[];                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_TessLevelInner[0] = 16.0;                                                       \n"
    "    gl_TessLevelInner[1] = 16.0;                                                       \n"
    "    gl_TessLevelOuter[0] = 8.0;                                                        \n"
    "    gl_TessLevelOuter[1] = 8.0;                                                        \n"
    "    gl_TessLevelOuter[2] = 8.0;                                                        \n"
    "    gl_TessLevelOuter[3] = 8.0;                                                        \n"
    "    gl_out[gl_InvocationID].gl_Position =                                              \n"
    "                    gl_in[gl_InvocationID].gl_Position;                                \n"
    "    Cont[gl_InvocationID].color =                                                      \n"
    "                   Vert[gl_InvocationID].color;                                        \n"
    "}                                                                                      \n"
};

const char *c_eval = {
    "#version 400 core                                                                      \n"
    "layout(quads, equal_spacing, ccw) in;                                                  \n"
    "struct vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "in vertex Cont[];                                                                      \n"
    "out vertex Eval;                                                                       \n"
    "vec4 interpolate(in vec4 v0, in vec4 v1, in vec4 v2, in vec4 v3)                       \n"
    "{                                                                                      \n"
    "    vec4 a = mix(v0, v1, gl_TessCoord.x);                                              \n"
    "    vec4 b = mix(v3, v2, gl_TessCoord.x);                                              \n"
    "    return mix(a, b, gl_TessCoord.y);                                                  \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = interpolate(                                                         \n"
    "                      gl_in[0].gl_Position,                                            \n"
    "                      gl_in[1].gl_Position,                                            \n"
    "                      gl_in[2].gl_Position,                                            \n"
    "                      gl_in[3].gl_Position);                                           \n"
    "    Eval.color = interpolate(                                                          \n"
    "                      Cont[0].color,                                                   \n"
    "                      Cont[1].color,                                                   \n"
    "                      Cont[2].color,                                                   \n"
    "                      Cont[3].color);                                                  \n"
    "}                                                                                      \n"
};

const char *c_geom = {
    "#version 400 core                                                                      \n"
    "layout(triangles, invocations = 1) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "struct vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "in vertex Eval[];                                                                      \n"
    "out vertex Geom;                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        gl_Position = gl_in[i].gl_Position;                                            \n"
    "        Geom.color = Eval[i].color;                                                    \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
    "struct vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "in vertex Geom;                                                                        \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = Geom.color;                                                                \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
                                {"tesc", c_cont},
                                {"tese", c_eval},
			        {"geom", c_geom},
                                {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<v2fc4f_t> c_vertices = {
			        {-1.0, -1.0, 1.0, 0.0, 0.0, +1.0},
			        {+1.0, -1.0, 1.0, 1.0, 0.0, +1.0},
			        {+1.0, +1.0, 0.0, 1.0, 0.0, +1.0},
			        {-1.0, +1.0, 0.0, 0.0, 1.0, +1.0}
                        };

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_color",    4                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
			spu_array_set(m_arrayId, "patch_vertices", c_vertices.size());
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.fill = false;
			// renderstate.use();
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_PATCHES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_program_varying_structs");
}  // namespace
}  // namespace spu
