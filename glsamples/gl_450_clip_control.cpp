//
// App :
//
#include "base_app.h"
namespace spu {
// #include "gl_aux.h"
namespace {
/* clang-format off */
const char *c_vert = {
    "#version 430 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 430 core                                                                      \n"
    "uniform sampler2DArray u_diffuse;                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, vec3(f_texcoord.st, 0.0));                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	uint32_t m_arrayId;

	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// gl_aux_init();  // temporary
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
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs);
		}
		// texture
		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D_ARRAY    },
			        {"mag_filter", GL_LINEAR              },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			};
			u_diffuse = loadDDS("kueken7_rgba8_srgb.dds", attrs);
		}
	}

	void render() override
	{
		auto w = viewport(0).sx / 2;
		auto h = viewport(0).sy;
		auto viewport0 = Rectf(0, 0, w, h);
		auto viewport1 = Rectf(w, 0, w, h);
		getCamera().setViewscreen(viewport0, 180 * 0.25f, 0.1, 100.0);
		auto clip_control0 = Mat4f().scale({+1, +1, +1});  // LOWER_LEFT, ZERO_TO_ONE
		auto clip_control1 = Mat4f().scale({+1, -1, +1});  // UPPER_LEFT, ZERO_TO_ONE

		{
			u_worldscreen = clip_control0 * getCamera().worldscreen();
			m_shader.use();
			spu_frame_set(-1, "viewport0", viewport0);
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
		{
			u_worldscreen = clip_control1 * getCamera().worldscreen();
			m_shader.use();
			spu_frame_set(-1, "viewport0", viewport1);
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_450_clip_control");
}  // namespace
}  // namespace spu
