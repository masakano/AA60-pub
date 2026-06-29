//
// SphereArray :
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
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "out vec3 f_normal_view;                                                                \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_normal = mat3(u_nodeworld)*a_normal;                                          \n"
    "       f_normal_view = mat3(u_worldview)*f_normal;                                     \n"
    "       f_light = u_light_pos - gl_Position.xyz;                                        \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_prologue_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "in vec3 f_normal_view;                                                                 \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float lighting = dot(                                                           \n"
    "               f_normal,                                                               \n"
    "               normalize(f_light)                                                      \n"
    "       );                                                                              \n"
    "       float intensity = clamp(                                                        \n"
    "               0.4 + lighting * 1.0,                                                   \n"
    "               0.0,                                                                    \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
};

const char *c_epilogue_frag =  {
    "       final_color = sig?                                                              \n"
    "               vec4(1.0, 1.0, 1.0, 1.0):                                               \n"
    "               vec4(color * intensity, 1.0);                                           \n"
    "}                                                                                      \n"
};

const char *c_proton_frag =  {
    "       bool sig = (                                                                    \n"
    "               abs(f_normal_view.x) < 0.5 &&                                           \n"
    "               abs(f_normal_view.y) < 0.2                                              \n"
    "       ) || (                                                                          \n"
    "               abs(f_normal_view.y) < 0.5 &&                                           \n"
    "               abs(f_normal_view.x) < 0.2                                              \n"
    "       );                                                                              \n"
    "       vec3 color = vec3(1.0, 0.0, 0.0);                                               \n"
};

const char *c_neutron_frag =  {
    "       bool sig = false;                                                               \n"
    "       vec3 color = vec3(0.5, 0.5, 0.5);                                               \n"
};

const char *c_electron_frag =  {
    "       bool sig = (                                                                    \n"
    "               abs(f_normal_view.x) < 0.5 &&                                           \n"
    "               abs(f_normal_view.y) < 0.2                                              \n"
    "       );                                                                              \n"
    "       vec3 color = vec3(0.0, 0.0, 1.0);                                               \n"
};

/* clang-format on */
class SphereArray : public shapes::Array {
private:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;

public:
	SphereArray(const std::string &vert, const std::string &frag)
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
		initShader(shader_attrs, unif_attrs);

		shapes::Sphere make_sphere;
		initArray(make_sphere, {"position", "normal"});
	}

	void setViewscreen(const Mat4f &viewscreen) { u_viewsceen = viewscreen; }

	void setLightAndCamera(const Vec3f &light, const Mat4f &worldview)
	{
		u_light_pos = light;
		u_worldview = worldview;
	}

	void render(const Mat4f &model)
	{
		u_nodeworld = model;
		draw(nullptr);
	}
};

class App : public SpuPage {
public:
	std::string makeFrag(const char *body_frag)
	{
		return std::string(c_prologue_frag) + body_frag + c_epilogue_frag;
	}

	SphereArray m_proton;
	SphereArray m_neutron;
	SphereArray m_electron;

	App(const char *name)
	        : SpuPage(name, true, {0.3, 0.3, 0.3, 0.0}), m_proton(c_vert, makeFrag(c_proton_frag)),
	          m_neutron(c_vert, makeFrag(c_neutron_frag)), m_electron(c_vert, makeFrag(c_electron_frag))
	{
	}

	void init(const Attrs &attrs) override { SpuPage::init(attrs); }

	void render() override
	{
		auto viewscreen = math::perspective(viewport(0), 68, 1, 50);
		m_proton.setViewscreen(viewscreen);
		m_neutron.setViewscreen(viewscreen);
		m_electron.setViewscreen(viewscreen);

		// make the light position vector
		auto esec = getSeconds().current();
		auto light = Vec3f(8.0, 8.0, 8.0);
		auto worldview = Mat4f::orbiting(ezero(), esec, 21, 0, 0, 0, 24, 0, 45, 6.66);
		auto nucl = Mat4f(Quatf(esec * math::two_pi(), Vec3f(1, 1, 1)));

		m_proton.setLightAndCamera(light, worldview);
		m_proton.render(nucl * math::unit().trans({+1.4, 0.0, 0.0}));
		m_proton.render(nucl * math::unit().trans({-1.4, 0.0, 0.0}));

		m_neutron.setLightAndCamera(light, worldview);
		m_neutron.render(nucl * math::unit().trans({0.0, 0.0, +1.0}));
		m_neutron.render(nucl * math::unit().trans(-ez()));

		m_electron.setLightAndCamera(light, worldview);

		m_electron.render(
		        math::unit().rot("y", -esec * 0.7 * math::two_pi())
		        * math::unit().trans({10.0, 0.0, 0.0}));
		m_electron.render(
		        math::unit().rot("x", -esec * 0.7 * math::two_pi())
		        * math::unit().trans({0.0, 0.0, 10.0}));
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("019_helium");
}  // namespace
}  // namespace spu::oglplus
