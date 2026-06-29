//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>
#include <images/filtered.hpp>
#include <shapes/cube.hpp>

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
    "in vec3 a_tangent;                                                                     \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_light;                                                                      \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out mat3 f_normal_matrix;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 frag_normal = mat3(u_nodeworld) * a_normal;                                \n"
    "       vec3 frag_tangent = mat3(u_nodeworld) * a_tangent;                              \n"
    "       f_normal_matrix[0] = frag_tangent;                                              \n"
    "       f_normal_matrix[1] = cross(frag_normal, frag_tangent);                          \n"
    "       f_normal_matrix[2] = frag_normal;                                               \n"

    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_light = u_light_pos - gl_Position.xyz;                                        \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_color_texture;                                                     \n"
    "uniform sampler2D u_normalmap;                                                         \n"
    "in mat3 f_normal_matrix;                                                               \n"
    "in vec3 f_light;                                                                       \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = dot(f_light, f_light);                                                \n"
    "       vec3 n = texture(u_normalmap, f_texcoord).xyz;                                  \n"
    "       vec3 final_normal = f_normal_matrix * n;                                        \n"
    "       float d = (l > 0.0) ? dot(                                                      \n"
    "               normalize(f_light),                                                     \n"
    "               final_normal                                                            \n"
    "       ) / l : 0.0;                                                                    \n"
    "       float i = 0.2 + 4.5*max(d, 0.0);                                                \n"
    "       vec4 t  = texture(u_color_texture, f_texcoord);                                 \n"
    "       final_color = vec4(t.rgb*i, 1.0);                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;
	SpuTexture m_colorTexture;
	SpuTexture m_normalTexture;

	uint32_t u_color_texture;
	uint32_t u_normalmap;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;

	App(const char *name) : SpuPage(name, true, {0.1, 0.1, 0.1, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			Attrs shader_attrs = {
			        {"frag", c_frag},
			        {"vert", c_vert},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen",     &u_viewsceen    },
                                {"u_worldview",     &u_worldview    },
			        {"u_nodeworld",     &u_nodeworld    },
                                {"u_light_pos",     &u_light_pos    },
			        {"u_color_texture", &u_color_texture},
                                {"u_normalmap",     &u_normalmap    },
			};
			m_array.initShader(shader_attrs, unif_attrs);
		}

		{
			m_array.initArray(shapes::Cube(), {"position", "normal", "tangent", "texcoord"});

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
		}

		{
			images::Image image = images::LoadTexture("wooden_crate");

			Attrs tex_attrs = {
			        {"target",     GL_TEXTURE_2D          },
			        {"iformat",    GL_RGBA8               },
			        {"width",      image.width()          },
			        {"height",     image.height()         },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_REPEAT              },
			        {"wrap_t",     GL_REPEAT              },
			        {"data",       image.data()           },
			};
			m_colorTexture.init(tex_attrs);
			u_color_texture = m_colorTexture.id();
		}
		{
			images::Image image = images::NormalMap(
			        images::LoadTexture("wooden_crate-hmap"), images::NormalMap::FromRed());

			Attrs tex_attrs = {
			        {"target",     GL_TEXTURE_2D          },
			        {"iformat",    GL_RGBA32F             },
			        {"width",      image.width()          },
			        {"height",     image.height()         },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_REPEAT              },
			        {"wrap_t",     GL_REPEAT              },
			        {"data",       image.data()           },
			};
			m_normalTexture.init(tex_attrs);
			u_normalmap = m_normalTexture.id();
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();

		u_viewsceen = math::perspective(viewport(0), 70, 1, 15);

		auto light_azimuth = esec * -0.5 * math::two_pi();
		u_light_pos = Vec3f(cos(light_azimuth), 1.0, sin(light_azimuth)) * 2.0;
		u_worldview = Mat4f::orbiting(ezero(), esec, 3.0, 0, 0, -45, 0, 0, 70, 30);
		u_nodeworld = math::unit().rot("y", -esec * 0.05 * math::two_pi());

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.cull_face = GL_BACK;
		renderstate.use();

		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("021_wooden_crate");
}  // namespace
}  // namespace spu::oglplus
