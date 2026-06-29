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
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position,0.0,1.0);                             \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord);                                            \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>();

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
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// texture
		{
			Attrs attrs = {
			        {"min_filter", GL_NEAREST_MIPMAP_NEAREST},
			        {"mag_filter", GL_NEAREST               },
			        {"wrap_s",     GL_CLAMP_TO_EDGE         },
			        {"wrap_t",     GL_CLAMP_TO_EDGE         },
			};
			u_diffuse = loadDDS("kueken7_bgra8_srgb.dds", attrs);
		}
	}

	void render() override
	{
		auto min_scissor = Vec3f(10000.0f);
		auto max_scissor = Vec3f(-10000.0f);

		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		auto screenviewport = Mat4f().scale({viewport(0).sx / 2.0f, viewport(0).sy / 2.0f, 1.0f})
		                              .trans({viewport(0).sx / 2.0f, viewport(0).sy / 2.0f, 0.0f});

		const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>();

		for (auto &vertice: c_vertices) {
			auto x = vertice.px;
			auto y = vertice.py;

			auto projected = (screenviewport * u_worldscreen).pers3(Vec3f(x, y, 0));

			min_scissor = min(min_scissor, Vec3f(projected));
			max_scissor = max(max_scissor, Vec3f(projected));
		}

		spu_frame_set(-1, "bgcolor0", c_orange);
		spu_frame_clear(-1);

		Rectf scissor = {
		        min_scissor.x,
		        min_scissor.y,
		        max_scissor.x - min_scissor.x,
		        max_scissor.y - min_scissor.y,
		};
		spu_frame_set(-1, "scissor0", scissor);
		spu_frame_set(-1, "bgcolor0", c_black);
		spu_frame_clear(-1);

		// Bind the program for use
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_test_scissor");
}  // namespace
}  // namespace spu
