//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/cloud.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "uniform mat4 u_worldview;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_worldview * a_position;                                         \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "layout(points) in;                                                                     \n"
    "layout(triangle_strip, max_vertices = 100) out;                                        \n"
    "const int p = 25;                                                                      \n"
    "const float hp = (p-1)*0.5;                                                            \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "out vec3 f_texcoord;                                                                   \n"
    "out vec3 f_light_dir;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float s = 0.6;                                                                  \n"
    "       float yo[2] = float[2](-1.0, 1.0);                                              \n"
    "       float xo[2] = float[2](-1.0, 1.0);                                              \n"
    "       vec3 cx = vec3(                                                                 \n"
    "               u_worldview[0][0],                                                      \n"
    "               u_worldview[1][0],                                                      \n"
    "               u_worldview[2][0]                                                       \n"
    "       );                                                                              \n"
    "       vec3 cy = vec3(                                                                 \n"
    "               u_worldview[0][1],                                                      \n"
    "               u_worldview[1][1],                                                      \n"
    "               u_worldview[2][1]                                                       \n"
    "       );                                                                              \n"
    "       vec3 cz = vec3(                                                                 \n"
    "               u_worldview[0][2],                                                      \n"
    "               u_worldview[1][2],                                                      \n"
    "               u_worldview[2][2]                                                       \n"
    "       );                                                                              \n"
    "       for (int k=0;k!=p;++k)                                                          \n"
    "       {                                                                               \n"
    "               for (int j=0;j!=2;++j)                                                  \n"
    "               for (int i=0;i!=2;++i)                                                  \n"
    "               {                                                                       \n"
    "                       float zo = ((k - hp) / hp);                                     \n"
    "                       float xoffs = xo[i]*s;                                          \n"
    "                       float yoffs = yo[j]*s;                                          \n"
    "                       float zoffs = zo   *s;                                          \n"
    "                       vec4 v = vec4(                                                  \n"
    "                               gl_in[0].gl_Position.x+xoffs,                           \n"
    "                               gl_in[0].gl_Position.y+yoffs,                           \n"
    "                               gl_in[0].gl_Position.z+zoffs,                           \n"
    "                               1.0                                                     \n"
    "                       );                                                              \n"
    "                       gl_Position = u_viewsceen * v;                                  \n"
    "                       f_light_dir = u_light_pos - v.xyz;                              \n"
    "                       f_texcoord =                                                    \n"
    "                               vec3(0.5, 0.5, 0.5)+                                    \n"
    "                               cx*(xo[i])*0.707+                                       \n"
    "                               cy*(yo[j])*0.707+                                       \n"
    "                               cz*(zo   )*0.707;                                       \n"
    "                       EmitVertex();                                                   \n"
    "               }                                                                       \n"
    "               EndPrimitive();                                                         \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler3D u_texture;                                                           \n"
    "in vec3 f_texcoord;                                                                    \n"
    "in vec3 f_light_dir;                                                                   \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float d = texture(u_texture, f_texcoord).r;                                     \n"
    "       float o = 1.0;                                                                  \n"
    "       float s = 2.0/128.0;                                                            \n"
    "       float r = s * 8.0;                                                              \n"
    "       vec3 sample_offs = normalize(f_light_dir) * s;                                  \n"
    "       vec3 sample_pos = f_texcoord;                                                   \n"
    "       if (d > 0.01) while (o > 0.0)                                                   \n"
    "       {                                                                               \n"
    "               if (sample_pos.x<0.0 || sample_pos.x>1.0)                               \n"
    "                       break;                                                          \n"
    "               if (sample_pos.y<0.0 || sample_pos.y>1.0)                               \n"
    "                       break;                                                          \n"
    "               if (sample_pos.z<0.0 || sample_pos.z>1.0)                               \n"
    "                       break;                                                          \n"
    "               o -= texture(u_texture, sample_pos).r*r;                                \n"
    "               sample_pos += sample_offs;                                              \n"
    "       }                                                                               \n"
    "       float a = 0.2 * d;                                                              \n"
    "       float i = mix(0.4, 1.0, o);                                                     \n"
    "       final_color = vec4(i, i, i, a);                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;
	SpuTexture m_texture;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Vec3f u_light_pos;
	uint32_t u_texture;

	App(const char *name) : SpuPage(name, true, {0.2, 0.3, 0.4, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		        {"geom", c_geom},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen", &u_viewsceen},
		        {"u_worldview", &u_worldview},
		        {"u_light_pos", &u_light_pos},
		        {"u_texture",   &u_texture  },
		};

		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		u_light_pos = {10.0, 1.0, 5.0};

		float positions[3] = {0.5, 0.1, 0.2};
		Attrs array_attrs = {
		        {"shader_id",    m_shader.id()},
		        {"data",         &positions[0]},
		        {"nelem",        1            },
		        {"a.a_position", 3            },
		};
		m_array.init(array_attrs);

		images::Image image = images::Cloud(128, 128, 128);
		Vec4f border_color = {0, 0, 0, 0};

		Attrs tex_attrs = {
		        {"target",     GL_TEXTURE_3D          },
		        {"iformat",    GL_R8                  },
		        {"width",      image.width()          },
		        {"height",     image.height()         },
		        {"depth",      image.depth()          },

		        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
		        {"mag_filter", GL_LINEAR              },
		        {"wrap_s",     GL_CLAMP_TO_BORDER     },
		        {"wrap_t",     GL_CLAMP_TO_BORDER     },
		        {"wrap_r",     GL_CLAMP_TO_BORDER     },
		        {"border",     border_color           },
		        {"data",       image.data()           },
		};

		m_texture.init(tex_attrs);
		u_texture = m_texture.id();

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.blend = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 80, 1, 80);
		u_worldview = Mat4f::orbiting(ezero(), esec, 3.5, 1.0, 6.0, 0, 5.0, 0, 80, 20);

		m_shader.use();
		m_array.draw(GL_POINTS);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("021_cloud");
}  // namespace
}  // namespace spu::oglplus
