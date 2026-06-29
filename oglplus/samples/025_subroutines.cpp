//
// CubeArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/newton.hpp>
#include <shapes/cube.hpp>

namespace spu::oglplus {

namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 400                                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"

    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec3 a_texcoord;                                                                    \n"

    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_light_refl;                                                                 \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld* a_position;                                          \n"
    "       f_normal = mat3(u_nodeworld)* a_normal;                                         \n"
    "       f_light_dir = normalize(u_light_position - gl_Position.xyz);                    \n"
    "       f_light_refl = reflect(-f_light_dir, f_normal);                                 \n"
    "       f_view_dir = normalize(u_eye_position - gl_Position.xyz);                       \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "       f_texcoord = a_texcoord.xy;                                                     \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 400                                                                           \n"
    "uniform vec3 u_color1;                                                                 \n"
    "uniform vec3 u_color2;                                                                 \n"
    "uniform vec2 u_tex_scale;                                                              \n"
    "uniform float u_specular_factor;                                                       \n"
    "uniform sampler2D u_texture;                                                           \n"
    "uniform int u_color_type;                                                              \n"
    "uniform int u_light_type;                                                              \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_light_refl;                                                                  \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "const vec3 light_color = vec3(1.0, 1.0, 1.0);                                          \n"
    "vec3 shiny(                                                                            \n"
    "       vec3 color,                                                                     \n"
    "       float ambient,                                                                  \n"
    "       float diffuse,                                                                  \n"
    "       float specular                                                                  \n"
    ")                                                                                      \n"
    "{                                                                                      \n"
    "       return (ambient+diffuse)*color +                                                \n"
    "               specular * light_color;                                                 \n"
    "}                                                                                      \n"
    "vec3 dull(                                                                             \n"
    "       vec3 color,                                                                     \n"
    "       float ambient,                                                                  \n"
    "       float diffuse,                                                                  \n"
    "       float specular                                                                  \n"
    ")                                                                                      \n"
    "{                                                                                      \n"
    "       return (ambient+diffuse*0.9)*color +                                            \n"
    "               specular*0.1 * light_color;                                             \n"
    "}                                                                                      \n"
    "vec3 checker(vec2 tex_coord)                                                           \n"
    "{                                                                                      \n"
    "       float a = (                                                                     \n"
    "               int(tex_coord.x)%2+                                                     \n"
    "               int(tex_coord.y)%2                                                      \n"
    "       )%2;                                                                            \n"
    "       return mix(u_color1, u_color2, a);                                              \n"
    "}                                                                                      \n"
    "vec3 strips(vec2 tex_coord)                                                            \n"
    "{                                                                                      \n"
    "       float a = int(tex_coord.x+tex_coord.y)%2;                                       \n"
    "       return mix(u_color1, u_color2, a);                                              \n"
    "}                                                                                      \n"
    "vec3 spiral(vec2 tex_coord)                                                            \n"
    "{                                                                                      \n"
    "       vec2  center = (tex_coord - vec2(0.5, 0.5)) * 16.0;                             \n"
    "       float l = length(center);                                                       \n"
    "       float t = atan(center.y, center.x)/(2.0*asin(1.0));                             \n"
    "       float m = int(l+t) % 2;                                                         \n"
    "       return mix(u_color1, u_color2, m);                                              \n"
    "}                                                                                      \n"
    "vec3 texture_rgb(vec2 tex_coord)                                                       \n"
    "{                                                                                      \n"
    "       return texture(u_texture, tex_coord).rgb;                                       \n"
    "}                                                                                      \n"
    "vec3 texture_bgr(vec2 tex_coord)                                                       \n"
    "{                                                                                      \n"
    "       return texture(u_texture, tex_coord).bgr;                                       \n"
    "}                                                                                      \n"
    "vec3 pixel_color_func(vec2 tex_coord)                                                  \n"
    "{                                                                                      \n"
    "       switch (u_color_type) {                                                         \n"
    "       case 0: return checker(tex_coord);                                              \n"
    "       case 1: return strips(tex_coord);                                               \n"
    "       case 2: return spiral(tex_coord);                                               \n"
    "       case 3: return texture_rgb(tex_coord);                                          \n"
    "       case 4: return texture_bgr(tex_coord);                                          \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
    "vec3 pixel_light_func(                                                                 \n"
    "       vec3 color,                                                                     \n"
    "       float ambient,                                                                  \n"
    "       float diffuse,                                                                  \n"
    "       float specular                                                                  \n"
    ")                                                                                      \n"
    "{                                                                                      \n"
    "       switch (u_light_type) {                                                         \n"
    "       case 0: return shiny(color, ambient, diffuse, specular);                        \n"
    "       case 1: return dull(color, ambient, diffuse, specular);                         \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float ambient = 0.5;                                                            \n"
    "       float diffuse = max(dot(                                                        \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light_dir)                                                  \n"
    "       )+0.1, 0.0);                                                                    \n"
    "       float specular = pow(clamp(dot(                                                 \n"
    "               normalize(f_view_dir),                                                  \n"
    "               normalize(f_light_refl)                                                 \n"
    "       ), 0.0, 1.0), u_specular_factor);                                               \n"

    "       vec3 color = pixel_color_func(f_texcoord * u_tex_scale);                        \n"
    "       final_color = pixel_light_func(                                                 \n"
    "               color,                                                                  \n"
    "               ambient,                                                                \n"
    "               diffuse,                                                                \n"
    "               specular                                                                \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

enum {
	e_light_type_shiny = 0,
	e_light_type_dull,
};
enum {
	e_color_type_checker = 0,
	e_color_type_strips,
	e_color_type_spiral,
	e_color_type_texture_rgb,
	e_color_type_texture_bgr,
};

/* clang-format on */
class CubeArray : public shapes::Array {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_eye_position = ezero();
	Vec3f u_light_position = ezero();
	Vec3f u_color1 = ezero();
	Vec3f u_color2 = ezero();
	Vec2f u_tex_scale = Vec2f(1.0);
	float u_specular_factor = 0;
	uint32_t u_texture = 0u;
	int32_t u_light_type = 0;
	int32_t u_color_type = 0;

	CubeArray()
	{
		Attrs shader_attrs = {
		        {"frag", c_frag},
                        {"vert", c_vert}
                };

		Attrs unif_attrs = {
		        {"u_viewsceen",       &u_viewsceen      },
		        {"u_worldview",       &u_worldview      },
		        {"u_nodeworld",       &u_nodeworld      },
		        {"u_eye_position",    &u_eye_position   },
		        {"u_light_position",  &u_light_position },
		        {"u_color1",          &u_color1         },
		        {"u_color2",          &u_color2         },
		        {"u_tex_scale",       &u_tex_scale      },
		        {"u_specular_factor", &u_specular_factor},
		        {"u_texture",         &u_texture        },
		        {"u_light_type",      &u_light_type     },
		        {"u_color_type",      &u_color_type     },
		};

		Array::initShader(shader_attrs, unif_attrs);
		u_light_type = e_light_type_shiny;
		u_color_type = e_color_type_checker;
	}
};

class App : public SpuPage {
public:
	CubeArray m_array;
	SpuTexture m_texture;

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_array.initArray(shapes::Cube(), {"position", "normal", "tangent", "texcoord"});

		// setup the texture
		{
			const auto c_tex_side = 512;

			auto image = images::NewtonFractal(
			        c_tex_side, c_tex_side, Vec3f(0.2, 0.1, 0.4), Vec3f(0.8, 0.8, 1.0),
			        Vec2f(-1, -1), Vec2f(1, 1), images::NewtonFractal::X4Minus1(),
			        images::NewtonFractal::DefaultMixer());

			Attrs tex_attrs = {
			        {"target",     GL_TEXTURE_2D },
                                {"iformat",    GL_RGB32F     },
			        {"width",      image.width() },
                                {"height",     image.height()},
			        {"min_filter", GL_LINEAR     },
                                {"mag_filter", GL_LINEAR     },
			        {"wrap_s",     GL_REPEAT     },
                                {"wrap_t",     GL_REPEAT     },
			        {"data",       image.data()  },
			};
			m_texture.init(tex_attrs);
			m_array.u_texture = m_texture.id();
		}
		m_array.u_light_position = {4, 4, -8};

		// need check
		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_BACK;
	}

	void render() override
	{
		auto esec = getSeconds().current();

		m_array.u_viewsceen = math::perspective(viewport(0), 60, 1, 25);
		m_array.u_worldview
		        = Mat4f::orbiting(Vec3f(0.0, 0.0, 0.0), esec, 7.0, 1.5, 11.0, 0, 19, 3, 30, 20);
		m_array.u_eye_position = m_array.u_worldview.unitary_inverse().c[3];

		// shiny gray/blue checkered cube
		{
			m_array.u_light_type = e_light_type_shiny;
			m_array.u_color_type = e_color_type_checker;
			m_array.u_specular_factor = 32;
			m_array.u_color1 = {0.9, 0.8, 0.7};
			m_array.u_color2 = {0.3, 0.4, 0.5};
			m_array.u_tex_scale = {4, 4};

			m_array.u_nodeworld = math::unit().rot("y", -esec / 7.0 * math::two_pi())
			                    * math::unit().trans({2.0, 0.0, 0.0})
			                    * math::unit().rot("X", 25 * esec);

			m_array.draw(nullptr);
		}

		// shiny textured cube
		{
			m_array.u_light_type = e_light_type_shiny;
			m_array.u_color_type = e_color_type_texture_rgb;
			m_array.u_specular_factor = 16;
			m_array.u_tex_scale = {1, 1};

			m_array.u_nodeworld = math::unit().rot("y", -esec / 7.0 * math::two_pi())
			                    * math::unit().trans({-2.0, 0.0, 0.0})
			                    * math::unit().rot("X", -17 * esec);

			m_array.draw(nullptr);
		}

		// shiny yellow/black striped cube
		{
			m_array.u_light_type = e_light_type_shiny;
			m_array.u_color_type = e_color_type_strips;
			m_array.u_specular_factor = 32;
			m_array.u_color1 = {0.9, 0.9, 0.1};
			m_array.u_color2 = {0.1, 0.1, 0.1};
			m_array.u_tex_scale = {16, 16};

			m_array.u_nodeworld = math::unit().rot("y", -esec / 7.0 * math::two_pi())
			                    * math::unit().trans({0.0, 2.0, 0.0})
			                    * math::unit().rot("Y", 37 * esec);

			m_array.draw(nullptr);
		}

		// shiny gray/green spiral cube
		{
			m_array.u_light_type = e_light_type_shiny;
			m_array.u_color_type = e_color_type_spiral;
			m_array.u_specular_factor = 24;
			m_array.u_color1 = {0.9, 0.9, 0.9};
			m_array.u_color2 = {0.4, 0.9, 0.4};
			m_array.u_tex_scale = {1, 1};

			m_array.u_nodeworld = math::unit().rot("y", -esec / 7.0 * math::two_pi())
			                    * math::unit().trans({0.0, -2.0, 0.0})
			                    * math::unit().rot("Y", -13 * esec);

			m_array.draw(nullptr);
		}

		// dull white/red striped cube
		{
			m_array.u_light_type = e_light_type_dull;
			m_array.u_color_type = e_color_type_strips;
			m_array.u_specular_factor = 32;
			m_array.u_color2 = eone();
			m_array.u_color1 = {0.9, 0.2, 0.2};
			m_array.u_tex_scale = {8, 6};

			m_array.u_nodeworld = math::unit().rot("y", -esec / 7.0 * math::two_pi())
			                    * math::unit().trans({0.0, 0.0, 2.0})
			                    * math::unit().rot("Z", 27 * esec);

			m_array.draw(nullptr);
		}

		// dull textured cube
		{
			m_array.u_light_type = e_light_type_dull;
			m_array.u_color_type = e_color_type_texture_bgr;
			m_array.u_tex_scale = {1, 1};

			m_array.u_nodeworld = math::unit().rot("y", -esec / 7.0 * math::two_pi())
			                    * math::unit().trans({0.0, 0.0, -2.0})
			                    * math::unit().rot("Z", -23 * esec);

			m_array.draw(nullptr);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("025_subroutines");
}  // namespace
}  // namespace spu::oglplus
