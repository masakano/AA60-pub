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
    "uniform mat4 u_viewsceen,                                                              \n"
    "u_worldview, u_nodeworld;                                                              \n"
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
    "       float l = length(f_light);                                                      \n"
    "       float d = l != 0.0 ? dot(f_normal, normalize(f_light))/l : 0.0;                 \n"
    "       float e = (d < 0? -0.7*d: d) * 3.0;                                             \n"
    "       float i = 0.1 + e;                                                              \n"
    "       vec4 t  = texture(u_texture, f_texcoord);                                       \n"
    "       final_color = vec4(t.rgb*i, t.a);                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;
	SpuTexture m_texture;

	uint32_t u_texture;
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
			        {"u_viewsceen", &u_viewsceen},
                                {"u_worldview", &u_worldview},
			        {"u_nodeworld", &u_nodeworld},
                                {"u_light_pos", &u_light_pos},
			        {"u_texture",   &u_texture  },
			};
			m_array.initShader(shader_attrs, unif_attrs);

			u_light_pos = {1.0, 2.0, 3.0};
		}

		{
			m_array.initArray(shapes::Cube(), {"position", "normal", "texcoord"});

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.flags.blend = true;

			renderstate.blend_func = {
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			        GL_SRC_ALPHA,
			        GL_ONE_MINUS_SRC_ALPHA,
			};
		}

		{
			images::Image image = images::LoadTexture("flower_glass");

			Attrs tex_attrs = {
			        {"target",     GL_TEXTURE_2D             },
			        {"iformat",    GL_RGBA8                  },
			        {"width",      image.width()             },
			        {"height",     image.height()            },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR   },
			        {"mag_filter", GL_LINEAR                 },
			        {"wrap_s",     GL_REPEAT                 },
			        {"wrap_t",     GL_REPEAT                 },
			        {"data",       image.data()},
			};
			m_texture.init(tex_attrs);
			u_texture = m_texture.id();
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 20);
		u_worldview = Mat4f::orbiting(ezero(), esec, 6, -3, 6, 0, 1.42, 0, 90, 30);
		u_nodeworld = math::unit().rot("x", -esec / 10 * math::two_pi());

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.cull_face = GL_FRONT;
		renderstate.use();
		m_array.draw(nullptr);

		renderstate.cull_face = GL_BACK;
		renderstate.use();
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("018_stained_glass_cube");
}  // namespace
}  // namespace spu::oglplus
