//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/torus.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "out vec2 f_texcoord;                                                                   \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_normal = mat3(u_nodeworld)*a_normal;                                          \n"
    "       f_light = u_light_pos - gl_Position.xyz;                                        \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_texture;                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = sqrt(length(f_light));                                                \n"
    "       float d = l > 0? dot(                                                           \n"
    "               f_normal,                                                               \n"
    "               normalize(f_light)                                                      \n"
    "       ) / l : 0.0;                                                                    \n"
    "       float i = 0.2 + 3.2*max(d, 0.0);                                                \n"
    "       final_color = texture(u_texture, f_texcoord)*i;                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;
	SpuTexture m_texture;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;
	uint32_t u_texture;

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			Attrs shader_attrs = {
			        {"frag", c_frag},
			        {"vert", c_vert},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen},
                                {"u_worldview", &u_worldview},
			        {"u_nodeworld", &u_nodeworld},
                                {"u_light_pos", &u_light_pos},
			        {"u_texture",   &u_texture  },
			};
			m_array.initShader(shader_attrs, unif_attrs);
			u_light_pos = {4, 4, -8};
		}

		{
			auto torus_shape = shapes::Torus(1.0, 0.5, 72, 48);
			m_array.initArray(torus_shape, {"position", "normal", "texcoord"});

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.cull_face = GL_BACK;
		}

		{
			auto s = size_t(256);
			std::vector<GLubyte> tex_data(s * s);

			for (auto v = 0u; v != s; ++v) {
				for (auto u = 0u; u != s; ++u) {
					tex_data[v * s + u] = rand() % 0x100;
				}
			}

			Attrs tex_attrs = {
			        {"target",     GL_TEXTURE_2D  },
			        {"iformat",    GL_R8          },
			        {"width",      s              },
			        {"height",     s              },

			        {"min_filter", GL_LINEAR      },
			        {"mag_filter", GL_LINEAR      },
			        {"wrap_s",     GL_REPEAT      },
			        {"wrap_t",     GL_REPEAT      },
			        {"swizzle_g",  GL_RED         },
			        {"swizzle_b",  GL_RED         },
			        {"data",       tex_data.data()},
			};
			m_texture.init(tex_attrs);
			u_texture = m_texture.id();
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 20);
		u_worldview = Mat4f::orbiting(ezero(), esec, 4.5, 0, 0, 0, 7.5, 0, 60, 20);
		u_nodeworld = math::unit().rot("x", -esec / 4 * math::two_pi());

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.fill = false;
		renderstate.cull_face = GL_FRONT;
		renderstate.use();

		m_array.draw(nullptr);  // no effect?

		renderstate.flags.fill = true;
		renderstate.cull_face = GL_BACK;
		renderstate.use();
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("016_noise_torus");
}  // namespace
}  // namespace spu::oglplus
