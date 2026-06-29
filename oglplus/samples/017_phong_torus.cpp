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
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_view_dir;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_normal = a_normal;                                                            \n"
    "       f_view_dir = (                                                                  \n"
    "               vec4(0.0, 0.0, 1.0, 1.0)*                                               \n"
    "               u_worldview                                                             \n"
    "       ).xyz;                                                                          \n"
    "       gl_Position = u_viewsceen * u_worldview *  a_position;                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_view_dir;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "uniform vec4 u_light_pos[3];                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float amb = 0.2;                                                                \n"
    "       float diff = 0.0;                                                               \n"
    "       float spec = 0.0;                                                               \n"
    "       for (int i=0;i!=3;++i)                                                          \n"
    "       {                                                                               \n"
    "               diff += max(                                                            \n"
    "                       dot(f_normal,  u_light_pos[i].xyz)/                             \n"
    "                       dot(u_light_pos[i].xyz, u_light_pos[i].xyz),                    \n"
    "                       0.0                                                             \n"
    "               );                                                                      \n"
    "               float k = dot(f_normal, u_light_pos[i].xyz);                            \n"
    "               vec3 r = 2.0*k*f_normal - u_light_pos[i].xyz;                           \n"
    "               spec += pow(max(                                                        \n"
    "                       dot(normalize(r), f_view_dir),                                  \n"
    "                       0.0                                                             \n"
    "               ), 32.0 * dot(r, r));                                                   \n"
    "       }                                                                               \n"
    "       final_color =                                                                   \n"
    "               vec4(1.0, 0.1, 0.3, 1.0)*(amb+diff)+                                    \n"
    "               vec4(1.0, 1.0, 1.0, 1.0)*spec;                                          \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos[3];

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		// Set the vertex shader source
		{
			Attrs shader_attrs = {
			        {"frag", c_frag},
			        {"vert", c_vert},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen   },
			        {"u_worldview", &u_worldview   },
			        {"u_nodeworld", &u_nodeworld   },
			        {"u_light_pos", &u_light_pos[0]},
			};

			m_array.initShader(shader_attrs, unif_attrs);

			u_light_pos[0] = {2, -1, 0};
			u_light_pos[1] = {0.0, 3.0, 0.0};
			u_light_pos[2] = {0, -1, 4};
		}

		{
			auto torus_shape = shapes::Torus(1.0, 0.5, 72, 48);
			m_array.initArray(torus_shape, {"position", "normal"});

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.flags.clip_distance0 = true;
			renderstate.cull_face = GL_BACK;
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 30);
		u_worldview = Mat4f::orbiting(ezero(), esec, 5.0, 0, 0, 0, 2.66, 0, 90, 20);
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("017_phong_torus");
}  // namespace
}  // namespace spu::oglplus
