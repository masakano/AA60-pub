//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "#define COUNT 2                                                                        \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "struct my_vertex                                                                       \n"
    "{                                                                                      \n"
    "    vec2 positions[2];                                                                 \n"
    "};                                                                                     \n"
    "struct Vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "in vec2 a_position0;                                                                \n"
    "in vec2 a_position1;                                                                \n"
    "in vec4 a_color;                                                                       \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "    float gl_ClipDistance[];                                                           \n"
    "};                                                                                     \n"
    "/*layout(location = 0)*/ out Vertex st_Out;                                                \n"
    "out block                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "    float lumimance[COUNT];                                                            \n"
    "} bl_Out;                                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4((a_position0 + a_position1) * 0.5, 0.0, 1.0);\n"
    "    st_Out.color = a_color * 0.75;                                                     \n"
    "    bl_Out.color = a_color * 0.25;                                                     \n"
    "    for(int i = 0; i < COUNT; ++i)                                                     \n"
    "        bl_Out.lumimance[i] = 1.0 / float(COUNT);                                      \n"
    "}                                                                                      \n"
};

const char *c_cont = {
    "#version 420 core                                                                      \n"
    "layout(vertices = 4) out;                                                              \n"
    "struct Vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "in gl_PerVertex                                                                        \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "    float gl_ClipDistance[];                                                           \n"
    "} gl_in[];                                                                             \n"
    "in Vertex st_In[];                                                                     \n"
    "#define COUNT 2                                                                        \n"
    "in block                                                                               \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "    float lumimance[COUNT];                                                            \n"
    "} bl_In[];                                                                             \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "    float gl_ClipDistance[];                                                           \n"
    "} gl_out[];                                                                            \n"
    "/*layout(location = 0)*/ out Vertex st_Out[];                                              \n"
    "out block                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} bl_Out[];                                                                            \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    float luminance = 0.0;                                                             \n"
    "    for(int i = 0; i < COUNT; ++i)                                                     \n"
    "        luminance += bl_In[gl_InvocationID].lumimance[i];                              \n"
    "    gl_TessLevelInner[0] = 16.0;                                                       \n"
    "    gl_TessLevelInner[1] = 16.0;                                                       \n"
    "    gl_TessLevelOuter[0] = 8.0;                                                        \n"
    "    gl_TessLevelOuter[1] = 8.0;                                                        \n"
    "    gl_TessLevelOuter[2] = 8.0;                                                        \n"
    "    gl_TessLevelOuter[3] = 8.0;                                                        \n"
    "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;          \n"
    "    st_Out[gl_InvocationID].color = st_In[gl_InvocationID].color;                      \n"
    "    bl_Out[gl_InvocationID].color = bl_In[gl_InvocationID].color;                      \n"
    "    bl_Out[gl_InvocationID].color = bl_In[gl_InvocationID].color * luminance;          \n"
    "}                                                                                      \n"
};
const char *c_eval = {
    "#version 420 core                                                                      \n"
    "layout(quads, equal_spacing, ccw) in;                                                  \n"
    "struct Vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "in gl_PerVertex                                                                        \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "    float gl_ClipDistance[];                                                           \n"
    "} gl_in[];                                                                             \n"
    "in Vertex st_In[];                                                                     \n"
    "in block                                                                               \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} bl_In[];                                                                             \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "    float gl_ClipDistance[];                                                           \n"
    "};                                                                                     \n"
    "/*layout(location = 0)*/ out Vertex st_Out;                                                \n"
    "out block                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} bl_Out;                                                                              \n"
    "vec4 interpolate(in vec4 v0, in vec4 v1, in vec4 v2, in vec4 v3)                       \n"
    "{                                                                                      \n"
    "    vec4 a = mix(v0, v1, gl_TessCoord.x);                                              \n"
    "    vec4 b = mix(v3, v2, gl_TessCoord.x);                                              \n"
    "    return mix(a, b, gl_TessCoord.y);                                                  \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = interpolate(                                                         \n"
    "                gl_in[0].gl_Position,                                                  \n"
    "                gl_in[1].gl_Position,                                                  \n"
    "                gl_in[2].gl_Position,                                                  \n"
    "                gl_in[3].gl_Position);                                                 \n"
    "    st_Out.color = interpolate(                                                        \n"
    "                st_In[0].color,                                                        \n"
    "                st_In[1].color,                                                        \n"
    "                st_In[2].color,                                                        \n"
    "                st_In[3].color);                                                       \n"
    "    bl_Out.color = interpolate(                                                        \n"
    "                bl_In[0].color,                                                        \n"
    "                bl_In[1].color,                                                        \n"
    "                bl_In[2].color,                                                        \n"
    "                bl_In[3].color);                                                       \n"
    "}                                                                                      \n"
};
const char *c_geom = {
    "#version 420 core                                                                      \n"
    "layout(triangles, invocations = 1) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "in gl_PerVertex                                                                        \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "    float gl_ClipDistance[];                                                           \n"
    "} gl_in[];                                                                             \n"
    "struct Vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "in Vertex st_In[];                                                                     \n"
    "in block                                                                               \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} bl_In[];                                                                             \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "    float gl_ClipDistance[];                                                           \n"
    "};                                                                                     \n"
    "/*layout(location = 0)*/ out Vertex st_Out;                                                \n"
    "out vec4 colorGNI;                                                                     \n"
    "out block                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} bl_Out;                                                                              \n"
    "out block2                                                                             \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} bl_Pou;                                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        gl_Position = gl_in[i].gl_Position;                                            \n"
    "        colorGNI = st_In[i].color;                                                     \n"
    "        st_Out.color = st_In[i].color;                                                 \n"
    "        bl_Out.color = bl_In[i].color;                                                 \n"
    "        bl_Pou.color = st_In[i].color + bl_In[i].color;                                \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "struct Vertex                                                                          \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "};                                                                                     \n"
    "in Vertex st_In;                                                                       \n"
    "in block                                                                               \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} bl_In;                                                                               \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = st_In.color + bl_In.color;                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_arrayId;
	SpuShader m_shader;
	Mat4f u_worldscreen;

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
			struct Vertex {
				float m_x0, m_y0, m_x1, m_y1, m_r, m_g, m_b, m_a;
			};

			const std::vector<Vertex> c_vertices = {
			        {-1.0, -1.0, -1.0, -1.0, 1.0, 0.0, 0.0, 1.0},
			        {+1.0, -1.0, +1.0, -1.0, 1.0, 1.0, 0.0, 1.0},
			        {+1.0, +1.0, +1.0, +1.0, 0.0, 1.0, 0.0, 1.0},
			        {-1.0, +1.0, -1.0, +1.0, 0.0, 0.0, 1.0, 1.0}
                        };

			Attrs attrs = {
			        {"shader_id",     m_shader.id()    },
                                {"a.a_position0", 2                },
			        {"a.a_position1", 2                },
                                {"a.a_color",     4                },
			        {"data",          c_vertices.data()},
                                {"nelem",         c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
			spu_array_set(m_arrayId, "patch_vertices", c_vertices.size());
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.fill = false;
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

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_interface_matching");
}  // namespace
}  // namespace spu
