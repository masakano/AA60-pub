//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/sphere_bmap.hpp>
#include <shapes/cube.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec3 a_tangent;                                                                     \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_eye;                                                                        \n"
    "out vec3 f_light;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out vec3 f_tangent_view;                                                               \n"
    "out mat3 f_normal_matrix;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec4 eye_pos = u_worldview * u_nodeworld * a_position;                          \n"
    "       f_eye = eye_pos.xyz;                                                            \n"
    "       vec3 frag_tangent = (                                                           \n"
    "               u_worldview *                                                           \n"
    "               u_nodeworld *                                                           \n"
    "               vec4(a_tangent, 0.0)                                                    \n"
    "       ).xyz;                                                                          \n"
    "       f_normal = (                                                                    \n"
    "               u_worldview *                                                           \n"
    "               u_nodeworld *                                                           \n"
    "               vec4(a_normal, 0.0)                                                     \n"
    "       ).xyz;                                                                          \n"
    "       f_light = (                                                                     \n"
    "               u_worldview *                                                           \n"
    "               vec4(u_light_pos-f_eye, 1.0)                                            \n"
    "       ).xyz;                                                                          \n"
    "       f_normal_matrix = mat3(                                                         \n"
    "               frag_tangent,                                                           \n"
    "               cross(f_normal, frag_tangent),                                          \n"
    "               f_normal                                                                \n"
    "       );                                                                              \n"
    "       f_tangent_view = vec3(                                                          \n"
    "               dot(f_normal_matrix[0], f_eye),                                         \n"
    "               dot(f_normal_matrix[1], f_eye),                                         \n"
    "               dot(f_normal_matrix[2], f_eye)                                          \n"
    "       );                                                                              \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "       gl_Position = u_viewsceen *                                                     \n"
    "               eye_pos;                                                                \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_texture;                                                           \n"
    "uniform int u_texture_width;                                                           \n"
    "uniform int u_texture_height;                                                          \n"
    "float depth_mult = 0.1;                                                                \n"
    "in vec3 f_eye;                                                                         \n"
    "in vec3 f_light;                                                                       \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec2 f_texcoord;                                                                    \n"
    "in vec3 f_tangent_view;                                                                \n"
    "in mat3 f_normal_matrix;                                                               \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 view_tangent = normalize(f_tangent_view);                                  \n"
    "       float perp = -dot(normalize(f_eye), f_normal);                                  \n"
    "       float sample_interval = 1.0/length(                                             \n"
    "               vec2(u_texture_width, u_texture_height)                                 \n"
    "       );                                                                              \n"
    "       vec3 sample_step = view_tangent*sample_interval;                                \n"
    "       float prev_d = 0.0;                                                             \n"
    "       float depth = texture(u_texture, f_texcoord).w;                                 \n"
    "       float max_offs = min((depth*depth_mult)/(-view_tangent.z), 1.0);                \n"
    "       vec3 view_offs = vec3(0.0, 0.0, 0.001);\n" // nv bug?
    "       vec2 offs_tex_c = f_texcoord + view_offs.xy;                                    \n"
    "       while (length(view_offs) < max_offs)                                            \n"
    "       {                                                                               \n"
    "               if (offs_tex_c.x <= 0.0 || offs_tex_c.x >= 1.0)                         \n"
    "                       break;                                                          \n"
    "               if (offs_tex_c.y <= 0.0 || offs_tex_c.y >= 1.0)                         \n"
    "                       break;                                                          \n"
    "               if (depth*depth_mult*perp <= -view_offs.z)                              \n"
    "                       break;                                                          \n"
    "               view_offs += sample_step;                                               \n"
    "               offs_tex_c = f_texcoord + view_offs.xy;                                 \n"
    "               prev_d = depth;                                                         \n"
    "               depth = texture(u_texture, offs_tex_c).w;                               \n"
    "       }                                                                               \n"
    "       offs_tex_c = vec2(                                                              \n"
    "               clamp(offs_tex_c.x, 0.0, 1.0),                                          \n"
    "               clamp(offs_tex_c.y, 0.0, 1.0)                                           \n"
    "       );                                                                              \n"
    "       float b = (                                                                     \n"
    "               1 +                                                                     \n"
    "               int(offs_tex_c.x*16) % 2+                                               \n"
    "               int(offs_tex_c.y*16) % 2                                                \n"
    "       ) % 2;                                                                          \n"
    "       vec3 c = vec3(b, b, b);                                                         \n"
    "       vec3 n = texture(u_texture, offs_tex_c).xyz;                                    \n"
    "       vec3 final_normal = f_normal_matrix * n;                                        \n"
    "       float l = length(f_light);                                                      \n"
    "       float d = (l > 0.0) ? dot(                                                      \n"
    "               normalize(f_light),                                                     \n"
    "               final_normal                                                            \n"
    "       ) / l : 0.0;                                                                    \n"
    "       float i = 0.1 + 2.5*max(d, 0.0);                                                \n"
    "       final_color = vec4(c*i, 1.0);                                                   \n"
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
	int32_t u_texture_width;
	int32_t u_texture_height;

	App(const char *name) : SpuPage(name, true, {0.1, 0.1, 0.1, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen",      &u_viewsceen     },
		        {"u_worldview",      &u_worldview     },
		        {"u_nodeworld",      &u_nodeworld     },
		        {"u_light_pos",      &u_light_pos     },
		        {"u_texture_width",  &u_texture_width },
		        {"u_texture_height", &u_texture_height},
		        {"u_texture",        &u_texture       },
		};
		m_array.initShader(shader_attrs, unif_attrs);

		{
			m_array.initArray(shapes::Cube(), {"position", "normal", "tangent", "texcoord"});

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.cull_face = GL_BACK;
		}

		{
			auto img = images::SphereBumpMap(512, 512, 2, 2);
			u_texture_width = img.width();
			u_texture_height = img.height();

			Attrs tex_attrs = {
			        {"target",     GL_TEXTURE_2D          },
			        {"iformat",    GL_RGBA32F             },
			        {"width",      img.width()            },
			        {"height",     img.height()           },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_REPEAT              },
			        {"wrap_t",     GL_REPEAT              },
			        {"data",       img.data()             },
			};
			m_texture.init(tex_attrs);
			u_texture = m_texture.id();
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto light_azimuth = esec * -0.5 * math::two_pi();

		u_viewsceen = math::perspective(viewport(0), 54, 1, 10);
		u_light_pos = Vec3f(-cos(light_azimuth), 1.0, -sin(light_azimuth)) * 2.0;
		u_worldview = Mat4f::orbiting(ezero(), esec, 3.0, 0, 0, -45, 0, 0, 70, 30);
		u_nodeworld = Mat4f(Quatf(-esec * 0.05 * math::two_pi(), Vec3f(1, 1, 1)));

		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("022_parallax_map");
}  // namespace
}  // namespace spu::oglplus
