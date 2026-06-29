//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/sphere.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "       gl_Position = u_viewsceen * u_worldview * u_nodeworld * a_position;             \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float i = int((                                                                 \n"
    "               f_texcoord.x+                                                           \n"
    "               f_texcoord.y                                                            \n"
    "       )*16) % 2;                                                                      \n"
    "       final_color = mix(                                                              \n"
    "               vec4(0.9, 0.9, 0.9, 1.0),                                               \n"
    "               vec4(1.0, 0.3, 0.4, 1.0),                                               \n"
    "               i                                                                       \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

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
		};
		m_array.initShader(shader_attrs, unif_attrs);

		shapes::Sphere make_sphere;
		m_array.initArray(make_sphere, {"position", "texcoord"});
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto eye = Vec3f(2.5, 3.5, 2.5);
		auto dir = Vec3f(-eye);
		auto up = ey();

		u_viewsceen = math::perspective(viewport(0), 60, 1, 20);
		u_worldview = math::worldview(eye, dir, up);
		u_nodeworld = math::unit()
		                      .rot("y", -esec / 2 * math::two_pi())
		                      .trans({0.0f, sqrtf(1.0f + sinf(esec / 2.0f * math::two_pi())), 0.0f});

		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("013_spiral_sphere");
}  // namespace
}  // namespace spu::oglplus
