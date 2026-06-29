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
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_color;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "uniform mat4 u_viewsceen,                                                              \n"
    "u_worldview, u_nodeworld;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_color = a_normal;                                                             \n"
    "       f_normal = mat3(u_nodeworld)* a_normal;                                         \n"
    "       f_light = u_light_pos - gl_Position.xyz;                                        \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_color;                                                                       \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = dot(f_light, f_light);                                                \n"
    "       float d = l > 0.0 ? dot(f_normal,                                               \n"
    "normalize(f_light)) / l : 0.0;                                                         \n"
    "       float i = 0.2 + max(d * 2.2, 0.0);                                              \n"
    "       final_color = vec4(abs(f_color)*i, 1.0);                                        \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_cubeArray;
	SpuArray m_planeArray;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		// cube array
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
			};
			m_cubeArray.initShader(shader_attrs, unif_attrs);
			u_light_pos = {1.5, 2.0, 2.5};
			m_cubeArray.initArray(shapes::Cube(), {"position", "normal"});
		}

		// plane array
		{
			float data0[4 * 3] = {-2.0, 0.0, 2.0, -2.0, 0.0, -2.0, +2.0, 0.0, 2.0, +2.0, 0.0, -2.0};
			Attrs attrs0 = {
			        {"shader_id",    m_cubeArray.getAShader().id()},
			        {"data",         &data0[0]      },
			        {"nelem",        4                            },
			        {"a.a_position", 3                            },
			};
			m_planeArray.aux(attrs0, 0);

			float data1[4 * 3] = {-0.1, 1.0, 0.1, -0.1, 1.0, -0.1, +0.1, 1.0, 0.1, +0.1, 1.0, -0.1};
			Attrs attrs1 = {
			        {"data",       &data1[0]},
			        {"nelem",      4                      },
			        {"a.a_normal", 3                      },
			};
			m_planeArray.aux(attrs1, 1);
		}

		// background
		{
			Vec4f bgcolor = {0.2, 0.2, 0.2, 0.0};
			int32_t bgstencil = 0.0;
			Attrs frame_attrs = {
			        {"bgcolor0",  bgcolor  },
			        {"bgstencil", bgstencil},
			};
			SpuPage::set(frame_attrs);
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 20);
		u_worldview = Mat4f::orbiting(ezero(), esec, 5, 0, 0, 0, 36, 52.5, -37.5, 12.5);

		auto model = math::unit().trans({0.0, 1.5, 0.0}) * Mat4f(Quatf(radians(esec * 90), eone()));
		auto &renderstate = SpuPage::getRenderstate();

		// draw the stencil plane
		{
			renderstate.flags.blend = false;
			renderstate.flags.depth_test = false;
			renderstate.flags.stencil_test = true;

			renderstate.write_mask = {0, 0, 0, 0, 1};
			renderstate.stencil_func = {GL_ALWAYS, 1, 1, GL_KEEP, GL_KEEP, GL_REPLACE,
			                            GL_ALWAYS, 1, 1, GL_KEEP, GL_KEEP, GL_REPLACE};
			renderstate.use();
			u_nodeworld = math::unit();

			m_cubeArray.getAShader().use();  // use cubeArray shader
			m_planeArray.draw(GL_TRIANGLE_STRIP);
		}

		// draw the reflected cube with stencil test
		{
			renderstate.flags.depth_test = true;
			renderstate.write_mask = {1, 1, 1, 1, 1};
			renderstate.stencil_func = {
			        GL_EQUAL, 1, 1, GL_KEEP, GL_KEEP, GL_KEEP,
			        GL_EQUAL, 1, 1, GL_KEEP, GL_KEEP, GL_KEEP,
			};
			renderstate.use();

			auto reflection = Mat4f().scale({+1, -1, +1});
			u_nodeworld = reflection * model;
			m_cubeArray.draw(nullptr);
		}

		// draw the cube
		{
			renderstate.flags.stencil_test = false;
			renderstate.use();

			u_nodeworld = model;
			m_cubeArray.draw(nullptr);
		}

		// draw the plane
		{
			renderstate.flags.blend = true;
			renderstate.blend_eq = {GL_MAX, GL_MAX};
			renderstate.use();

			u_nodeworld = math::unit();
			m_cubeArray.getAShader().use();  // use cubeArray shader
			m_planeArray.draw(GL_TRIANGLE_STRIP);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("023_reflected_cube");
}  // namespace
}  // namespace spu::oglplus
