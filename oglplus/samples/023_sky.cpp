//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>
#include <shapes/sky_box.hpp>
#include <shapes/wicker_torus.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_sky_box_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "mat4 u_matrix = u_viewsceen*u_worldview;                                               \n"
    "in vec3 a_position;                                                                    \n"
    "out vec3 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_matrix * vec4(a_position * 100.0, 1.0);                         \n"
    "       f_texcoord = a_position;                                                        \n"
    "}                                                                                      \n"
};

const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_sun_position;                                                           \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_normal, f_view_refl, f_light_dir;                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_normal = mat3(u_nodeworld)* a_normal;                                         \n"
    "       f_view_refl = reflect(                                                          \n"
    "               gl_Position.xyz - u_eye_position,                                       \n"
    "               f_normal                                                                \n"
    "       );                                                                              \n"
    "       f_light_dir = u_sun_position - gl_Position.xyz;                                 \n"
    "       gl_Position = u_viewsceen*u_worldview*gl_Position;                              \n"
    "}                                                                                      \n"
};

const char *c_sky_box_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "vec3 sky_color(vec3 vd);                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = sky_color(normalize(f_texcoord));                                 \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal, f_view_refl, f_light_dir;                                            \n"
    "out vec3 final_color;                                                                  \n"
    "vec3 sky_color(vec3 vd);                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = max(dot(normalize(f_normal), normalize(f_light_dir))+0.1, 0.0);       \n"
    "       float a = 0.1;                                                                  \n"
    "       final_color =                                                                   \n"
    "               0.1*vec3(1.0, 1.0, 1.0)*(a+l)+                                          \n"
    "               0.9*sky_color(normalize(f_view_refl));                                  \n"
    "}                                                                                      \n"
};

const char *c_sky_frag = {
        //"#version 330\n" // for concatinate
    "                                                                                       \n"
    "const float world_radius = 6371000;                                                    \n"
    "const float atm_thickness = 50000;                                                     \n"
    "const vec3 air_color = vec3(0.32, 0.36, 0.45);                                         \n"
    "const vec3 light_color = vec3(1.0, 1.0, 1.0);                                          \n"
    "uniform vec3 u_sun_position;                                                           \n"
    "uniform samplerCube u_texture;                                                         \n"
    "float atm_intersection(vec3 v)                                                         \n"
    "{                                                                                      \n"
    "       const vec3 c = vec3(0.0, -world_radius, 0.0);                                   \n"
    "       const float r = world_radius + atm_thickness;                                   \n"
    "       const float c_c = dot(-c, -c);                                                  \n"
    "       float v_c = dot( v, -c);                                                        \n"
    "       return (-v_c + sqrt(v_c*v_c - c_c + r*r))/atm_thickness;                        \n"
    "}                                                                                      \n"
    "vec3 sky_color(vec3 vd)                                                                \n"
    "{                                                                                      \n"
    "       vec3 up = vec3(0.0, 1.0, 0.0);                                                  \n"
    "       vec3 ld = normalize(u_sun_position);                                            \n"
    "       vec4 cl = texture(u_texture, vd);                                               \n"
    "       float ai = atm_intersection(vd);                                                \n"
    "       float al = max(dot(ld, up) + 0.12, 0.0);                                        \n"
    "       float vl = max(dot(vd, ld), 0.0);                                               \n"
    "       float ct = (1.0-cl.a)*cl.b;                                                     \n"
    "       vec3 ac = max(light_color-air_color*pow(ai, 0.33), vec3(0.0, 0.0, 0.0));        \n"
    "       vec3 sun =                                                                      \n"
    "               ac*(vl>0.995+0.004*al ? 1.0:0.0);                                       \n"
    "       vec3 air =                                                                      \n"
    "               min(air_color*sqrt(pow(al,0.25)*ai), vec3(al, al, al)*1.5)+             \n"
    "               ac*pow(min(vl+0.001*ai, 1.0), 1024.0/pow(ai, 2.0))+                     \n"
    "               ac*(vl/(1.0+pow(3.0*al, 8.0)))*pow(ai, 0.6)*0.5;                        \n"
    "       vec3 clouds =                                                                   \n"
    "               ac*pow(min(vl*(cl.g+cl.b), 1.015), 64.0)*pow(ct, 2.0)+                  \n"
    "               ac*pow(min(vl*cl.g+cl.b, 1.020), 32.0)*ct+                              \n"
    "               ac*pow(min(vl*cl.g*cl.b, 1.010), 16.0)*pow(ct, 0.5)+                    \n"
    "               ac*0.7*min(cl.g + cl.b*0.5, 1.0)*al+                                    \n"
    "               ac*(cl.g*(1.0-cl.b*0.2)*5.0)*pow(1.0-al, 2.0)*(al)+                     \n"
    "               light_color*0.5*min(al + cl.g*0.4+cl.b*0.1, 1.0)*sqrt(al);              \n"
    "       return mix(air, clouds, cl.a*(1.0-cl.r*0.8))+sun*(1.0-cl.a);                    \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_skyArray;
	shapes::Array m_shapeArray;

	SpuTexture m_texture;

	uint32_t u_texture;
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_sun_position;
	Vec3f u_eye_position;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		auto skybox_frag = std::string(c_sky_box_frag) + c_sky_frag;

		Attrs skybox_shader_attrs = {
		        {"frag", skybox_frag   },
		        {"vert", c_sky_box_vert},
		};

		auto shape_frag = std::string(c_shape_frag) + c_sky_frag;

		Attrs shape_shader_attrs = {
		        {"frag", shape_frag  },
		        {"vert", c_shape_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen",    &u_viewsceen   },
                        {"u_worldview",    &u_worldview   },
		        {"u_nodeworld",    &u_nodeworld   },
                        {"u_sun_position", &u_sun_position},
		        {"u_eye_position", &u_eye_position},
                        {"u_texture",      &u_texture     },
		};

		m_skyArray.initShader(skybox_shader_attrs, unif_attrs);
		m_skyArray.initArray(shapes::SkyBox(), {"position"});

		m_shapeArray.initShader(shape_shader_attrs, unif_attrs);
		m_shapeArray.initArray(shapes::WickerTorus(), {"position", "normal", "tangent", "texcoord"});

		images::Image image0 = images::LoadTexture("clouds01-cm_0", false, false);
		images::Image image1 = images::LoadTexture("clouds01-cm_1", false, false);
		images::Image image2 = images::LoadTexture("clouds01-cm_2", false, false);
		images::Image image3 = images::LoadTexture("clouds01-cm_3", false, false);
		images::Image image4 = images::LoadTexture("clouds01-cm_4", false, false);
		images::Image image5 = images::LoadTexture("clouds01-cm_5", false, false);

		auto unit = image0.dataSize();
		std::vector<uint8_t> pix(unit * 6);

		memcpy(&pix[unit * 0], image0.data(), image0.dataSize());
		memcpy(&pix[unit * 1], image1.data(), image1.dataSize());
		memcpy(&pix[unit * 2], image2.data(), image2.dataSize());
		memcpy(&pix[unit * 3], image3.data(), image3.dataSize());
		memcpy(&pix[unit * 4], image4.data(), image4.dataSize());
		memcpy(&pix[unit * 5], image5.data(), image5.dataSize());

		int32_t cube_target[6] = {
		        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};

		Attrs tex_attrs = {
		        {"target",      GL_TEXTURE_CUBE_MAP     },
                        {"iformat",     GL_RGBA8                },
		        {"width",       image0.width()          },
                        {"height",      image0.height()         },
		        {"min_filter",  GL_LINEAR               },
                        {"mag_filter",  GL_LINEAR               },
		        {"wrap_s",      GL_CLAMP_TO_EDGE        },
                        {"wrap_t",      GL_CLAMP_TO_EDGE        },
		        {"data",        (const void *)pix.data()},
                        {"cube_target", cube_target             },
		};
		m_texture.init(tex_attrs);
		u_texture = m_texture.id();
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto day_duration = 67.0;
		auto sun = Vec3f(0.000, 1.000, 0.000) * 1e10 * sin(esec / day_duration * math::two_pi())
		         + Vec3f(0.000, 0.000, -1.000) * 1e10 * cos(esec / day_duration * math::two_pi());

		u_viewsceen = math::perspective(viewport(0), 70, 1, 200);
		u_worldview = Mat4f::orbiting(ezero(), esec, 5, 0, 0, -0.1, 27, -20, -30, 17);
		u_nodeworld = Mat4f(Quatf(esec / 13.0 * math::two_pi(), Vec3f(1, 1, 1)));
		u_sun_position = sun;
		m_skyArray.draw(nullptr);

		u_eye_position = u_worldview.unitary_inverse().c[3];
		m_shapeArray.draw(nullptr);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("023_sky");
}  // namespace
}  // namespace spu::oglplus
