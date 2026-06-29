//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/cube.hpp>
#include <shapes/torus.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_worldview * u_nodeworld * a_position;                           \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 8) out;                                          \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform vec4 u_light_pos_cam;                                                          \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out float f_opacity;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec4 c = vec4((                                                                 \n"
    "               gl_in[0].gl_Position.xyz+                                               \n"
    "               gl_in[1].gl_Position.xyz+                                               \n"
    "               gl_in[2].gl_Position.xyz                                                \n"
    "       ) * 0.333333, 1.0);                                                             \n"
    "       for (int v = 0; v != 4; ++v)                                                    \n"
    "       {                                                                               \n"
    "               vec4 b = gl_in[v%3].gl_Position;                                        \n"
    "               vec4 a = vec4(                                                          \n"
    "                       b.xyz + (c.xyz - b.xyz)*0.3,                                    \n"
    "                       1.0                                                             \n"
    "               );                                                                      \n"

    "               gl_Position = u_viewsceen * a;                                          \n"
    "               f_light_dir = (u_light_pos_cam - a).xyz;                                \n"
    "               f_opacity = 1.0;                                                        \n"
    "               EmitVertex();                                                           \n"
    "               gl_Position = u_viewsceen * b;                                          \n"
    "               f_light_dir = (u_light_pos_cam - b).xyz;                                \n"
    "               f_opacity = 0.0;                                                        \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in float f_opacity;                                                                    \n"
    "uniform vec3 u_front_color;                                                            \n"
    "uniform vec3 u_back_color;                                                             \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = length(f_light_dir);                                                  \n"
    "       vec3 color = gl_FrontFacing?                                                    \n"
    "               u_front_color:                                                          \n"
    "               u_back_color;                                                           \n"
    "       final_color = vec4(color*(4.0/l), f_opacity);                                   \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos_cam;
	Vec3f u_front_color;
	Vec3f u_back_color;

	App(const char *name) : SpuPage(name, true, {0.8, 0.7, 0.6, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
                        {"geom", c_geom},
                        {"vert", c_vert}
                };

		Attrs unif_attrs = {
		        {"u_viewsceen",     &u_viewsceen    },
                        {"u_worldview",     &u_worldview    },
		        {"u_nodeworld",     &u_nodeworld    },
                        {"u_light_pos_cam", &u_light_pos_cam},
		        {"u_front_color",   &u_front_color  },
                        {"u_back_color",    &u_back_color   },
		};
		m_array.initShader(shader_attrs, unif_attrs);

		auto shape_torus = shapes::Torus(1.0, 0.5, 12, 12);
		m_array.initArray(shape_torus, {"position"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 20);
		u_worldview = Mat4f::orbiting(ezero(), esec, 4.5, 0, 0, 0, 10.25, 0, 60, 20);

		u_nodeworld = math::unit().rot("xy", -0.25 * math::two_pi(), -esec * 0.25 * math::two_pi());

		Vec4f light_pos(4.0, 4.0, -8.0, 1.0);
		u_light_pos_cam = u_worldview * light_pos;  // correct?
		u_front_color = {0.3, 0.2, 0.0};
		u_back_color = {0.2, 0.1, 0.0};

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.fill = false;
		renderstate.use();
		m_array.draw(nullptr);

		u_front_color = {0.9, 0.8, 0.1};
		u_back_color = {1.0, 0.9, 0.8};

		renderstate.flags.fill = true;
		renderstate.use();
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("023_lattice_torus");
}  // namespace
}  // namespace spu::oglplus
