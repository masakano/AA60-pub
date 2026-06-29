//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "out vec4 g_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    g_color = a_color;                                                                 \n"
    "    gl_Position = a_position;                                                          \n"
    "    gl_PointSize = 512 / -(u_worldview * a_position).z;                                \n"
    "}                                                                                      \n"
};

const char *c_geom = {
    "#version 330                                                                           \n"
    "layout(points) in;                                                                     \n"
    "#define RENDER_QUAD                                                                    \n"
    "#ifdef RENDER_QUAD                                                                     \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "#else                                                                                  \n"
    "layout(points, max_vertices = 4) out;                                                  \n"
    "#endif                                                                                 \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec3 u_camera_world;                                                           \n"
    "in gl_PerVertex                                                                        \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "} gl_in[];                                                                             \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "    float gl_PointSize;                                                                \n"
    "};                                                                                     \n"
    "in vec4 g_color[];                                                                     \n"
    "out vec4 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_color = g_color[0];                                                              \n"
    "#ifdef RENDER_QUAD                                                                     \n"
    "    vec3 center = gl_in[0].gl_Position.xyz;                                            \n"
    "    vec3 zAxis = vec3(u_worldview * vec4(normalize(center - u_camera_world), 1.0));  \n"
    "    vec3 yAxis = vec3(0.0, 1.0, 0.0);                                                  \n"
    "    vec3 xAxis = normalize(cross(zAxis, yAxis));                                       \n"
    "    yAxis = normalize(cross(xAxis, zAxis));                                            \n"
    "    vec3 x = xAxis * 0.5;                                                              \n"
    "    vec3 y = yAxis * 0.5;                                                              \n"
    "    gl_Position = u_worldscreen * vec4(center - x - y, 1.0);                           \n"
    "    EmitVertex();                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(center + x - y, 1.0);                           \n"
    "    EmitVertex();                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(center - x + y, 1.0);                           \n"
    "    EmitVertex();                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(center + x + y, 1.0);                           \n"
    "    EmitVertex();                                                                      \n"
    "#else                                                                                  \n"
    "    gl_Position = u_worldscreen * gl_in[0].gl_Position;                                \n"
    "    gl_PointSize = gl_in[0].gl_PointSize;                                              \n"
    "    EmitVertex();                                                                      \n"
    "#endif                                                                                 \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};
const char *c_frag = {
    "#version 330                                                                           \n"
    "in vec4 f_color;                                                                       \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = f_color;                                                                   \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Mat4f u_worldview;
	Vec3f u_camera_world;

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
			        {"u_worldscreen",  &u_worldscreen },
			        {"u_worldview",    &u_worldview   },
			        {"u_camera_world", &u_camera_world},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const auto c_vtx_count = 5;

			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 4            },
			        {"a.a_color",    4            },
			        {"nelem",        c_vtx_count  },
			};
			m_arrayId = spu_array_new(attrs);

			auto *data = static_cast<v4fc4f_t *>(spu_array_map(m_arrayId, GL_MAP_WRITE_BIT, 0));

			data[0] = {0.0, 0.0, -0.5, 1.0, 1.0, 0.0, 0.0, 1.0};
			data[1] = {0.2, 0.0, +0.5, 1.0, 1.0, 0.5, 0.0, 1.0};
			data[2] = {0.4, 0.0, +1.5, 1.0, 1.0, 1.0, 0.0, 1.0};
			data[3] = {0.6, 0.0, +2.5, 1.0, 0.0, 1.0, 0.0, 1.0};
			data[4] = {0.8, 0.0, +3.5, 1.0, 0.0, 0.0, 1.0, 1.0};

			spu_array_unmap(m_arrayId, 0);
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.program_point_size = true;
			renderstate.depth_func = GL_LESS;
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();
		u_worldview = getCamera().worldview();

		u_camera_world = -getCamera().position();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_POINTS);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_primitive_point_quad");
}  // namespace
}  // namespace spu
