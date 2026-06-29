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
    "uniform mat4 u_worldview;                                                               \n"
    "in vec4 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "out vec4 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_color = a_color;                                                                 \n"
    "    gl_Position = u_worldscreen * a_position;                                           \n"
    "    gl_PointSize = 512.0 / -(u_worldview * a_position).z;                               \n"
    "    gl_PointSize = clamp(gl_PointSize, 0, 128); /*debug*/                              \n"
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

	uint32_t m_arrayId;

	// set vertex via spu_array_map()

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_worldview",   &u_worldview  },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// arrayax
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

			data[0] = {-1.0, 0.0, 2.0, +1.0, 1.0, 0.0, 0.0, +1.0};
			data[1] = {-0.5, 0.0, 2.5, +1.0, 1.0, 0.5, 0.0, +1.0};
			data[2] = {+0.0, 0.0, 3.0, +1.0, 1.0, 1.0, 0.0, +1.0};
			data[3] = {+0.5, 0.0, 3.5, +1.0, 0.0, 1.0, 0.0, +1.0};
			data[4] = {+1.0, 0.0, 4.0, +1.0, 0.0, 0.0, 1.0, +1.0};

			spu_array_unmap(m_arrayId, 0);
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.point_sprite = true;  // no effect?
			renderstate.flags.program_point_size = true;
			renderstate.depth_func = GL_LESS;
			// renderstate.use();
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();
		u_worldview = getCamera().worldview();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_POINTS);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_primitive_point_clip");
}  // namespace
}  // namespace spu
