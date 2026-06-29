//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform UB_TRANSFORM                                                                   \n"
    "{                                                                                      \n"
    "    mat4 worldscreen;                                                                   \n"
    "    mat4 worldview;                                                                     \n"
    "    vec3 camera;                                                                       \n"
    "} ub_transform;                                                                        \n"
    "const vec3 constView = vec3(0, 0,-1);                                                  \n"
    "const vec3 constNormal = vec3(0, 0, 1);                                                \n"
    "in vec2 a_position;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec3 f_reflect;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    mat3 worldview3x3 = mat3(ub_transform.worldview);                                    \n"
    "    gl_Position = ub_transform.worldscreen * vec4(a_position, 0.0, 1.0);                \n"
    "    vec3 P = worldview3x3 * vec3(a_position, 0.0);                                      \n"
    "    vec3 N = worldview3x3 * constNormal;                                                \n"
    "    vec3 E = normalize(P - ub_transform.camera);                                       \n"
    "    f_reflect = reflect(E, N);                                                         \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform uint u_layer;                                                              \n"
    "uniform samplerCubeArray u_environment;                                                \n"
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
	uint32_t u_environment;
	uint32_t u_sampler;
	uint32_t u_layer = 0;

	struct {
		Mat4f worldscreen;
		Mat4f worldview;
		Vec3f camera;
	} ub_transform;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_sky) {}

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
			const std::vector<vec2sf_t> c_vertices = {
			        {-4.0, -4.0},
                                {+4.0, -4.0},
                                {+4.0, +4.0},
			        {+4.0, +4.0},
                                {-4.0, +4.0},
                                {-4.0, -4.0}
                        };

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
			static const auto c_size = 512u;

			const std::vector<Vec4f> c_pixs0 = {

			        // layer #0
			        {0.8, 0.0, 0.0, 0.8},
			        {0.0, 0.8, 0.0, 0.8},
			        {0.8, 0.0, 0.5, 0.8},
			        {0.0, 0.8, 0.5, 0.8},
			        {0.8, 0.0, 0.5, 0.8},
			        {0.8, 0.8, 0.0, 0.8},

			        // layer #1
			        {0.4, 0.0, 0.0, 0.4},
			        {0.0, 0.4, 0.0, 0.4},
			        {0.4, 0.0, 0.4, 0.4},
			        {0.0, 0.4, 0.4, 0.4},
			        {0.4, 0.0, 0.4, 0.4},
			        {0.4, 0.4, 0.0, 0.4},

			        // layer 21
			        {0.2, 0.0, 0.0, 0.2},
			        {0.0, 0.2, 0.0, 0.2},
			        {0.2, 0.0, 0.2, 0.2},
			        {0.0, 0.2, 0.2, 0.2},
			        {0.2, 0.0, 0.2, 0.2},
			        {0.2, 0.2, 0.0, 0.2},
			};

			const auto c_pixs1 = [&]() {
				auto pixs = c_pixs0;
				for (auto &pix: pixs) {
					pix *= 0.50;
				}
				return pixs;
			}();
			const auto c_pixs2 = [&]() {
				auto pixs = c_pixs0;
				for (auto &pix: pixs) {
					pix *= 0.25;
				}
				return pixs;
			}();

			Attrs attrs = {
			        {"target",      GL_TEXTURE_CUBE_MAP_ARRAY},
			        {"iformat",     GL_RGBA8                 },
			        {"width",       c_size                   },
			        {"height",      c_size                   },
			        {"depth",       18                       }, // 6faces x 3 layers
			        {"base_level",  0                        },
			        {"max_level",   2                        },
			        {"auto_mipmap", 0                        },
			};
			u_environment = spu_texture_new(attrs);

			for (auto i = 0; i < 18; i++) {
				uint32_t size4[4] = {c_size, c_size, 1, 1};

				{
					int32_t dst_loc[4] = {0, 0, i, 0};
					spu_texture_send(
					        u_environment, &c_pixs0[i], GL_RGBA32F, dst_loc, size4, true);
				}
				{
					int32_t dst_loc[4] = {0, 0, i, 1};
					spu_texture_send(
					        u_environment, &c_pixs1[i], GL_RGBA32F, dst_loc, size4, true);
				}
				{
					int32_t dst_loc[4] = {0, 0, i, 2};
					spu_texture_send(
					        u_environment, &c_pixs2[i], GL_RGBA32F, dst_loc, size4, true);
				}
			}
		}
		// sampler
		{
			Vec4f border = {0, 0, 0, 0};
			float min_lod = -1000;
			float max_lod = 1000;
			float lod_bias = 0;

			Attrs attrs = {
			        {"target",       GL_TEXTURE_SAMPLER       },
			        {"min_filter",   GL_NEAREST_MIPMAP_NEAREST},
			        {"mag_filter",   GL_NEAREST               },
			        {"wrap_s",       GL_CLAMP_TO_EDGE         },
			        {"wrap_t",       GL_CLAMP_TO_EDGE         },
			        {"wrap_r",       GL_CLAMP_TO_EDGE         },
			        {"border",       border                   },
			        {"min_lod",      min_lod                  },
			        {"max_lod",      max_lod                  },
			        {"lod_bias",     lod_bias                 },
			        {"compare_mode", GL_NONE                  },
			        {"compare_func", GL_LEQUAL                },
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

		u_layer = (getSeconds().count() / 60) % 3;

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_texture_cube");
}  // namespace
}  // namespace spu
