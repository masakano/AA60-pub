//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/cloud.hpp>
#include <shapes/sphere.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_light_vert =  {
    "#version 330                                                                           \n"
    "in vec3 a_position;                                                                    \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float s = 0.1;                                                                  \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen*                                                            \n"
    "               u_worldview*                                                            \n"
    "               vec4(a_position*s + u_light_pos, 1.0);                                  \n"
    "}                                                                                      \n"
};

const char *c_light_frag =  {
    "#version 330                                                                           \n"
    "out vec4 frag_light;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       frag_light = vec4(1.0, 1.0, 1.0, 1.0);                                          \n"
    "}                                                                                      \n"
};

const char *c_cloud_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in float u_size;                                                                       \n"
    "uniform int u_sample_count;                                                            \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec4 u_view_z;                                                                 \n"
    "out float g_z_offset;                                                                  \n"
    "out float g_size;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float hp = (u_sample_count-1) * 0.5;                                            \n"
    "       g_z_offset = (gl_InstanceID - hp)/hp;                                           \n"
    "       g_size = u_size;                                                                \n"
    "       gl_Position = vec4(                                                             \n"
    "               a_position.xyz +                                                        \n"
    "               u_view_z.xyz*g_z_offset*u_size*0.5,                                     \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_gloud_geom =  {
    "#version 330                                                                           \n"
    "layout(points) in;                                                                     \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "in float g_z_offset[];                                                                 \n"
    "in float g_size[];                                                                     \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform vec4 u_view_x;                                                                 \n"
    "uniform vec4 u_view_y;                                                                 \n"
    "uniform vec4 u_view_z;                                                                 \n"
    "out vec3 f_texcoord;                                                                   \n"
    "out vec3 f_light_dir;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float zo = g_z_offset[0];                                                       \n"
    "       float s = g_size[0];                                                            \n"
    "       float yo[2] = float[2](-1.0, 1.0);                                              \n"
    "       float xo[2] = float[2](-1.0, 1.0);                                              \n"
    "       for (int j=0;j!=2;++j)                                                          \n"
    "       for (int i=0;i!=2;++i)                                                          \n"
    "       {                                                                               \n"
    "               vec4 v = vec4(                                                          \n"
    "                       gl_in[0].gl_Position.xyz+                                       \n"
    "                       u_view_x.xyz * xo[i] * s * 0.5+                                 \n"
    "                       u_view_y.xyz * yo[j] * s * 0.5,                                 \n"
    "                       1.0                                                             \n"
    "               );                                                                      \n"
    "               gl_Position = u_viewsceen * u_worldview * v;                            \n"
    "               f_light_dir = u_light_pos - v.xyz;                                      \n"
    "               f_texcoord =                                                            \n"
    "                       vec3(0.5, 0.5, 0.5)+                                            \n"
    "                       u_view_x.xyz*(xo[i])*0.707+                                     \n"
    "                       u_view_y.xyz*(yo[j])*0.707+                                     \n"
    "                       u_view_z.xyz*(zo   )*0.707;                                     \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_cloud_frag =  {
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
    "       float a = 0.4 * d;                                                              \n"
    "       float i = mix(0.2, 1.0, o);                                                     \n"
    "       final_color = vec4(i, i, i, a);                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	const float c_quality = 0.5;
	const uint32_t c_samples = 25 + c_quality * c_quality * 100;

	Vec3f u_light_pos;
	Mat4f u_worldview;
	Mat4f u_viewsceen;
	Vec4f u_view_x, u_view_y, u_view_z;
	int32_t u_sample_count;
	uint32_t u_texture;

	shapes::Array m_lightArray;

	SpuShader m_cloudShader;
	SpuArray m_cloudArray;

	std::vector<SpuTexture> m_textures;

	const std::vector<Vec3f> m_positions = {
	        {-1.4, -0.3, -0.7},
	        {-1.6, +0.4, +0.7},
	        {+0.6, -0.1, +0.1},
	};

	const std::vector<float> m_sizes = {1.0, 1.2, 1.5};

	std::vector<Vec3f> m_makeLightPathCps
	        = {Vec3f(-3, -3, -3),       Vec3f(+0.0, +0.0, +0.0), Vec3f(+3.0, +3.0, +3.0),
	           Vec3f(+3.0, -3.0, -3.0), Vec3f(+0.0, +0.0, +0.0), Vec3f(-3.0, +3.0, +3.0)};

	CubicBezierLoop<Vec3f, double> m_lightPath;

	App(const char *name) : SpuPage(name, true, {0.0, 0.1, 0.2, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_lightPath.init(m_makeLightPathCps);
		m_textures.resize(m_positions.size());

		assert(m_positions.size() == m_sizes.size());
		srand(123456);

		{
			Attrs shader_attrs = {
			        {"frag", c_light_frag},
			        {"vert", c_light_vert},
			};

			Attrs unif_attrs = {
			        {"u_light_pos", &u_light_pos},
			        {"u_worldview", &u_worldview},
			        {"u_viewsceen", &u_viewsceen},
			};
			shapes::Sphere make_sphere;

			m_lightArray.initShader(shader_attrs, unif_attrs);
			m_lightArray.initArray(make_sphere, {"position"});
		}
		{
			Attrs shader_attrs = {
			        {"frag", c_cloud_frag},
			        {"geom", c_gloud_geom},
			        {"vert", c_cloud_vert},
			};

			Attrs unif_attrs = {
			        {"u_light_pos",    &u_light_pos   },
			        {"u_worldview",    &u_worldview   },
			        {"u_viewsceen",    &u_viewsceen   },
			        {"u_view_x",       &u_view_x      },
			        {"u_view_y",       &u_view_y      },
			        {"u_view_z",       &u_view_z      },
			        {"u_sample_count", &u_sample_count},
			        {"u_texture",      &u_texture     },
			};
			shapes::loadShader(m_cloudShader, shader_attrs, unif_attrs);

			Attrs vert_attrs = {
			        {"shader_id",    m_cloudShader.id()},
			        {"a.a_position", 3                 },
			        {"nelem",        m_positions.size()},
			        {"data",         m_positions.data()},
			};
			m_cloudArray.init(vert_attrs);

			Attrs aux_attrs = {
			        {"a.u_size", 1             },
			        {"nelem",    m_sizes.size()},
			        {"data",     m_sizes.data()},
			};
			m_cloudArray.aux(aux_attrs, 1);
		}

		// set the number of samples
		u_sample_count = c_samples;

		for (size_t i = 0, n = m_positions.size(); i != n; ++i) {
			auto image = images::Cloud(128, 128, 128, Vec3f(0.1, -0.5, 0.3), 0.5);

			Attrs attrs = {
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
			        {"data",       image.data()           },
			};
			m_textures[i].init(attrs);
		}

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

		u_viewsceen = math::perspective(viewport(0), 65, 1, 40);
		u_light_pos = m_lightPath.position(esec * 0.05);
		u_worldview = Mat4f::orbiting(ezero(), esec, 4.5, 0, 0, 0, 0, 0, 80, 20);

		m_lightArray.draw(nullptr);
		u_view_x = u_worldview.transpose4().c[0];
		u_view_y = u_worldview.transpose4().c[1];
		u_view_z = u_worldview.transpose4().c[2];

		// TODO(suzu): need depth sort
		for (size_t i = 0, n = m_positions.size(); i != n; ++i) {
			u_texture = m_textures[i].id();
			m_cloudShader.use();
			m_cloudArray.draw(GL_POINTS, i, 1, c_samples);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("026_clouds");
}  // namespace
}  // namespace spu::oglplus
