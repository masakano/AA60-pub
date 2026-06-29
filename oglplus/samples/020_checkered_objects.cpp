//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/cube.hpp>
#include <shapes/plane.hpp>
#include <shapes/sphere.hpp>
#include <shapes/torus.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_light_refl;                                                                 \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       f_normal = mat3(u_nodeworld)*a_normal;                                          \n"
    "       f_light_refl = reflect(                                                         \n"
    "               -normalize(f_light_dir),                                                \n"
    "               normalize(f_normal)                                                     \n"
    "       );                                                                              \n"
    "       f_view_dir = (                                                                  \n"
    "               vec4(0.0, 0.0, 1.0, 1.0)*                                               \n"
    "               u_worldview                                                             \n"
    "       ).xyz;                                                                          \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               gl_Position;                                                            \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform int u_srepeat;                                                                 \n"
    "uniform int u_trepeat;                                                                 \n"
    "uniform vec3 u_color1;                                                                 \n"
    "uniform vec3 u_color2;                                                                 \n"
    "uniform float u_refl1;                                                                 \n"
    "uniform float u_refl2;                                                                 \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_light_refl;                                                                  \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"

    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = length(f_light_dir);                                                  \n"
    "       float d = dot(                                                                  \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light_dir)                                                  \n"
    "       ) / l;                                                                          \n"
    "       float s = dot(                                                                  \n"
    "               normalize(f_light_refl),                                                \n"
    "               normalize(f_view_dir)                                                   \n"
    "       );                                                                              \n"
    "       float c = (                                                                     \n"
    "               int(f_texcoord.x*u_srepeat) % 2+                                        \n"
    "               int(f_texcoord.y*u_trepeat) % 2                                         \n"
    "       ) % 2;                                                                          \n"
    "       vec3 lt = vec3(1.0, 1.0, 1.0);                                                  \n"
    "       vec3 chkr = mix(u_color1, u_color2, c);                                         \n"
    "       final_color = vec4(                                                             \n"
    "               chkr * 0.3 +                                                            \n"
    "               (lt + chkr) * 1.5 * max(d, 0.0) +                                       \n"
    "               lt * pow(max(s, 0.0), mix(u_refl1, u_refl2, c)),                        \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};
/* clang-format on */

class Uniforms {
public:
	Vec3f u_light_position;
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	int32_t u_srepeat;
	int32_t u_trepeat;
	Vec3f u_color1;
	Vec3f u_color2;
	float u_refl1;
	float u_refl2;

	Uniforms()
	{
		m_attrs = {
		        {"u_light_position", &u_light_position},
		        {"u_viewsceen",      &u_viewsceen     },
		        {"u_worldview",      &u_worldview     },
		        {"u_nodeworld",      &u_nodeworld     },
		        {"u_srepeat",        &u_srepeat       },
		        {"u_trepeat",        &u_trepeat       },
		        {"u_color1",         &u_color1        },
		        {"u_color2",         &u_color2        },
		        {"u_refl1",          &u_refl1         },
		        {"u_refl2",          &u_refl2         },
		};
	}
	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

template<class shape_t> class CheckerArray : public shapes::Array {
public:
	explicit CheckerArray(const shape_t &shape, const Uniforms &unifs)
	{
		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};
		initShader(shader_attrs, Attrs(unifs));
		initArray(shape, {"position", "normal", "texcoord"});
	}
};

class App : public SpuPage {
public:
	Uniforms m_unifs;
	CheckerArray<shapes::Plane> m_plane;
	CheckerArray<shapes::Sphere> m_sphere;
	CheckerArray<shapes::Cube> m_cube;
	CheckerArray<shapes::Torus> m_torus;

	App(const char *name)
	        : SpuPage(name, true, {0.6, 0.6, 0.5, 0.0}),
	          m_plane(shapes::Plane(Vec3f(5, 0, 0), Vec3f(0, 0, -5)), m_unifs),
	          m_sphere(shapes::Sphere(), m_unifs), m_cube(shapes::Cube(), m_unifs),
	          m_torus(shapes::Torus(), m_unifs)
	{
	}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_BACK;

		m_unifs.u_light_position = {4.0, 10.0, 4.0};
	}

	void render() override
	{
		auto esec = getSeconds().current();
		m_unifs.u_viewsceen = math::perspective(viewport(0), 60, 1, 30);
		m_unifs.u_worldview = Mat4f::orbiting(ezero(), esec, 10, 0, 0, 0, 12, 45, 40, 20);

		// Render the plane
		m_unifs.u_nodeworld = math::unit();
		m_unifs.u_srepeat = 24;
		m_unifs.u_trepeat = 24;
		m_unifs.u_color1 = {1.0, 1.0, 0.9};
		m_unifs.u_color2 = {1.0, 0.9, 0.8};
		m_unifs.u_refl1 = 64;
		m_unifs.u_refl2 = 8;
		m_plane.draw(nullptr);

		// Render the sphere
		m_unifs.u_nodeworld = math::unit().trans({0.0, 1.5, 0.0})
		                    * math::unit().rot("x", -esec / 9.0 * math::two_pi());

		m_unifs.u_srepeat = 36;
		m_unifs.u_trepeat = 24;
		m_unifs.u_color1 = {0.5, 0.6, 1.0};
		m_unifs.u_color2 = {0.2, 0.3, 0.7};
		m_unifs.u_refl1 = 64;
		m_unifs.u_refl2 = 32;
		m_sphere.draw(nullptr);

		m_unifs.u_nodeworld = math::unit().trans({3.0, 1.5, 0.0})
		                    * math::unit().rot("z", esec / 8.0 * math::two_pi());

		m_unifs.u_color1 = {0.5, 1.0, 0.6};
		m_unifs.u_color2 = {0.2, 0.7, 0.3};
		m_unifs.u_refl1 = 64;
		m_unifs.u_refl2 = 64;
		m_torus.draw(nullptr);

		// Render the cube
		m_unifs.u_nodeworld = math::unit().trans({-2.0, 1.5, 0.0})
		                    * Mat4f(Quatf(esec / 7.0 * math::two_pi(), Vec3f(1, 1, 1)));

		m_unifs.u_srepeat = 8;
		m_unifs.u_trepeat = 8;
		m_unifs.u_color1 = {1.0, 0.6, 0.5};
		m_unifs.u_color2 = {0.7, 0.3, 0.2};
		m_unifs.u_refl1 = 64;
		m_unifs.u_refl2 = 8;
		m_cube.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("020_checkered_objects");
}  // namespace
}  // namespace spu::oglplus
