//
// GridArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/squares.hpp>
#include <shapes/tetrahedrons.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_grid_offset;                                                            \n"
    "uniform float u_time;                                                                  \n"
    "in vec4 a_position;                                                                    \n"
    "out vec3 g_normal;                                                                     \n"
    "out float g_value;                                                                     \n"
    "flat out int g_sign;                                                                   \n"
    "const vec4 source[4] = vec4[4](                                                        \n"
    "       vec4(-3.3, 0.1, 1.9, 0.21),                                                     \n"
    "       vec4(-3.9, 0.2,-2.1, 0.15),                                                     \n"
    "       vec4(-3.3, 0.2, 3.9, 0.17),                                                     \n"
    "       vec4(-2.6, 0.3,-3.9, 0.11)                                                      \n"
    ");                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position + vec4(u_grid_offset, 0.0);                            \n"
    "       g_value = -gl_Position.y;                                                       \n"
    "       g_normal = vec3(0.0, 1.0, 0.0);                                                 \n"
    "       for (int s=0; s!=4; ++s)                                                        \n"
    "       {                                                                               \n"
    "               float x = gl_Position.x - source[s].x;                                  \n"
    "               float z = gl_Position.z - source[s].z;                                  \n"
    "               float a = source[s].y;                                                  \n"
    "               float w = source[s].w*8.0;                                              \n"
    "               float r = x*x + z*z;                                                    \n"
    "               float t = r + s - (u_time*1.9 - 0.2*s);                                 \n"
    "               float g = w * t;                                                        \n"
    "               float d = a*exp((-r-1.0)*0.17)*(w*cos(g)-sin(g));                       \n"
    "               g_value += sin(g)*exp((-r-1.0)*0.17)*a;                                 \n"
    "               g_normal += vec3(x*d, 0.0, z*d);                                        \n"
    "       }                                                                               \n"
    "       g_sign = g_value < 0.0 ? 0 : 1;                                                 \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles_adjacency) in;                                                        \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "uniform isampler1D u_config_texture;                                                   \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "in vec3 g_normal[];                                                                    \n"
    "in float g_value[];                                                                    \n"
    "flat in int g_sign[];                                                                  \n"
    "out vec3 f_normal, f_light_dir, f_view_dir;                                            \n"
    "void make_vertex(int i1, int i2)                                                       \n"
    "{                                                                                      \n"
    "       float t = g_value[i1]/(g_value[i1] - g_value[i2]);                              \n"
    "       gl_Position = mix(                                                              \n"
    "               gl_in[i1].gl_Position,                                                  \n"
    "               gl_in[i2].gl_Position,                                                  \n"
    "               t                                                                       \n"
    "       );                                                                              \n"
    "       f_normal = mix(                                                                 \n"
    "               g_normal[i1],                                                           \n"
    "               g_normal[i2],                                                           \n"
    "               t                                                                       \n"
    "       );                                                                              \n"
    "       f_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       f_view_dir = u_eye_position - gl_Position.xyz;                                  \n"
    "       gl_Position = u_worldview * gl_Position;                                        \n"
    "       EmitVertex();                                                                   \n"
    "}                                                                                      \n"
    "void make_triangle(const ivec4 v1, const ivec4 v2)                                     \n"
    "{                                                                                      \n"
    "       make_vertex(v1.x, v2.x);                                                        \n"
    "       make_vertex(v1.y, v2.y);                                                        \n"
    "       make_vertex(v1.z, v2.z);                                                        \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
    "void make_quad(const ivec4 v1, const ivec4 v2)                                         \n"
    "{                                                                                      \n"
    "       make_vertex(v1.x, v2.x);                                                        \n"
    "       make_vertex(v1.y, v2.y);                                                        \n"
    "       make_vertex(v1.z, v2.z);                                                        \n"
    "       make_vertex(v1.w, v2.w);                                                        \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
    "void process_tetrahedron(int a, int b, int c, int d)                                   \n"
    "{                                                                                      \n"
    "       ivec4 i = ivec4(g_sign[a],g_sign[b],g_sign[c],g_sign[d]);                       \n"
    "       int si = int(dot(i, ivec4(1, 1, 1, 1))) % 4;                                    \n"
    "       if (si != 0)                                                                    \n"
    "       {                                                                               \n"
    "               int iv = int(dot(i, ivec4(16, 8, 4, 2)));                               \n"
    "               ivec4 v1 = texelFetch(u_config_texture, iv+0, 0);                       \n"
    "               ivec4 v2 = texelFetch(u_config_texture, iv+1, 0);                       \n"
    "               if (si % 2 == 0) make_quad(v1, v2);                                     \n"
    "               else make_triangle(v1, v2);                                             \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       process_tetrahedron(0, 2, 4, 1);                                                \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform samplerCube u_env_texture;                                                     \n"
    "in vec3 f_normal, f_light_dir, f_view_dir;                                             \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 normal = normalize(f_normal);                                              \n"
    "       vec3 light_dir = normalize(f_light_dir);                                        \n"
    "       vec3 view_dir = normalize(f_view_dir);                                          \n"
    "       float light_refl = dot(reflect(-light_dir, normal), view_dir);                  \n"
    "       float light_hit = dot(normal, light_dir);                                       \n"
    "       float specular = pow(clamp(light_refl+0.1, 0.0, 1.0), 32);                      \n"
    "       float diffuse1 = pow(max(light_hit*0.6+0.4, 0.0), 2.0);                         \n"
    "       float diffuse2 = sqrt(max(light_hit+0.2, 0.0));                                 \n"
    "       float diffuse = diffuse1 * 0.8 + diffuse2 * 0.2;                                \n"
    "       float view_light = max(0.3-dot(view_dir, light_dir), 0.0);                      \n"
    "       vec3 environ = texture(u_env_texture, reflect(-view_dir, normal)).rgb;          \n"
    "       final_color =                                                                   \n"
    "               environ * 0.1 +                                                         \n"
    "               vec3(0.4, 0.4, 0.8) * diffuse+                                          \n"
    "               vec3(0.2, 0.2, 0.3) * specular;                                         \n"
    "}                                                                                      \n"
};

/* clang-format on */

class GridArray : public shapes::Array {
public:
	SpuTexture m_configTexture;
	SpuTexture m_envTexture;

	Vec3f u_grid_offset;
	float u_time;
	uint32_t u_config_texture;
	Mat4f u_worldview;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	uint32_t u_env_texture;

	explicit GridArray(float quality)
	{
		// shader
		{
			Attrs shader_attrs = {
			        {"frag", c_frag},
                                {"vert", c_vert},
                                {"geom", c_geom}
                        };

			Attrs unif_attrs = {
			        {"u_grid_offset",    &u_grid_offset   },
			        {"u_time",           &u_time          },
			        {"u_config_texture", &u_config_texture},
			        {"u_worldview",      &u_worldview     },
			        {"u_eye_position",   &u_eye_position  },
			        {"u_light_position", &u_light_position},
			        {"u_env_texture",    &u_env_texture   },
			};
			Array::initShader(shader_attrs, unif_attrs);
		}

		// config texture
		{
			const uint32_t a = 0x0;
			const uint32_t b = 0x2;
			const uint32_t c = 0x4;
			const uint32_t d = 0x1;
			const uint32_t x = 0xFF;

			const uint32_t tex_data[]
			        = {x, x, x, x, x, x, x, x, a, c, b, x, d, d, d, x, b, d, a, x, c, c, c, x, b, b,
			           a, a, c, d, c, d, a, d, c, x, b, b, b, x, a, a, c, c, b, d, b, d, a, a, d, d,
			           c, b, c, b, a, a, a, x, b, d, c, x, c, d, b, x, a, a, a, x, c, c, b, b, a, d,
			           a, d, b, d, b, d, c, c, a, a, b, b, b, x, c, d, a, x, c, d, c, d, a, a, b, b,
			           c, c, c, x, a, d, b, x, d, d, d, x, a, b, c, x, x, x, x, x, x, x, x, x};

			Attrs attrs = {
			        {"target",     GL_TEXTURE_1D   },
			        {"iformat",    GL_RGBA32UI     },
			        {"width",      128             },
			        {"min_filter", GL_NEAREST      },
			        {"mag_filter", GL_NEAREST      },
			        {"wrap_s",     GL_CLAMP_TO_EDGE},
			        {"data",       tex_data        },
			};
			m_configTexture.init(attrs);
			u_config_texture = m_configTexture.id();
		}

		// env texture
		{
			auto image = images::Squares(512, 512, 0.9, 8, 8);

			auto unit = image.dataSize();
			std::vector<uint8_t> pix(unit * 6);

			for (auto i = 0; i < 6; i++) {
				memcpy(&pix[unit * i], image.data(), image.dataSize());
			}

			int32_t cube_target[6] = {
			        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
			};

			Attrs attrs = {
			        {"target",      GL_TEXTURE_CUBE_MAP    },
			        {"iformat",     GL_R8                  },
			        {"width",       image.width()          },
			        {"height",      image.height()         },
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
			        {"wrap_s",      GL_CLAMP_TO_EDGE       },
			        {"wrap_t",      GL_CLAMP_TO_EDGE       },
			        {"wrap_r",      GL_CLAMP_TO_EDGE       },
			        {"swizzle_g",   GL_RED                 },
			        {"swizzle_b",   GL_RED                 },
			        {"data",        pix.data()             },
			        {"cube_target", cube_target            },
			};
			m_envTexture.init(attrs);
			u_env_texture = m_envTexture.id();
		}

		shapes::Tetrahedrons make_grid(1.0, 16 + quality * quality * 64);
		Array::initArray<shapes::Tetrahedrons, shapes::Shape::WithAdjacencyTag>(
		        make_grid, {"position"});
	}
};

class App : public SpuPage {
public:
	static constexpr float c_quality = 0.5;
	static constexpr int32_t c_grid_repeat = 1 + c_quality * 2;

	GridArray m_array;

	App(const char *name) : SpuPage(name, true, {0.7, 0.65, 0.55, 0.0}), m_array(c_quality) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		const auto light_position = Vec3f(12.0, 1.0, 8.0);

		m_array.u_light_position = light_position;

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.flags.ccw = false;
		renderstate.cull_face = GL_BACK;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto viewscreen = math::perspective(viewport(0), 65, 1, 100);
		auto worldview = Mat4f::orbiting(ezero(), esec, 5, -1, 31, 0, 71, 55, 30, 27);
		auto eye_position = worldview.unitary_inverse().c[3];

		m_array.u_time = esec;
		m_array.u_eye_position = eye_position;
		m_array.u_worldview = viewscreen * worldview;

		for (auto z = -c_grid_repeat; z != c_grid_repeat; ++z) {
			for (auto x = -c_grid_repeat; x != c_grid_repeat; ++x) {
				m_array.u_grid_offset = {float(x), -0.5f, float(z)};
				m_array.draw(nullptr);
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("029_waves");
}  // namespace
}  // namespace spu::oglplus
