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
    "in vec2 a_position;                                                                    \n"
    "in vec4 a_color;                                                                       \n"
    "out vec4 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_color = a_color;                                                                 \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "    gl_PointSize = 256.0 / -(u_worldview * vec4(a_position, 0.0, 1.0)).z;               \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec4 f_color;                                                                       \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = f_color * texture(u_diffuse, gl_PointCoord);                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Mat4f u_worldview;
	uint32_t u_diffuse;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture
		{
			Attrs attrs = {
			        {"min_filter", GL_NEAREST},
			        {"mag_filter", GL_NEAREST},
			};
			u_diffuse = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}
		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_worldview",   &u_worldview  },
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<v2fc4f_t> c_vertices = {
			        {-1.0, -1.0, 1, 0, 0, 1},
			        {+1.0, -1.0, 1, 1, 0, 1},
			        {+1.0, +1.0, 0, 1, 0, 1},
			        {-1.0, +1.0, 0, 0, 1, 1},
			};

			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_color",    4            },
			};
			m_arrayId = spu_array_new(attrs);
			spu_array_send(m_arrayId, c_vertices.data(), c_vertices.size(), 0);
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.depth_test = false;
			renderstate.flags.program_point_size = true;
			renderstate.flags.point_sprite = true;
			renderstate.depth_func = GL_LESS;
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();
		u_worldview = getCamera().worldview();

		{
			Attrs attrs = {
			        {"viewport0", viewport(0)},
			        {"scissor0",  viewport(0)},
			        {"bgcolor0",  c_orange   },
			};
			spu_frame_set(-1, attrs);
			spu_frame_clear(-1);
		}

		{
			auto half_viewport = Rectf(
			        viewport(0).sx / 4, viewport(0).sy / 4, viewport(0).sx / 2, viewport(0).sy / 2);
			Attrs attrs = {
			        {"viewport0", half_viewport},
			        {"scissor0",  half_viewport},
			        {"bgcolor0",  c_white      }
                        };
			spu_frame_set(-1, attrs);
			spu_frame_clear(-1);
		}
		m_shader.use();
		spu_array_draw(m_arrayId, GL_POINTS);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_primitive_sprite");
}  // namespace
}  // namespace spu
