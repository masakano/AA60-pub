//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

const uint32_t c_vertex_count = 6;

const std::vector<vec2sf_t> c_vertices_f32 = {
        {-1.0, -1.0},
        {+1.0, -1.0},
        {+1.0, +1.0},
        {+1.0, +1.0},
        {-1.0, +1.0},
        {-1.0, -1.0}
};

const std::vector<vec2sb_t> c_vertices_i8 = {
        {-1, -1},
        {+1, -1},
        {+1, +1},
        {+1, +1},
        {-1, +1},
        {-1, -1}
};

const std::vector<uint32_t> c_vertices_rg_b10_a2 = {
        packSnorm3x10_1x2(Vec4f(-1.0, -1.0, 0.0, +1.0)), packSnorm3x10_1x2(Vec4f(+1.0, -1.0, 0.0, +1.0)),
        packSnorm3x10_1x2(Vec4f(+1.0, +1.0, 0.0, +1.0)), packSnorm3x10_1x2(Vec4f(+1.0, +1.0, 0.0, +1.0)),
        packSnorm3x10_1x2(Vec4f(-1.0, +1.0, 0.0, +1.0)), packSnorm3x10_1x2(Vec4f(-1.0, -1.0, 0.0, +1.0)),
};

const std::vector<vec2i_t> c_vertices_i32 = {
        {-1, -1},
        {+1, -1},
        {+1, +1},
        {+1, +1},
        {-1, +1},
        {-1, -1}
};

/* clang-format off */
const char *c_vert = {
    "#version 330 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * a_position;                                           \n"
    "}                                                                                      \n"
};
	
const char *c_frag = {
    "#version 330 core                                                                      \n"
    "uniform vec4 u_diffuse;                                                                \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = u_diffuse;                                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Vec4f u_diffuse;

	std::vector<Rectf> m_viewports;

	uint32_t m_arrayIds[4];

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 2);
		// program
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
			const void *data[] = {
			        c_vertices_f32.data(),
			        c_vertices_i8.data(),
			        c_vertices_i32.data(),
			        c_vertices_rg_b10_a2.data(),
			};

			const uint32_t format[] = {
			        GL_FLOAT,
			        GL_BYTE,
			        GL_INT,
			        GL_INT_2_10_10_10_REV,
			};

			const uint32_t normalize[] = {
			        0,
			        0,
			        0,
			        1,
			};

			const uint32_t width[] = {
			        2,
			        2,
			        2,
			        4,
			};

			for (auto i = 0; i < 4; i++) {
				Attrs attrs = {
				        {"shader_id",    m_shader.id() },
				        {"format",       format[i]     },
				        {"normalize",    normalize[i]  },
				        {"a.a_position", width[i]      },
				        {"data",         data[i]       },
				        {"nelem",        c_vertex_count},
				};
				m_arrayIds[i] = spu_array_new(attrs);
			}
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();
		u_diffuse = Vec4f(1.0, 0.5, 0.0, +1.0);
		m_shader.use();

		for (auto i = 0; i < 4; i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);
			spu_array_draw(m_arrayIds[i], GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_buffer_type");
}  // namespace
}  // namespace spu
