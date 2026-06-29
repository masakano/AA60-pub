//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 400 core                                                                      \n"
    "uniform UB_TRANSFORM                                                                   \n"
    "{                                                                                      \n"
    "    mat4 worldscreen;                                                                  \n"
    "    mat4 worldview;                                                                    \n"
    "    vec3 camera;                                                                       \n"
    "} ub_transform;                                                                        \n"
    "const vec3 constView = vec3(0, 0,-1);                                                  \n"
    "const vec3 constNormal = vec3(0, 0, 1);                                                \n"
    "in vec2 a_position;                                                                    \n"
    "out vec3 f_reflect;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    mat3 worldview3x3 = mat3(ub_transform.worldview);                                  \n"
    "    gl_Position = ub_transform.worldscreen * vec4(a_position, 0.0, 1.0);               \n"
    "    vec3 P = worldview3x3 * vec3(a_position, 0.0);                                     \n"
    "    vec3 N = worldview3x3 * constNormal;                                               \n"
    "    vec3 E = normalize(P - ub_transform.camera);                                       \n"
    "    f_reflect = reflect(E, N);                                                         \n"
    "}                                                                                      \n"
};
const char *c_frag = {
    "#version 400 core                                                                      \n"
    "uniform samplerCubeArray u_environment;                                                \n"
    "uniform int u_layer;                                                                   \n"
    "in vec3 f_reflect;                                                                     \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_environment, vec4(f_reflect, u_layer));                          \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;

	struct {
		Mat4f worldscreen;
		Mat4f worldview;
		Vec3f camera;
	} ub_transform;

	uint32_t m_arrayId;

	uint32_t u_environment;
	uint32_t u_sampler;
	uint32_t u_layer;

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
			        {"ub_transform",  &ub_transform },
			        {"u_environment", &u_environment},
			        {"u_environment", &u_sampler    },
			        {"u_layer",       &u_layer      },
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
			const auto c_size = 8;
			const auto c_layer_count = 12;  // 2 layer

			const std::vector<uint32_t> c_pixs = [=]() {
				std::vector<uint32_t> pixs(c_layer_count * c_size * c_size);
				for (auto i = 0; i < c_size * c_size; i++) {
					// cube #0
					pixs[0 * c_size * c_size + i] = 0xff0000ff;
					pixs[1 * c_size * c_size + i] = 0xff00ff00;
					pixs[2 * c_size * c_size + i] = 0xffff0000;
					pixs[3 * c_size * c_size + i] = 0xffffff00;
					pixs[4 * c_size * c_size + i] = 0xffff00ff;
					pixs[5 * c_size * c_size + i] = 0xff00ffff;

					// cube #1
					pixs[6 * c_size * c_size + i] = 0xff00007f;
					pixs[7 * c_size * c_size + i] = 0xff007f00;
					pixs[8 * c_size * c_size + i] = 0xff7f0000;
					pixs[9 * c_size * c_size + i] = 0xff7f7f00;
					pixs[10 * c_size * c_size + i] = 0xff7f007f;
					pixs[11 * c_size * c_size + i] = 0xff007f7f;
				}
				return pixs;
			}();
			/*
			const void *pixv[] = {
			        pix[0], 0,
			};
			*/
			Attrs attrs = {
			        {"target",      GL_TEXTURE_CUBE_MAP_ARRAY},
			        {"data",        c_pixs.data()            },
			        {"iformat",     GL_RGBA8                 },
			        {"width",       c_size                   },
			        {"height",      c_size                   },
			        {"depth",       c_layer_count            },
			        {"base_level",  0                        },
			        {"max_level",   0                        },
			        {"auto_mipmap", 0                        },
			};
			u_environment = spu_texture_new(attrs);
		}
		// sampler
		{
			float min_lod = -1000;
			float max_lod = 1000;
			float lod_bias = 0;
			float max_aniso = 16;
			Vec4f border = {0.5, 0.5, 0.5, 0};

			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER     },
			        {"wrap_s",       GL_CLAMP_TO_EDGE       },
			        {"wrap_t",       GL_CLAMP_TO_EDGE       },
			        {"wrap_r",       GL_CLAMP_TO_EDGE       },
			        {"min_filter",   GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",   GL_LINEAR              },
			        {"min_lod",      min_lod                },
			        {"max_lod",      max_lod                },
			        {"lod_bias",     lod_bias               },
			        {"compare_mode", GL_NONE                },
			        {"compare_func", GL_LEQUAL              },
			        {"border",       border                 },
			        {"max_aniso",    max_aniso              },
			};
			u_sampler = spu_texture_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);

		ub_transform.worldscreen = getCamera().worldscreen();
		ub_transform.worldview = getCamera().worldview();
		ub_transform.camera = getCamera().position();

		u_layer = (getSeconds().count() / 60) & 1;
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_texture_cube");
}  // namespace
}  // namespace spu
