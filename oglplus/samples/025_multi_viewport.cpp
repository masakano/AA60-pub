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
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_texcoord;                                                                   \n"
    "out vec3 g_light_dir;                                                                  \n"
    "out vec3 g_light_refl;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       g_normal = mat3(u_nodeworld)* a_normal;                                         \n"
    "       g_texcoord = a_normal;                                                          \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       g_light_dir = u_light_pos-gl_Position.xyz;                                      \n"
    "       g_light_refl = reflect(-g_light_dir, g_normal);                                 \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "#extension GL_ARB_viewport_array : enable                                              \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 12) out;                                         \n"
    "uniform mat4 u_worldscreen[4];                                                         \n"
    "uniform vec3 u_eye_position[4];                                                        \n"
    "in vec3 g_normal[];                                                                    \n"
    "in vec3 g_texcoord[];                                                                  \n"
    "in vec3 g_light_dir[];                                                                 \n"
    "in vec3 g_light_refl[];                                                                \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_texcoord;                                                                   \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_light_refl;                                                                 \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out vec3 f_view_refl;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       for (int vp=0; vp!=4; ++vp)                                                     \n"
    "       {                                                                               \n"
    "               gl_ViewportIndex = vp;                                                  \n"
    "               for (int v=0; v!=3; ++v)                                                \n"
    "               {                                                                       \n"
    "                       f_normal = g_normal[v];                                         \n"
    "                       f_texcoord = g_texcoord[v];                                     \n"
    "                       f_light_dir = g_light_dir[v];                                   \n"
    "                       f_light_refl = g_light_refl[v];                                 \n"
    "                       f_view_dir = u_eye_position[vp] - gl_in[v].gl_Position.xyz;     \n"
    "                       f_view_refl = reflect(-f_view_dir, f_normal);                   \n"
    "                       gl_Position = u_worldscreen[vp] * gl_in[v].gl_Position;         \n"
    "                       EmitVertex();                                                   \n"
    "               }                                                                       \n"
    "               EndPrimitive();                                                         \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform samplerCube u_texture;                                                         \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_texcoord;                                                                    \n"
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
    "       vec3 tex = texture(u_texture, f_texcoord).rgb;                                  \n"
    "       final_color = vec4(                                                             \n"
    "               tex * 0.4 +                                                             \n"
    "               (lt + tex) * 1.5 * max(d, 0.0) +                                        \n"
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

	Vec3f u_eye_position[4];
	Mat4f u_worldscreen[4];

	Mat4f u_nodeworld;
	Vec3f u_light_pos;
	uint32_t u_texture;

	App(const char *name) : SpuPage(name, true, {0.1, 0.05, 0.2, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
                        {"geom", c_geom},
                        {"vert", c_vert}
                };

		Attrs unif_attrs = {
		        {"u_eye_position", &u_eye_position[0]},
		        {"u_worldscreen",  &u_worldscreen[0] },
		        {"u_nodeworld",    &u_nodeworld      },
		        {"u_light_pos",    &u_light_pos      },
		        {"u_texture",      &u_texture        },
		};

		m_array.initShader(shader_attrs, unif_attrs);

		{
			shapes::SpiralSphere spiral_shape(1.0, 0.1, 8, 4, 48);
			m_array.initArray(spiral_shape, {"position", "normal"});

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.cull_face = GL_BACK;
			// renderstate.use();
		}

		{
			const auto c_tex_side = 512;
			auto image = images::NewtonFractal(
			        c_tex_side, c_tex_side, Vec3f(0.8, 0.8, 1.0), Vec3f(0.1, 0.0, 0.2),
			        Vec2f(-0.707, -0.707), Vec2f(0.707, 0.707), images::NewtonFractal::X4Minus1(),
			        [](double x) -> double { return pow(sin(pow(x, 0.5) * math::two_pi()), 4.0); });

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
			        {"data",       (const void*)pix.data()},
			};
			m_texture.init(tex_attrs);
			u_texture = m_texture.id();
		}

		u_light_pos = {3.0, 5.0, 4.0};

		u_eye_position[0] = {2, 0, 0};
		u_eye_position[1] = {0, 2, 0};
		u_eye_position[2] = {0, 0, 2};
	}

	void render() override
	{
		// reshape
		auto viewscreen = math::perspective(viewport(0), 90, 1, 30);

		auto m0 = Mat4f(Vec4f(0, 0, -1, 0), Vec4f(0, 1, 0, 0), Vec4f(1, 0, 0, -3), Vec4f(0, 0, 0, 1));

		auto m1 = Mat4f(Vec4f(1, 0, 0, 0), Vec4f(0, 0, -1, 0), Vec4f(0, 1, 0, -3), Vec4f(0, 0, 0, 1));
		auto m2 = Mat4f(Vec4f(1, 0, 0, 0), Vec4f(0, 1, 0, 0), Vec4f(0, 0, 1, -3), Vec4f(0, 0, 0, 1));

		// matrix

		u_worldscreen[0] = viewscreen * m0.transpose4();
		u_worldscreen[1] = viewscreen * m1.transpose4();
		u_worldscreen[2] = viewscreen * m2.transpose4();

		auto w = viewport(0).sx;
		auto h = viewport(0).sy;

		auto viewport_org = Rectf(0, 0, w, h);

		std::vector<Rectf> viewports = {
		        {0,     0,     w / 2, h / 2},
		        {w / 2, 0,     w / 2, h / 2},
		        {0,     h / 2, w / 2, h / 2},
		        {w / 2, h / 2, w / 2, h / 2},
		};

		// sev viewport
		{
			Attrs canvas_attrs = {
			        {"viewport0", viewports[0]},
			        {"viewport1", viewports[1]},
			        {"viewport2", viewports[2]},
			        {"viewport3", viewports[3]},
			};
			SpuPage::set(canvas_attrs);
			SpuPage::clear();
		}

		// draw
		{
			auto esec = getSeconds().current();
			auto worldview = Mat4f::orbiting(ezero(), esec, 4.5, -1.5, 16, 0, 12, 0, 90, 30);

			u_worldscreen[3] = viewscreen * worldview;
			u_eye_position[3] = u_worldscreen[3].unitary_inverse().c[3];
			u_nodeworld = math::unit().rot("x", -esec / 10.0 * math::two_pi());

			m_array.draw(nullptr);
		}

		// resume (for spu_print)
		{
			Attrs canvas_attrs = {
			        {"viewport0", viewport_org},
			};
			SpuPage::set(canvas_attrs);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("025_multi_viewport");
}  // namespace
}  // namespace spu::oglplus
