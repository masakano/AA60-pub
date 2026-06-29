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
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_normal = mat3(u_worldview)*mat3(u_nodeworld)*a_normal;                        \n"
    "       gl_Position = u_viewsceen * u_worldview * u_nodeworld * a_position;             \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "                                                                                       \n"
    "uniform int u_color_count;                                                             \n"
    "uniform vec4 u_color[8];                                                               \n"
    "                                                                                       \n"
    "in vec3 f_normal;                                                                      \n"
    "                                                                                       \n"
    "vec3 view_dir = vec3(0.0, 0.0, 1.0);                                                   \n"
    "                                                                                       \n"
    "vec3 top_dir = vec3(0.0, 1.0, 0.0);                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float k = dot(f_normal, view_dir);                                              \n"
    "       vec3 refl_dir = 2.0*k*f_normal - view_dir;                                      \n"
    "       float a = dot(refl_dir, top_dir);                                               \n"
    "       vec3 refl_color;                                                                \n"
    "       for (int i = 0; i != (u_color_count - 1); ++i)                                  \n"
    "       {                                                                               \n"
    "               if (a < u_color[i].a && a >= u_color[i+1].a)                            \n"
    "               {                                                                       \n"
    "                       float m =                                                       \n"
    "                               (a - u_color[i].a)/ (u_color[i+1].a-u_color[i].a);      \n"
    "                       refl_color = mix(                                               \n"
    "                               u_color[i].rgb,                                         \n"
    "                               u_color[i+1].rgb,                                       \n"
    "                               m                                                       \n"
    "                       );                                                              \n"
    "                       break;                                                          \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       float i = max(dot(f_normal, top_dir), 0.0);                                     \n"
    "       vec3 diff_color = vec3(i, i, i);                                                \n"
    "       final_color = vec4(mix(refl_color, diff_color, 0.3 + i*0.7), 1.0);              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	std::vector<Vec4f> u_color = {
	        {1.0, 1.0, 0.9, +1.00},
                {1.0, 0.9, 0.8, +0.97},
                {0.9, 0.7, 0.5, +0.95},
                {0.5, 0.5, 1.0, +0.95},
	        {0.2, 0.2, 0.7, +0.00},
                {0.1, 0.1, 0.1, +0.00},
                {0.2, 0.2, 0.2, -0.10},
                {0.0, 0.0, 0.0, +0.00},
	};
	int32_t u_color_count = u_color.size();

	App(const char *name) : SpuPage(name, true, {0.1, 0.1, 0.1, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen",   &u_viewsceen  },
                        {"u_worldview",   &u_worldview  },
		        {"u_nodeworld",   &u_nodeworld  },
                        {"u_color",       &u_color[0]   },
		        {"u_color_count", &u_color_count},
		};
		m_array.initShader(shader_attrs, unif_attrs);

		auto torus_shape = shapes::Torus(1.0, 0.5, 72, 48);
		m_array.initArray(torus_shape, {"position", "normal"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_BACK;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 80, 1, 20);
		u_worldview = Mat4f::orbiting(ezero(), esec, 3.5, 0, 0, 0, 7.5, 0, 80, 60);
		u_nodeworld = math::unit().rot("x", -esec / 4 * math::two_pi());
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("016_metallic_torus");
}  // namespace
}  // namespace spu::oglplus
