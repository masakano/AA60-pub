//
// VaseArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/newton.hpp>
#include <shapes/revolve.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                            \n"
    "uniform vec3 u_light_position;                                                          \n"
    "uniform vec3 u_eye_position;                                                            \n"
    "uniform mat4 u_viewscreen;                                                              \n"
    "uniform mat4 u_worldview;                                                               \n"
    "uniform mat4 u_nodeworld;                                                               \n"
    "in vec4 a_position;                                                                     \n"
    "in vec3 a_normal;                                                                       \n"
    "in vec3 a_texcoord;                                                                     \n"
    "out vec3 f_normal;                                                                      \n"
    "out vec3 f_light_dir;                                                                   \n"
    "out vec3 f_light_refl;                                                                  \n"
    "out vec3 f_view_dir;                                                                    \n"
    "out vec2 f_texcoord;                                                                    \n"
    "out float f_shadow;                                                                     \n"
    "void main()                                                                             \n"
    "{                                                                                       \n"
    "       gl_Position = u_nodeworld* a_position;                                           \n"
    "       f_normal = mat3(u_nodeworld)* a_normal;                                          \n"
    "       f_light_dir = normalize(u_light_position - gl_Position.xyz);                     \n"
    "       f_light_refl = reflect(-f_light_dir, f_normal);                                  \n"
    "       f_view_dir = normalize(u_eye_position - gl_Position.xyz);                        \n"
    "       gl_Position = u_viewscreen * u_worldview * gl_Position;                          \n"
    "       f_texcoord = a_texcoord.xy;                                                      \n"
    "       f_shadow = a_texcoord.z;                                                         \n"
    "}                                                                                       \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_texture;                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_light_refl;                                                                  \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec2 f_texcoord;                                                                    \n"
    "in float f_shadow;                                                                     \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float ambient = 0.2 + 0.3 * f_shadow;                                           \n"
    "       float diffuse = max(dot(                                                        \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light_dir)                                                  \n"
    "       )+0.1, 0.0) * f_shadow;                                                         \n"
    "       float specular = pow(clamp(dot(                                                 \n"
    "               normalize(f_view_dir),                                                  \n"
    "               normalize(f_light_refl)                                                 \n"
    "       ), 0.0, 1.0), 32.0) * f_shadow;                                                 \n"
    "       const vec3 light_color = vec3(1.0, 1.0, 1.0);                                   \n"
    "       vec3 texel = texture(u_texture, f_texcoord).rgb;                                \n"
    "       final_color =                                                                   \n"
    "               (ambient + diffuse)*texel +                                             \n"
    "               specular * light_color;                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class VaseArray : public shapes::Array {
public:
	Mat4f u_viewscreen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_eye_position, u_light_position;
	uint32_t u_texture;

	VaseArray()
	{
		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewscreen",     &u_viewscreen    },
                        {"u_worldview",      &u_worldview     },
		        {"u_nodeworld",      &u_nodeworld     },
                        {"u_eye_position",   &u_eye_position  },
		        {"u_light_position", &u_light_position},
                        {"u_texture",        &u_texture       },
		};
		Array::initShader(shader_attrs, unif_attrs);
	}
};

class App : public SpuPage {
public:
	VaseArray m_array;
	SpuTexture m_texture;

	std::vector<Vec3f> makeVasePositions()
	{
		std::vector<Vec3f> control_points = {
		        {0.00, 0.00, 0.00},
                        {0.20, 0.00, 0.00},
                        {0.40, 0.00, 0.00},
                        {0.80, 0.00, 0.00},
		        {1.45, 0.00, 0.00},
                        {1.45, 0.40, 0.00},
                        {1.40, 0.80, 0.00},
                        {1.35, 1.20, 0.00},
		        {1.20, 1.30, 0.00},
                        {1.00, 1.50, 0.00},
                        {0.80, 1.70, 0.00},
                        {0.80, 2.20, 0.00},
		        {0.90, 2.50, 0.00},
                        {1.00, 2.80, 0.00},
                        {0.98, 2.88, 0.00},
                        {1.05, 2.90, 0.00},
		        {1.12, 2.92, 0.00},
                        {1.09, 2.96, 0.00},
                        {1.05, 2.99, 0.00},
                        {1.01, 3.02, 0.00},
		        {0.96, 2.99, 0.00},
                        {0.93, 2.90, 0.00},
                        {0.90, 2.81, 0.00},
                        {0.88, 2.70, 0.00},
		        {0.82, 2.50, 0.00},
                        {0.74, 2.30, 0.00},
                        {0.70, 1.70, 0.00},
                        {0.88, 1.50, 0.00},
		        {1.06, 1.30, 0.00},
                        {1.25, 1.20, 0.00},
                        {1.30, 0.80, 0.00},
                        {1.35, 0.40, 0.00},
		        {1.35, 0.10, 0.00},
                        {0.80, 0.10, 0.00},
                        {0.40, 0.10, 0.00},
                        {0.25, 0.10, 0.00},
		        {0.00, 0.10, 0.00},
		};
		BezierCurves<Vec3f, float, 3> bezier;
		bezier.init(control_points);
		auto vecs = bezier.approximate(8);
		return std::vector<Vec3f>(vecs.begin(), vecs.end());
	}

	std::vector<Vec3f> makeVaseTexcoords()
	{
		std::vector<Vec3f> control_points = {
		        {5.00, -2.00, 1.00},
                        {5.00, -1.67, 1.00},
                        {5.00, -1.33, 1.00},
                        {5.00, -1.00, 1.00},
		        {5.00, -0.67, 1.00},
                        {5.00, -0.33, 1.00},
                        {5.00, +0.00, 1.00},
                        {5.00, +0.20, 1.00},
		        {5.00, +0.40, 1.00},
                        {5.00, +0.60, 1.00},
                        {5.00, +0.80, 1.00},
                        {5.00, +1.00, 1.00},
		        {5.00, +1.15, 1.00},
                        {5.00, +1.30, 1.00},
                        {5.00, +1.45, 0.40},
                        {5.00, +1.60, 0.60},
		        {5.00, +1.75, 1.00},
                        {5.00, +1.90, 1.00},
                        {5.00, +2.10, 1.00},
                        {5.00, +2.20, 1.00},
		        {5.00, +2.35, 1.00},
                        {5.00, +2.50, 1.00},
                        {5.00, +2.65, 1.00},
                        {5.00, +2.80, 1.00},
		        {5.00, +2.95, 1.00},
                        {5.00, +3.10, 0.90},
                        {5.00, +3.25, 0.50},
                        {5.00, +3.40, 0.10},
		        {5.00, +3.55, 0.00},
                        {5.00, +3.70, 0.00},
                        {5.00, +3.95, 0.00},
                        {5.00, +4.10, 0.00},
		        {5.00, +4.30, 0.00},
                        {5.00, +4.50, 0.10},
                        {5.00, +4.70, 0.15},
                        {5.00, +4.90, 0.20},
		        {5.00, +5.00, 0.30},
		};

		BezierCurves<Vec3f, float, 3> bezier;
		bezier.init(control_points);
		auto vecs = bezier.approximate(8);
		return std::vector<Vec3f>(vecs.begin(), vecs.end());
	}

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		auto shape = shapes::RevolveY(
		        36,
		        makeVasePositions(),   // poisitons
		        std::vector<Vec3f>(),  // normals
		        makeVaseTexcoords());  // texcoords

		m_array.initArray(shape, {"position", "normal", "tangent", "texcoord"});

		const auto c_tex_side = 512;
		auto image = images::NewtonFractal(
		        c_tex_side, c_tex_side, Vec3f(0.8, 0.8, 1.0), Vec3f(0.1, 0.0, 0.2), Vec2f(-1, -1),
		        Vec2f(1, 1), images::NewtonFractal::X4Minus1(),
		        [](double x) -> double { return pow(sin(pow(x, 0.5) * math::two_pi()), 4.0); });

		Vec4f border_color = {0.8, 0.8, 1.0, 1.0};

		Attrs tex_attrs = {
		        {"target",     GL_TEXTURE_2D          },
		        {"iformat",    GL_RGB32F              },
		        {"width",      image.width()          },
		        {"height",     image.height()         },
		        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
		        {"mag_filter", GL_LINEAR              },
		        {"wrap_s",     GL_REPEAT              },
		        {"wrap_t",     GL_CLAMP_TO_BORDER     },
		        {"border",     border_color           },
		        {"data",       image.data()           },
		};
		m_texture.init(tex_attrs);
		m_array.u_texture = m_texture.id();
		m_array.u_light_position = {4, 4, -8};

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_BACK;
	}

	void render() override
	{
		auto esec = getSeconds().current();

		m_array.u_viewscreen = math::perspective(viewport(0), 60, 1, 30);
		m_array.u_worldview
		        = Mat4f::orbiting(Vec3f(0.0, 1.5, 0.0), esec, 7.0, 2.5, 11.0, 0, 19, 35, 30, 20);
		m_array.u_eye_position = m_array.u_worldview.unitary_inverse().c[3];
		m_array.u_nodeworld = math::unit().trans(-ez()).rot("Xy", 25.0, -esec / 7.0 * math::two_pi());
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("022_vase");
}  // namespace
}  // namespace spu::oglplus
