//
// Shape :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/cube.hpp>
#include <shapes/sphere.hpp>
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
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_normal = mat3(u_nodeworld)*a_normal;                                          \n"
    "       f_light = u_light_pos - gl_Position.xyz;                                        \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_prologue_frag =  {
    "#version 330                                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float len = dot(f_light, f_light);                                              \n"
    "       float dot = len > 0.0 ? dot(                                                    \n"
    "               f_normal,                                                               \n"
    "               normalize(f_light)                                                      \n"
    "       ) / len : 0.0;                                                                  \n"
    "       float intensity = 0.2 + max(dot, 0.0) * 4.0;                                    \n"
};

const char *c_epilogue_frag =  {
    "       final_color = vec4(u_color * intensity, 1.0);                                   \n"
    "}                                                                                      \n"
};

const char *c_bw_checker_frag =  {
    "       float c = (                                                                     \n"
    "               1 +                                                                     \n"
    "               int(f_texcoord.x*8) % 2+                                                \n"
    "               int(f_texcoord.y*8) % 2                                                 \n"
    "       ) % 2;                                                                          \n"
    "       vec3 u_color = vec3(c, c, c);                                                   \n"
};

const char *c_yb_strips_frag =  {
    "       float m = int((f_texcoord.x+f_texcoord.y)*16) % 2;                              \n"
    "       vec3 u_color = mix(                                                             \n"
    "               vec3(0.0, 0.0, 0.0),                                                    \n"
    "               vec3(1.0, 1.0, 0.0),                                                    \n"
    "               m                                                                       \n"
    "       );                                                                              \n"
};

const char *c_wo_vstrips_frag =  {
    "       float m = int(f_texcoord.x*8) % 2;                                              \n"
    "       vec3 u_color = mix(                                                             \n"
    "               vec3(1.0, 0.6, 0.1),                                                    \n"
    "               vec3(1.0, 0.9, 0.8),                                                    \n"
    "               m                                                                       \n"
    "       );                                                                              \n"
};

const char *c_br_circles_frag =  {
    "       vec2  center = f_texcoord - vec2(0.5, 0.5);                                     \n"
    "       float m = int(sqrt(length(center))*16) % 2;                                     \n"
    "       vec3 u_color = mix(                                                             \n"
    "               vec3(1.0, 0.0, 0.0),                                                    \n"
    "               vec3(0.0, 0.0, 1.0),                                                    \n"
    "               m                                                                       \n"
    "       );                                                                              \n"
};

const char *c_wg_spirals_frag =  {
    "       vec2  center = (f_texcoord - vec2(0.5, 0.5)) * 16.0;                            \n"
    "       float l = length(center);                                                       \n"
    "       float t = atan(center.y, center.x)/(2.0*asin(1.0));                             \n"
    "       float m = int(l+t) % 2;                                                         \n"
    "       vec3 u_color = mix(                                                             \n"
    "               vec3(0.0, 1.0, 0.0),                                                    \n"
    "               vec3(1.0, 1.0, 1.0),                                                    \n"
    "               m                                                                       \n"
    "       );                                                                              \n"
};

template <class shape_t> class Shape {
private:
        shapes::Array m_array;
        Vec3f u_light_pos; 
        Mat4f u_viewsceen; 
        Mat4f u_worldview; 
        Mat4f u_nodeworld; 
        //std::vector<SpuCommand> m_ops;

public:
        Shape(const std::string &vert, const std::string &frag)
        {
		Attrs shader_attrs = {
			{"frag", frag.c_str()},
			{"vert", vert.c_str()},
		};
			
		Attrs unif_attrs = {
			{"u_viewsceen", &u_viewsceen},
			{"u_worldview", &u_worldview},
			{"u_nodeworld", &u_nodeworld},
			{"u_light_pos", &u_light_pos},
		};
		m_array.initShader(shader_attrs, unif_attrs);

		shape_t shape;
		m_array.initArray(shape, {"position", "normal", "texcoord"});
        }
        void setViewscreen(const Mat4f &viewscreen)
        {
                u_viewsceen = viewscreen;
        }

	void render(const Vec3f &light, const Mat4f &worldview, const Mat4f &model)	
        {
                u_light_pos = light;
                u_nodeworld = model;
                u_worldview = worldview;
                m_array.draw(nullptr);
        }
};

/* clang-format on */
class App : public SpuPage {
public:
	std::string makeFs(const char *body_frag)
	{
		return std::string(c_prologue_frag) + body_frag + c_epilogue_frag;
	}

	Shape<shapes::Sphere> m_sphere;
	Shape<shapes::Cube> m_cubeX;
	Shape<shapes::Cube> m_cubeY;
	Shape<shapes::Cube> m_cubeZ;
	Shape<shapes::Torus> m_torus;

	App(const char *name)
	        : SpuPage(name, true, {0.5, 0.5, 0.5, 0.0}), m_sphere(c_vert, makeFs(c_yb_strips_frag)),
	          m_cubeX(c_vert, makeFs(c_bw_checker_frag)), m_cubeY(c_vert, makeFs(c_br_circles_frag)),
	          m_cubeZ(c_vert, makeFs(c_wg_spirals_frag)), m_torus(c_vert, makeFs(c_wo_vstrips_frag))
	{
	}

	void init(const Attrs &attrs) override { SpuPage::init(attrs); }

	void render() override
	{
		auto esec = getSeconds().current();
		auto viewscreen = math::perspective(viewport(0), 68, 1, 50);
		auto worldview = Mat4f::orbiting(ezero(), esec, 6, 0, 0, 0, 24, 0, 45, 6);
		auto light = Vec3f(2.0, 2.0, 2.0);

		m_sphere.setViewscreen(viewscreen);
		m_cubeX.setViewscreen(viewscreen);
		m_cubeY.setViewscreen(viewscreen);
		m_cubeZ.setViewscreen(viewscreen);
		m_torus.setViewscreen(viewscreen);

		m_sphere.render(light, worldview, math::unit());

		m_cubeX.render(
		        light, worldview,
		        math::unit().trans({2.0, 0.0, 0.0}) * math::unit().rot("X", esec * 45));

		m_cubeY.render(
		        light, worldview,
		        math::unit().trans({0.0, 2.0, 0.0}) * math::unit().rot("Y", esec * 90));

		m_cubeZ.render(
		        light, worldview,
		        math::unit().trans({0.0, 0.0, 2.0}) * math::unit().rot("Y", esec * 135));

		m_torus.render(
		        light, worldview,
		        math::unit().trans(-eone()) * Mat4f(Quatf(radians(esec * 45), Vec3f(1, 1, 1)))
		                * math::unit().rot("XY", 45.0, 45.0));
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("020_shaded_objects");
}  // namespace
}  // namespace spu::oglplus
