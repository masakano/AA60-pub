//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/cube.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "uniform int u_inst_count;                                                              \n"
    "uniform int u_front_facing;                                                            \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out float f_mult;                                                                      \n"
    "out vec3 f_color;                                                                      \n"
    "out vec3 f_wrap_normal;                                                                \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       int inst = (u_front_facing != 0) ?                                              \n"
    "               (u_inst_count - gl_InstanceID - 1):                                     \n"
    "               gl_InstanceID;                                                          \n"
    "       f_mult = float(inst) / float(u_inst_count-1);                                   \n"
    "       float sca = 1.0 - 0.3 * pow(f_mult, 2);                                         \n"
    "       mat4 scale_matrix = mat4(                                                       \n"
    "               sca, 0.0, 0.0, 0.0,                                                     \n"
    "               0.0, sca, 0.0, 0.0,                                                     \n"
    "               0.0, 0.0, sca, 0.0,                                                     \n"
    "               0.0, 0.0, 0.0, 1.0                                                      \n"
    "       );                                                                              \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_color = a_normal;                                                             \n"
    "       vec3 wrap = a_position.xyz - a_normal;                                          \n"
    "       f_wrap_normal =                                                                 \n"
    "               mat3(u_nodeworld)*                                                      \n"
    "               normalize(mix(                                                          \n"
    "                       a_normal,                                                       \n"
    "                       wrap,                                                           \n"
    "                       mix(0.5, 1.0, f_mult)                                           \n"
    "               ));                                                                     \n"
    "       f_normal = mat3(u_nodeworld)*a_normal;                                          \n"
    "       f_light = u_light_pos-gl_Position.xyz;                                          \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               scale_matrix *                                                          \n"
    "               gl_Position;                                                            \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in float f_mult;                                                                       \n"
    "in vec3 f_color;                                                                       \n"
    "in vec3 f_wrap_normal;                                                                 \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "uniform int u_inst_count;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = dot(f_light, f_light);                                                \n"
    "       float d = l > 0.0 ? dot(                                                        \n"
    "               f_normal,                                                               \n"
    "               normalize(f_light)                                                      \n"
    "       ) / l : 0.0;                                                                    \n"
    "       float s = max(                                                                  \n"
    "               dot(f_wrap_normal, f_light)/l,                                          \n"
    "               0.0                                                                     \n"
    "       );                                                                              \n"
    "       float intensity = clamp(                                                        \n"
    "               0.2 + d * 3.0 + s * 5.5,                                                \n"
    "               0.0,                                                                    \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "       final_color = vec4(                                                             \n"
    "               abs(f_color) * intensity,                                               \n"
    "               (2.5 + 1.5*d + 1.5*s) / u_inst_count                                    \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	static constexpr auto c_inst_count = 12;

	shapes::Array m_array;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	int32_t u_front_facing;
	int32_t u_inst_count;
	Vec3f u_light_pos;

	App(const char *name) : SpuPage(name, true, {0.5, 0.6, 0.5, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
                        {"vert", c_vert},
		        //{"use_unif_block", true},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen",    &u_viewsceen   },
                        {"u_worldview",    &u_worldview   },
		        {"u_nodeworld",    &u_nodeworld   },
                        {"u_light_pos",    &u_light_pos   },
		        {"u_front_facing", &u_front_facing},
                        {"u_inst_count",   &u_inst_count  },
		};

		m_array.initShader(shader_attrs, unif_attrs);

		u_light_pos = {1.0, 2.0, 3.0};
		u_inst_count = c_inst_count;

		m_array.initArray(shapes::Cube(), {"position", "normal"});
		m_array.setInstanceCount(c_inst_count);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.blend = true;
		renderstate.flags.cull_face = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 70, 1, 20);
		u_worldview = Mat4f::orbiting(ezero(), esec, 3, 0, 0, 0, 7.2, 0, 80, 16);
		u_nodeworld = math::unit().rot("X", esec * 25);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.cull_face = GL_FRONT;
		renderstate.use();

		u_front_facing = 0;
		m_array.draw(nullptr);

		renderstate.cull_face = GL_BACK;
		renderstate.use();
		u_front_facing = 1;
		m_array.draw(nullptr);
		// m_array.getShaders().at(0).report("shader");
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("019_subsurf_scatter");
}  // namespace
}  // namespace spu::oglplus
