//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_volume_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "out float g_z_offset;                                                                  \n"
    "uniform int u_sample_count;                                                            \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec3 u_view_z;                                                                 \n"
    "uniform float u_size;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float hp = (u_sample_count-1) * 0.5;                                            \n"
    "       g_z_offset = (gl_InstanceID - hp)/hp;                                           \n"
    "       gl_Position = vec4(                                                             \n"
    "               a_position.xyz +                                                        \n"
    "               u_view_z*g_z_offset*u_size*0.5,                                         \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_volume_geom =  {
    "#version 330                                                                           \n"
    "layout(points) in;                                                                     \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldtexc;                                                              \n"
    "uniform vec3 u_view_x;                                                                 \n"
    "uniform vec3 u_view_y;                                                                 \n"
    "uniform float u_size;                                                                  \n"
    "in float g_z_offset[];                                                                 \n"
    "out vec4 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float zo = g_z_offset[0];                                                       \n"
    "       float yo[2] = float[2](-1.0, 1.0);                                              \n"
    "       float xo[2] = float[2](-1.0, 1.0);                                              \n"
    "       for (int j=0;j!=2;++j)                                                          \n"
    "       for (int i=0;i!=2;++i)                                                          \n"
    "       {                                                                               \n"
    "               vec4 v = vec4(                                                          \n"
    "                       gl_in[0].gl_Position.xyz+                                       \n"
    "                       u_view_x * xo[i] * u_size+                                      \n"
    "                       u_view_y * yo[j] * u_size,                                      \n"
    "                       1.0                                                             \n"
    "               );                                                                      \n"
    "               f_texcoord =                                                            \n"
    "                       u_worldtexc * v;                                                \n"
    "               gl_Position =                                                           \n"
    "                       u_viewsceen *                                                   \n"
    "                       u_worldview * v;                                                \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_volume_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_texture;                                                           \n"
    "uniform int u_sample_count;                                                            \n"
    "in vec4 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec2 coord = f_texcoord.st/f_texcoord.q;                                        \n"
    "       float depth = f_texcoord.z;                                                     \n"
    "       if (depth < 0.0) discard;                                                       \n"
    "       vec4 t  = texture(u_texture, coord*0.5 + 0.5);                                  \n"
    "       if (t.a == 0.0) discard;                                                        \n"
    "       float alpha = 10.0*(1.0-t.a)/u_sample_count;                                    \n"
    "       alpha *= (t.r+t.g+t.b)*0.3333;                                                  \n"
    "       alpha /= sqrt(depth);                                                           \n"
    "       final_color = vec4(t.rgb, alpha);                                               \n"
    "}                                                                                      \n"
};

const char *c_plane_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldtexc;                                                              \n"
    "out vec2 f_checker;                                                                    \n"
    "out vec4 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               a_position;                                                             \n"
    "       f_texcoord =                                                                    \n"
    "               u_worldtexc *                                                           \n"
    "               a_position;                                                             \n"
    "       f_checker = a_position.xz;                                                      \n"
    "}                                                                                      \n"
};

const char *c_plane_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_texture;                                                           \n"
    "in vec4 f_texcoord;                                                                    \n"
    "in vec2 f_checker;                                                                     \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec2 coord = f_texcoord.st/f_texcoord.q;                                        \n"
    "       vec4 t  = texture(u_texture, coord*0.5 + 0.5);                                  \n"
    "       float i = (                                                                     \n"
    "               1 +                                                                     \n"
    "               int(f_checker.x+10) % 2+                                                \n"
    "               int(f_checker.y+10) % 2                                                 \n"
    "       ) % 2;                                                                          \n"
    "       vec3 color = vec3(0.1, 0.1, 0.1);                                               \n"
    "       color += t.rgb * (1.0 - t.a);                                                   \n"
    "       color *= mix(                                                                   \n"
    "               vec3(0.9, 0.9, 1.0),                                                    \n"
    "               vec3(0.4, 0.4, 0.9),                                                    \n"
    "               i                                                                       \n"
    "       );                                                                              \n"
    "       final_color = vec4(color, 1.0);                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	const uint32_t c_samples = 150;

	SpuShader m_volumeShader;
	SpuArray m_volumeArray;

	SpuShader m_planeShader;
	SpuArray m_planeArray;
	SpuTexture m_texture;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_worldtexc;
	Vec3f u_light_pos;
	Vec3f u_view_x, u_view_y, u_view_z;
	float u_size;
	uint32_t u_texture;
	int32_t u_sample_count;

	App(const char *name) : SpuPage(name, true, {0.0, 0.05, 0.1, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			Attrs shader_attrs = {
			        {"vert", c_volume_vert},
			        {"geom", c_volume_geom},
			        {"frag", c_volume_frag},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen",    &u_viewsceen   },
			        {"u_worldview",    &u_worldview   },
			        {"u_worldtexc",    &u_worldtexc   },
			        {"u_light_pos",    &u_light_pos   },
			        {"u_view_x",       &u_view_x      },
			        {"u_view_y",       &u_view_y      },
			        {"u_view_z",       &u_view_z      },
			        {"u_size",         &u_size        },
			        {"u_sample_count", &u_sample_count},
			};

			shapes::loadShader(m_volumeShader, shader_attrs, unif_attrs);

			u_light_pos = {2, 4, -3};
			u_sample_count = c_samples;
			u_size = length(u_light_pos);

			auto texc_viewport = Rectf(0, 0, 1, 1);
			u_worldtexc = math::perspective(texc_viewport, 30, 0.3, 20)
			            * math::lookat(u_light_pos, ezero(), ey());
		}

		{
			float position[3] = {0.0, 0.0, 0.0};

			Attrs attrs = {
			        {"shader_id",    m_volumeShader.id()},
			        {"data",         &position[0]       },
			        {"nelem",        1                  },
			        {"a.a_position", 3                  },
			};
			m_volumeArray.init(attrs);
		}

		{
			Attrs shader_attrs = {
			        {"frag", c_plane_frag},
			        {"vert", c_plane_vert},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen},
			        {"u_worldview", &u_worldview},
			        {"u_worldtexc", &u_worldtexc},
			        {"u_texture",   &u_texture  },
			};
			shapes::loadShader(m_planeShader, shader_attrs, unif_attrs);
		}

		{
			float data[4 * 3]
			        = {-9.0, -4.0, 9.0, -9.0, -4.0, -9.0, 9.0, -4.0, 9.0, 9.0, -4.0, -9.0};

			Attrs attrs = {
			        {"shader_id",    m_planeShader.id()},
			        {"data",         &data[0]          },
			        {"nelem",        4                 },
			        {"a.a_position", 3                 },
			};
			m_planeArray.init(attrs);
		}

		{
			auto image = images::LoadTexture("flower_glass");

			Vec4f border_color = {0, 0, 0, 0};

			Attrs tex_attrs = {
			        {"target",     GL_TEXTURE_2D          },
			        {"iformat",    GL_RGBA8               },
			        {"width",      image.width()          },
			        {"height",     image.height()         },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_CLAMP_TO_BORDER     },
			        {"wrap_t",     GL_CLAMP_TO_BORDER     },
			        {"border",     border_color           },
			        {"data",       image.data()           },
			};
			m_texture.init(tex_attrs);
			u_texture = m_texture.id();
		}

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.blend = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE,
		        GL_SRC_ALPHA,
		        GL_ONE,
		};
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 40);
		u_worldview = Mat4f::orbiting(Vec3f(0, 3, 0), esec, 8, 0, 0, 0, 12, 0, 60, 20);
		m_planeShader.use();
		m_planeArray.draw(GL_TRIANGLE_STRIP, 0, 4);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.blend = true;
		renderstate.use();

		u_view_x = u_worldview.transpose4().c[0];
		u_view_y = u_worldview.transpose4().c[1];
		u_view_z = u_worldview.transpose4().c[2];

		m_volumeShader.use();
		m_volumeArray.draw(GL_POINTS, 0, 1, c_samples);

		renderstate.flags.blend = false;
		renderstate.use();
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("022_volumetric_light");
}  // namespace
}  // namespace spu::oglplus
