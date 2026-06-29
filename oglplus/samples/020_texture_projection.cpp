//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>
#include <shapes/cube.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform mat4 u_worldtexc;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "out vec4 f_texcoord;                                                                   \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_normal = (                                                                    \n"
    "               u_nodeworld *                                                           \n"
    "               vec4(-a_normal, 0.0)                                                    \n"
    "       ).xyz;                                                                          \n"
    "       f_light = (                                                                     \n"
    "               vec4(u_light_pos, 0.0) - u_nodeworld * a_position                       \n"
    "       ).xyz;                                                                          \n"
    "       f_texcoord =                                                                    \n"
    "               u_worldtexc * u_nodeworld * a_position;                                 \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen * u_worldview * u_nodeworld * a_position;                   \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_texture;                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "in vec4 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = length(f_light);                                                      \n"
    "       float d = l != 0.0 ? dot(                                                       \n"
    "               f_normal,                                                               \n"
    "               normalize(f_light)                                                      \n"
    "       ) / l : 0.0;                                                                    \n"
    "       float i = 0.1 + 4.2*max(d, 0.0);                                                \n"
    "       vec2 coord = f_texcoord.st/f_texcoord.q;                                        \n"
    "       vec4 t  = texture(u_texture, coord*0.5 + 0.5);                                  \n"
    "       final_color = vec4(t.rgb*i*sqrt(1.0-t.a), 1.0);                                 \n"
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
	Mat4f u_worldtexc;
	Vec3f u_light_pos;
	uint32_t u_texture;

	App(const char *name) : SpuPage(name, true, {0.1, 0.1, 0.1, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};
		Attrs unif_attrs = {
		        {"u_viewsceen", &u_viewsceen},
                        {"u_worldview", &u_worldview},
		        {"u_nodeworld", &u_nodeworld},
                        {"u_worldtexc", &u_worldtexc},
		        {"u_light_pos", &u_light_pos},
                        {"u_texture",   &u_texture  },
		};
		m_array.initShader(shader_attrs, unif_attrs);

		auto eye = Vec3f(0, 1, 2);
		auto dir = Vec3f(-eye);
		auto up = ey();
		u_worldview = math::worldview(eye, dir, up);

		m_array.initArray(shapes::Cube(), {"position", "normal"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_FRONT;

		images::Image image = images::LoadTexture("flower_glass");

		Vec4f border_color = {1, 1, 1, 0};
		Attrs tex_attrs = {
		        {"target",     GL_TEXTURE_2D          },
		        {"iformat",    GL_RGBA8               },
		        {"width",      image.width()          },
		        {"height",     image.height()         },
		        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
		        {"mag_filter", GL_LINEAR              },
		        {"wrap_s",     GL_CLAMP_TO_BORDER     },
		        {"wrap_t",     GL_CLAMP_TO_BORDER     },
		        {"border",     border_color           },
		        {"data",       image.data()           },
		};
		m_texture.init(tex_attrs);
		u_texture = m_texture.id();
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto texc_viewport = Rectf(0, 0, 1, 1);
		u_viewsceen = math::perspective(viewport(0), 68, 1, 20);
		u_light_pos = Vec3f(-1.0, 2.0, 2.0) * (1.0 - sin(esec / 5 * math::two_pi()) * 0.4);
		u_worldtexc = math::perspective(texc_viewport, 15.0, 1.0, 20.0)
		            * math::lookat(u_light_pos, ezero(), ey());
		u_nodeworld = math::unit().rot("y", -esec / 10 * math::two_pi());
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("020_texture_projection");
}  // namespace
}  // namespace spu::oglplus
