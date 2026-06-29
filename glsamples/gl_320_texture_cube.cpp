//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                               \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "uniform vec3 u_camera;                                                                 \n"
    "const vec3 constView = vec3(0, 0,-1);                                                  \n"
    "const vec3 constNormal = vec3(0, 0, 1);                                                \n"
    "in vec2 a_position;                                                                    \n"
    "out vec3 f_reflect;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    mat3 u_worldview3x3 = mat3(u_worldview);                                             \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "    vec3 P = u_worldview3x3 * vec3(a_position, 0.0);                                    \n"
    "    vec3 N = u_worldview3x3 * constNormal;                                              \n"
    "    vec3 E = normalize(P - u_camera);                                                  \n"
    "    f_reflect = reflect(E, N);                                                         \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform samplerCube u_environment;                                                     \n"
    "in vec3 f_reflect;                                                                     \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_environment, f_reflect);                                         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Mat4f u_worldview;
	uint32_t u_environment;
	Vec3f u_camera;
	uint32_t m_arrayId;
	std::vector<Rectf> m_viewports;

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 1);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_worldview",   &u_worldview  },
			        {"u_environment", &u_environment},
			        {"u_camera",      &u_camera     },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<vec2sf_t> c_vertices = squareTriangles();

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
			        {"a.a_position", 2                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// texture
		{
			int32_t cube_target[] = {
			        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
			};

			float min_lod = -1000;
			float max_lod = +1000;
			float lod_bias = 0.0;
			Vec4f border = {0, 0, 0, 0};

			Attrs attrs = {
			        {"target",      GL_TEXTURE_CUBE_MAP},
			        {"iformat",     GL_RGBA8           },
			        {"width",       2                  },
			        {"height",      2                  },
			        {"cube_target", cube_target        },
			        {"min_filter",  GL_LINEAR          },
			        {"mag_filter",  GL_LINEAR          },
			        {"wrap_s",      GL_CLAMP_TO_EDGE   },
			        {"wrap_t",      GL_CLAMP_TO_EDGE   },
			        {"wrap_r",      GL_CLAMP_TO_EDGE   },
			        {"min_lod",     min_lod            },
			        {"max_lod",     max_lod            },
			        {"lod_bias",    lod_bias           },
			        {"border",      border             },
			};
			u_environment = spu_texture_new(attrs);
		}

		// texels
		{
			const std::vector<Vec4f> c_pixs = {
			        {1, 0, 0, 1},
                                {1, 0, 0, 1},
                                {1, 0, 0, 1},
                                {1, 0, 0, 1},
                                {0, 1, 0, 1},
			        {0, 1, 0, 1},
                                {0, 1, 0, 1},
                                {0, 1, 0, 1},
                                {0, 0, 1, 1},
                                {0, 0, 1, 1},
			        {0, 0, 1, 1},
                                {0, 0, 1, 1},
                                {0, 1, 1, 1},
                                {0, 1, 1, 1},
                                {0, 1, 1, 1},
			        {0, 1, 1, 1},
                                {1, 0, 1, 1},
                                {1, 0, 1, 1},
                                {1, 0, 1, 1},
                                {1, 0, 1, 1},
			        {1, 1, 0, 1},
                                {1, 1, 0, 1},
                                {1, 1, 0, 1},
                                {1, 1, 0, 1},
			};
			spu_texture_send(u_environment, c_pixs.data(), GL_RGBA32F);
		}
	}

	void render() override
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.cube_map_seamless = false /*1*/;
		renderstate.use();

		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();
		u_worldview = getCamera().worldview();

		// u_camera = Vec3f(0.0, 0.0, -getCameraDistance());
		u_camera = getCamera().position();
		m_shader.use();

		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_texture_cube");
}  // namespace
}  // namespace spu
