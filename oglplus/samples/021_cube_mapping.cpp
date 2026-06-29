//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/newton.hpp>
#include <shapes/spiral_sphere.hpp>

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
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_light_refl;                                                                 \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out vec3 f_view_refl;                                                                  \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_normal = mat3(u_nodeworld)* a_normal;                                         \n"
    "       f_light_dir = u_light_pos - gl_Position.xyz;                                    \n"
    "       f_light_refl = reflect(                                                         \n"
    "               -normalize(f_light_dir),                                                \n"
    "               normalize(f_normal)                                                     \n"
    "       );                                                                              \n"
    "       f_view_dir = (                                                                  \n"
    "               vec4(0.0, 0.0, 1.0, 1.0)*                                               \n"
    "               u_worldview                                                             \n"
    "       ).xyz;                                                                          \n"
    "       f_view_refl = reflect(                                                          \n"
    "               normalize(f_view_dir),                                                  \n"
    "               normalize(f_normal)                                                     \n"
    "       );                                                                              \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform samplerCube u_texture;                                                         \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_light_refl;                                                                  \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec3 f_view_refl;                                                                   \n"
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
    "       vec3 lt = vec3(1.0, 1.0, 1.0);                                                  \n"
    "       vec3 env = texture(u_texture, f_view_refl).rgb;                                 \n"
    "       final_color = vec4(                                                             \n"
    "               env * 0.4 +                                                             \n"
    "               (lt + env) * 1.5 * max(d, 0.0) +                                        \n"
    "               lt * pow(max(s, 0.0), 64),                                              \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;
	SpuTexture m_texture;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;
	uint32_t u_texture;

	App(const char *name) : SpuPage(name, true, {0.2, 0.05, 0.1, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen", &u_viewsceen},
                        {"u_worldview", &u_worldview},
		        {"u_nodeworld", &u_nodeworld},
                        {"u_light_pos", &u_light_pos},
		        {"u_texture",   &u_texture  },
		};
		m_array.initShader(shader_attrs, unif_attrs);
		u_light_pos = {3.0, 5.0, 4.0};

		m_array.initArray(shapes::SpiralSphere(), {"position", "normal"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_BACK;

		const auto c_tex_side = 256;
		images::Image image = images::NewtonFractal(
		        c_tex_side, c_tex_side, Vec3f(0.3, 0.1, 0.2), Vec3f(1.0, 0.8, 0.9), Vec2f(-1, -1),
		        Vec2f(1, 1), images::NewtonFractal::X4Minus1(), images::NewtonFractal::DefaultMixer());

		auto unit = image.dataSize();
		std::vector<uint8_t> pix(unit * 6);

		for (auto i = 0; i < 6; i++) {
			memcpy(&pix[unit * i], image.data(), image.dataSize());
		}

		Attrs tex_attrs = {
		        {"target",     GL_TEXTURE_CUBE_MAP     },
                        {"iformat",    GL_RGB32F               },
		        {"width",      image.width()           },
                        {"height",     image.height()          },
		        {"min_filter", GL_LINEAR               },
                        {"mag_filter", GL_LINEAR               },
		        {"wrap_s",     GL_CLAMP_TO_EDGE        },
                        {"wrap_t",     GL_CLAMP_TO_EDGE        },
		        {"data",       (const void *)pix.data()},
		};
		m_texture.init(tex_attrs);
		u_texture = m_texture.id();
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 100);
		u_worldview = Mat4f::orbiting(ezero(), esec, 4.5, -2.0, 16.0, 0, 12.0, 0, 90, 30);
		u_nodeworld = Mat4f(Quatf(esec / 10.0 * math::two_pi(), Vec3f(1, 1, 1)));

		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("021_cube_mapping");
}  // namespace
}  // namespace spu::oglplus
