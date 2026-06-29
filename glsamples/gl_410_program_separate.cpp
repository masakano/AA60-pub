//
// App :
//
#if 0  // deprecated
//
// 
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 410 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 vertcolor;                                                                    \n"
    "out vec2 verttexcoord;                                                                 \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "    verttexcoord = a_texcoord;                                                         \n"
    "    vertcolor = vec3(1.0, 0.9, 0.8);                                                   \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 410 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec3 vertcolor;                                                                     \n"
    "in vec2 verttexcoord;                                                                  \n"
    "layout(location = 0, index = 0) out vec4 final_color;                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    final_color = texture(u_diffuse, verttexcoord) * vec4(vertcolor, 1.0);             \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;      // unified
	SpuShader m_shader;  // separated
	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program unified
		{
			Attrs shader_attrs = {
			        {"vert",         c_vert       },
                                {"frag",         c_frag       },
			};
			Attrs unif_attrs = {
                                {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// program separated
		{
			Attrs shader_attrs = {
			        {"vert",         c_vert       },
                                {"frag",         c_frag       },
			};
			Attrs unif_attrs = {
                                {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// array
		{
			Attrs attrs = {
				{"shader_id",    m_shader.id()},
				{"a.a_position", 2                },
				{"a.a_texcoord", 2                },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs, Vec2f(1.0, 0.5));
		}
		
		// texture
		{
			u_diffuse = loadDDS("kueken7_rgba_dxt5_unorm.dds");
		}
	}

	void render() override
	{

		std::vector<Rectf> m_viewports = makeViewports(2, 1);

		// Render with the separate programs
		{
			spu_frame_set(-1, {"viewport0", m_viewports[0]});

			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
			u_worldscreen = getCamera().worldscreen();

			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}

		// Render with the unified programs
		{
			spu_frame_set(-1, {"viewport0", m_viewports[1]});
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_410_program_separate");
}  // namespace
}  // namespace spu
#endif
