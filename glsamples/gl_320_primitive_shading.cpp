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
    "in vec4 a_color;                                                                       \n"
    "out vec4 g_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position,0.0,1.0);                             \n"
    "    g_color = a_color;                                                                 \n"
    "}                                                                                      \n"
};
const char *c_geom = {
    "#version 330                                                                           \n"
    "// If this is not declared, the compiler should generate an error                      \n"
    "#ifndef GEN_ERROR                                                                      \n"
    "layout(triangles) in;                                                                  \n"
    "#endif                                                                                 \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "uniform vec4 u_color[3];                                                               \n"
    "in vec4 g_color[];                                                                     \n"
    "out vec4 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for(int i = 0; i < gl_in.length(); ++i)                                            \n"
    "    {                                                                                  \n"
    "        gl_Position = gl_in[i].gl_Position;                                            \n"
    "        f_color = (g_color[i] + u_color[i]) * 0.5;                                     \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
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

const uint32_t c_color_count(3);
Vec4f const u_colors[c_color_count] = {
        {0.5, 0.5, 0.5, +1.0}, {0.7, 0.7, 0.7, +1.0}, {0.3, 0.3, 0.3, +1.0},
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Mat4f u_worldview;

	uint32_t m_arrayId;
	uint32_t m_queryId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// query
		{
			Attrs attrs = {
			        {"target", GL_PRIMITIVES_GENERATED},
			};
			m_queryId = spu_query_new(attrs);
		}
		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"geom", c_geom},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_color",       &u_colors[0]  },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<v2fc4ub_t> c_vertices = {
			        {-1.0, -1.0, 255, 0,   0,   255},
			        {+1.0, -1.0, 255, 255, 255, 255},
			        {+1.0, +1.0, 0,   255, 0,   255},
			        {-1.0, +1.0, 0,   0,   255, 255},
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
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		m_shader.use();

		spu_query_begin(m_queryId);
		spu_array_draw(m_arrayId, GL_TRIANGLES);

		uint64_t primitive_count = 0;
		spu_query_end(m_queryId, &primitive_count, true);  // this cause sync
		spu_printf(0, "primtive count: %d\n", primitive_count);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_primitive_shading");
}  // namespace
}  // namespace spu
