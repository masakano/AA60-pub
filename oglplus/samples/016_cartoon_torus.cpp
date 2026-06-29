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
    "out vec3 f_normal;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_normal = mat3(u_nodeworld)*a_normal;                                          \n"
    "       gl_Position = u_viewsceen * u_worldview * u_nodeworld * a_position;             \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "out vec4 final_color;                                                                  \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float intensity = 2.0 * max(                                                    \n"
    "               dot(f_normal,  u_light_pos)/                                            \n"
    "               length(u_light_pos),                                                    \n"
    "               0.0                                                                     \n"
    "       );                                                                              \n"
    "       if (!gl_FrontFacing)                                                            \n"
    "               final_color = vec4(0.0, 0.0, 0.0, 1.0);                                 \n"
    "       else if (intensity > 0.9)                                                       \n"
    "               final_color = vec4(1.0, 0.9, 0.8, 1.0);                                 \n"
    "       else if (intensity > 0.1)                                                       \n"
    "               final_color = vec4(0.7, 0.6, 0.4, 1.0);                                 \n"
    "       else                                                                            \n"
    "               final_color = vec4(0.3, 0.2, 0.1, 1.0);                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		// shader
		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen", &u_viewsceen},
		        {"u_worldview", &u_worldview},
		        {"u_nodeworld", &u_nodeworld},
		        {"u_light_pos", &u_light_pos},
		};
		m_array.initShader(shader_attrs, unif_attrs);

		u_light_pos = {4, 4, -8};

		auto torus_shape = shapes::Torus(1.0, 0.5, 72, 48);
		m_array.initArray(torus_shape, {"position", "normal"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_BACK;
		renderstate.line_width = 4.0;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		// reshape
		u_viewsceen = math::perspective(viewport(0), 75, 1, 30);
		u_worldview = Mat4f::orbiting(ezero(), esec, 3.5, 0, 0, 0, 7.5, 0, 60, 20);
		u_nodeworld
		        = math::unit().rot("y", -esec / 4 * math::two_pi()).rot("x", -1.0 / 4 * math::two_pi());

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.fill = false;
		renderstate.cull_face = GL_FRONT;
		renderstate.use();
		m_array.draw(nullptr);

		renderstate.flags.fill = true;
		renderstate.cull_face = GL_BACK;
		renderstate.use();
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("016_cartoon_torus");
}  // namespace
}  // namespace spu::oglplus
