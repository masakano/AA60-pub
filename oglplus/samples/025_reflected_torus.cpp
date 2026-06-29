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
const char *c_norm_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_color;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_color = a_normal;                                                             \n"
    "       f_normal = mat3(u_nodeworld)* a_normal;                                         \n"
    "       f_light = u_light_pos-gl_Position.xyz;                                          \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_refl_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 g_normal;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       g_normal = a_normal;                                                            \n"
    "}                                                                                      \n"
};

const char *c_refl_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 6) out;                                          \n"

    "in vec3 g_normal[];                                                                    \n"

    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"

    "out vec3 f_color;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "uniform vec3 u_light_pos;                                                              \n"

    "mat4 reflection_matrix = mat4(                                                         \n"
    "       1.0, 0.0, 0.0, 0.0,                                                             \n"
    "       0.0,-1.0, 0.0, 0.0,                                                             \n"
    "       0.0, 0.0, 1.0, 0.0,                                                             \n"
    "       0.0, 0.0, 0.0, 1.0                                                              \n"
    ");                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       for (int v=0; v!=gl_in.length(); ++v)                                           \n"
    "       {                                                                               \n"
    "               vec4 position = gl_in[v].gl_Position;                                   \n"
    "               gl_Position = u_nodeworld * position;                                   \n"
    "               f_color = g_normal[v];                                                  \n"
    "               f_normal = mat3(u_nodeworld)*g_normal[v];                               \n"
    "               f_light = u_light_pos - gl_Position.xyz;                                \n"
    "               gl_Position =                                                           \n"
    "                       u_viewsceen *                                                   \n"
    "                       u_worldview *                                                   \n"
    "                       reflection_matrix *                                             \n"
    "                       gl_Position;                                                    \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
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
    "       float l = length(f_light);                                                      \n"
    "       float d = l > 0.0 ? dot(                                                        \n"
    "               f_normal,                                                               \n"
    "               normalize(f_light)                                                      \n"
    "        ) / l : 0.0;                                                                   \n"
    "       float i = 0.2 + max(d, 0.0) * 2.0;                                              \n"
    "       final_color = vec4(abs(f_normal)*i, 1.0);                                       \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	enum {
		e_normal = 0,
		e_reflect,
	};

	shapes::Array m_shapeArray;
	SpuArray m_planeArray;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_shapeArray.setMaxShaderType(2);  // 0:normal 1:reflect
		{
			Attrs normal_shader_attrs = {
			        {"frag", c_frag     },
			        {"vert", c_norm_vert},
			};

			Attrs reflect_shader_attrs = {
			        {"frag", c_frag     },
			        {"geom", c_refl_geom},
			        {"vert", c_refl_vert},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen},
			        {"u_worldview", &u_worldview},
			        {"u_nodeworld", &u_nodeworld},
			        {"u_light_pos", &u_light_pos},
			};
			m_shapeArray.initShader(normal_shader_attrs, unif_attrs, 0);
			m_shapeArray.initShader(reflect_shader_attrs, unif_attrs, 1);
			m_shapeArray.initArray(shapes::Torus(), {"position", "normal"});
		}

		u_light_pos = {2.0, 2.0, 3.0};
		{
			// clang-format off
			float data[4 * 3] = {
				-2.0, 0.0, 2.0, -2.0, 0.0, -2.0,
				+2.0, 0.0, 2.0, +2.0, 0.0, -2.0,
			};
			// clang-format on

			Attrs vert_attrs = {
			        {"shader_id",    m_shapeArray.getShaders().at(e_normal).id()},
			        {"data",         data                                       },
			        {"nelem",        4                                          },
			        {"a.a_position", 3                                          },
			};
			m_planeArray.init(vert_attrs);
		}
		{
			// clang-format off
			float data[4 * 3] = {
				-0.1, 1.0, 0.1, -0.1, 1.0, -0.1,
				+0.1, 1.0, 0.1, +0.1, 1.0, -0.1,
			};
			// clang-format on

			Attrs norm_attrs = {
			        {"data",       data},
			        {"nelem",      4   },
			        {"a.a_normal", 3   },
			};
			m_planeArray.aux(norm_attrs, 1);
		}

		auto bgcolor = Vec4f(0.2, 0.2, 0.2, 0.0);
		auto bgstencil = 0;
		Attrs frame_attrs = {
		        {"bgcolor0",  bgcolor  },
		        {"bgstencil", bgstencil},
		};
		SpuPage::set(frame_attrs);
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 65, 1, 40);

		auto model = math::unit().trans({0.0, 1.5, 0.0});
		u_worldview = Mat4f::orbiting(ezero(), esec, 6.5, 0, 0, 0, 2.67, 52.5, -37.5, 12.5);

		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.blend = false;
			renderstate.flags.depth_test = false;
			renderstate.flags.stencil_test = true;
			renderstate.write_mask = {0, 0, 0, 0, 1};

			renderstate.stencil_func = {
			        GL_ALWAYS, 1, 1, GL_KEEP, GL_KEEP, GL_REPLACE,
			        GL_ALWAYS, 1, 1, GL_KEEP, GL_KEEP, GL_REPLACE,
			};
			renderstate.use();

			u_nodeworld = math::unit();
			m_shapeArray.getShaders().at(e_reflect).use();
			m_planeArray.draw(GL_TRIANGLE_STRIP);
		}
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.write_mask = {1, 1, 1, 1, 1};
			renderstate.flags.depth_test = true;
			renderstate.stencil_func = {
			        GL_EQUAL, 1, 1, GL_KEEP, GL_KEEP, GL_KEEP,
			        GL_EQUAL, 1, 1, GL_KEEP, GL_KEEP, GL_KEEP,
			};
			renderstate.use();

			u_nodeworld = model;
			m_shapeArray.setShaderType(e_reflect);
			m_shapeArray.draw(nullptr);
		}
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.stencil_test = false;
			renderstate.use();
			u_nodeworld = model;

			m_shapeArray.setShaderType(e_normal);
			m_shapeArray.draw(nullptr);
		}
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.blend = true;
			renderstate.blend_eq = {GL_MAX, GL_MAX};
			renderstate.use();

			u_nodeworld = math::unit();
			m_shapeArray.getShaders().at(e_normal).use();
			m_planeArray.draw(GL_TRIANGLE_STRIP);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("025_reflected_torus");
}  // namespace
}  // namespace spu::oglplus
