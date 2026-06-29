//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 400 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "in vec2 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "out vec4 tc_color;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                          \n"
    "    tc_color = a_color;                                                                \n"
    "}                                                                                      \n"
};

const char *c_tesc = {
    "#version 400 core                                                                      \n"
    "layout(vertices = 4) out;                                                              \n"
    "in vec4 tc_color[];                                                                    \n"
    "out vec4 te_color[];                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_TessLevelInner[0] = 16.0;                                                       \n"
    "    gl_TessLevelInner[1] = 16.0;                                                       \n"
    "    gl_TessLevelOuter[0] = 16.0;                                                       \n"
    "    gl_TessLevelOuter[1] = 16.0;                                                       \n"
    "    gl_TessLevelOuter[2] = 16.0;                                                       \n"
    "    gl_TessLevelOuter[3] = 16.0;                                                       \n"
    "    gl_out[gl_InvocationID].gl_Position =                                              \n"
    "                    gl_in[gl_InvocationID].gl_Position;                                \n"
    "    te_color[gl_InvocationID] = tc_color[gl_InvocationID];                             \n"
    "}                                                                                      \n"
};

const char *c_tese = {
    "#version 400 core                                                                      \n"
    "layout(quads, equal_spacing, ccw) in;                                                  \n"
    "in vec4 te_color[];                                                                    \n"
    "out vec4 g_color;                                                                      \n"
    "vec4 interpolate(in vec4 v0, in vec4 v1, in vec4 v2, in vec4 v3)                       \n"
    "{                                                                                      \n"
    "    vec4 a = mix(v0, v1, gl_TessCoord.x);                                              \n"
    "    vec4 b = mix(v3, v2, gl_TessCoord.x);                                              \n"
    "    return mix(a, b, gl_TessCoord.y);                                                  \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = interpolate(                                                         \n"
    "                    gl_in[0].gl_Position,                                              \n"
    "                    gl_in[1].gl_Position,                                              \n"
    "                    gl_in[2].gl_Position,                                              \n"
    "                    gl_in[3].gl_Position);                                             \n"
    "    g_color = interpolate(                                                             \n"
    "                    te_color[0],                                                       \n"
    "                    te_color[1],                                                       \n"
    "                    te_color[2],                                                       \n"
    "                    te_color[3]);                                                      \n"
    "}                                                                                      \n"
};
const char *c_geom = {
    "#version 400 core                                                                      \n"
    "layout(triangles, invocations = 1) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "in vec4 g_color[];                                                                     \n"
    "out vec4 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        gl_Position = gl_in[i].gl_Position;                                            \n"
    "        f_color = g_color[i];                                                          \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
    "in vec4 f_color;                                                                       \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = f_color;                                                                   \n"
    "}                                                                                      \n"
};

const char *c_vert_smooth = {
    "#version 400 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "in vec2 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "out vec4 g_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                          \n"
    "    g_color = a_color;                                                                 \n"
    "}                                                                                      \n"
};

const char *c_geom_smooth = {
    "#version 400 core                                                                      \n"
    "layout(triangles, invocations = 1) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "in vec4 g_color[];                                                                     \n"
    "layout(stream = 0) out vec4 f_color;                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        f_color = g_color[i];                                                          \n"
    "        gl_Position = gl_in[i].gl_Position;                                            \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *c_frag_smooth = {
    "#version 400 core                                                                      \n"
    "in vec4 f_color;                                                                       \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = f_color;                                                                   \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shaders[2];
	Mat4f u_worldscreen;
	std::vector<Rectf> m_viewports;

	uint32_t m_arrayIds[2];

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 1);
		// program #0
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
                                {"tesc", c_tesc},
                                {"tese", c_tese},
			        {"geom", c_geom},
                                {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			};
			loadShader(m_shaders[0], shader_attrs, unif_attrs);
		}

		// program #1
		{
			Attrs shader_attrs = {
			        {"vert", c_vert_smooth},
			        {"geom", c_geom_smooth},
			        {"frag", c_frag_smooth},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			};
			loadShader(m_shaders[1], shader_attrs, unif_attrs);
		}

		// array
		{
			const std::vector<v2fc4ub_t> c_vertices = {
			        {-1.0, -1.0, 255, 0,   0,   255},
			        {+1.0, -1.0, 255, 255, 255, 255},
			        {+1.0, +1.0, 0,   255, 0,   255},
			        {-1.0, +1.0, 0,   0,   255, 255}
                        };

			const std::vector<uint16_t> c_indices = {0, 1, 2, 2, 3, 0};

			for (auto i = 0; i < 2; i++) {
				Attrs attrs = {
				        {"shader_id",    m_shaders[i].id()},
				        {"a.a_position", 2                },
				        {"format",       GL_UNSIGNED_BYTE },
				        {"normalize",    1                },
				        {"a.a_color",    4                },
				};
				m_arrayIds[i] = spu_array_new(attrs);
				if (i == 0) {
					spu_array_send(m_arrayIds[i], c_vertices.data(), c_vertices.size(), 0);
				}
			}
			spu_array_link(m_arrayIds[1], m_arrayIds[0], 0, 0);
			spu_array_send(m_arrayIds[1], c_indices.data(), c_indices.size(), -1, 2);
			spu_array_set(m_arrayIds[0], "patch_vertices", c_vertices.size());
		}
	}

	void render() override
	{
		spu_frame_set(-1, "viewport0", m_viewports[0]);

		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shaders[0].use();
		spu_array_draw(m_arrayIds[0], GL_PATCHES);

		spu_frame_set(-1, "viewport0", m_viewports[1]);

		m_shaders[1].use();
		spu_array_draw(m_arrayIds[1], GL_TRIANGLES);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_primitive_smooth_shading");
}  // namespace
}  // namespace spu
