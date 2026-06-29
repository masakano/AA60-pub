//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>
#include <shapes/subdiv_sphere.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_texcoord;                                                                   \n"
    "out vec3 g_light_dir;                                                                  \n"
    "out vec3 g_view_dir;                                                                   \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 normal = a_position.xyz;                                                   \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       g_normal = mat3(u_nodeworld)* normal;                                           \n"
    "       g_texcoord = normal;                                                            \n"
    "       g_light_dir = u_light_pos - gl_Position.xyz;                                    \n"
    "       g_view_dir = (vec4(0.0, 0.0, 1.0, 1.0)*u_worldview).xyz;                        \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "layout (triangles) in;                                                                 \n"
    "layout (triangle_strip, max_vertices = 3) out;                                         \n"
    "in vec3 g_normal[3];                                                                   \n"
    "in vec3 g_texcoord[3];                                                                 \n"
    "in vec3 g_light_dir[3];                                                                \n"
    "in vec3 g_view_dir[3];                                                                 \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_texcoord;                                                                   \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_light_refll;                                                                \n"
    "out vec3 f_view_dir;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 face_normal = 0.333333*(                                                   \n"
    "               g_normal[0]+                                                            \n"
    "               g_normal[1]+                                                            \n"
    "               g_normal[2]                                                             \n"
    "       );                                                                              \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "       {                                                                               \n"
    "               gl_Position = gl_in[v].gl_Position;                                     \n"
    "               f_normal = 0.5*(g_normal[v]+face_normal);                               \n"
    "               f_texcoord = g_texcoord[v];                                             \n"
    "               f_light_dir = g_light_dir[v];                                           \n"
    "               f_light_refll = reflect(                                                \n"
    "                       -normalize(f_light_dir),                                        \n"
    "                       normalize(face_normal)                                          \n"
    "               );                                                                      \n"
    "               f_view_dir = g_view_dir[v];                                             \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform samplerCube u_texture;                                                         \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_texcoord;                                                                    \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_light_refll;                                                                 \n"
    "in vec3 f_view_dir;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 lt = vec3(1.0, 1.0, 1.0);                                                  \n"
    "       vec3 tex = texture(u_texture, f_texcoord).rgb;                                  \n"
    "       float d = dot(                                                                  \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light_dir)                                                  \n"
    "       );                                                                              \n"
    "       float s = dot(                                                                  \n"
    "               normalize(f_light_refll),                                               \n"
    "               normalize(f_view_dir)                                                   \n"
    "       );                                                                              \n"
    "       float b = 1.0-sqrt(max(dot(                                                     \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_view_dir)                                                   \n"
    "       ), 0.0));                                                                       \n"
    "       float ea = clamp(tex.b*(-d+0.2), 0.0, 1.0);                                     \n"
    "       float sr = 1.0-tex.b*0.8;                                                       \n"

    "       final_color =                                                                   \n"
    "               tex * (0.3*ea + 0.6*b + 0.8*max(d, 0.0)) +                              \n"
    "               (tex+lt) * 0.8*sr*pow(clamp(s+0.05, 0.0, 1.0), 32);                     \n"
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

	App(const char *name) : SpuPage(name, true, {0.05, 0.2, 0.1, 0.0}) {}

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
		        {"u_nodeworld", &u_nodeworld},
                        {"u_light_pos", &u_light_pos},
		        {"u_texture",   &u_texture  },
		};
		m_array.initShader(shader_attrs, unif_attrs);
		m_array.initArray(shapes::SimpleSubdivSphere(4), {"position"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_BACK;

		const char *tex_name[6] = {
		        "cube_0_right",  "cube_1_left",  "cube_2_top",
		        "cube_3_bottom", "cube_4_front", "cube_5_back",
		};

		int32_t cube_target[] = {
		        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};

		std::vector<uint8_t> pix;
		auto unit = size_t(0);
		auto width = 0u;
		auto height = 0u;
		for (auto i = 0; i < 6; i++) {
			images::Image image = images::LoadTexture(tex_name[i], false, true);
			if (i == 0) {
				unit = image.dataSize();
				width = image.width();
				height = image.height();
				pix.resize(unit * 6);
			}
			assert(image.dataSize() == unit);
			memcpy(&pix[unit * i], image.data(), unit);
		}

		Attrs tex_attrs = {
		        {"target",      GL_TEXTURE_CUBE_MAP},
		        {"iformat",     GL_RGBA8           },
		        {"width",       width              },
		        {"height",      height             },
		        {"data",        pix.data()         },
		        {"cube_target", cube_target        },
		        {"min_filter",  GL_LINEAR          },
		        {"mag_filter",  GL_LINEAR          },
		        {"wrap_s",      GL_CLAMP_TO_EDGE   },
		        {"wrap_t",      GL_CLAMP_TO_EDGE   },
		        {"auto_mipmap", 0                  },
		};

		m_texture.init(tex_attrs);
		u_texture = m_texture.id();

		u_light_pos = {3.0, 5.0, 4.0};
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 100);
		u_worldview = Mat4f::orbiting(ezero(), esec, 4.5, -2, 16, 0, 12, 0, 90, 30);
		u_nodeworld = Mat4f(Quatf(esec / 10.0 * math::two_pi(), Vec3f(1, 1, 1)));
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("020_icosphere");
}  // namespace
}  // namespace spu::oglplus
