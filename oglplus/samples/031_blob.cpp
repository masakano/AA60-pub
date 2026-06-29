//
// BlobArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/brushed_metal.hpp>
#include <shapes/plane.hpp>
#include <shapes/tetrahedrons.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_blob_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_grid_offset;                                                            \n"
    "uniform sampler1D u_metaballs_texture;                                                 \n"
    "in vec3 a_position;                                                                    \n"
    "out vec3 g_center;                                                                     \n"
    "out float g_value;                                                                     \n"
    "flat out int g_inside;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 g_position = a_position + u_grid_offset;                                   \n"
    "       float sum = 0.0;                                                                \n"
    "       g_value = 0.0;                                                                  \n"
    "       g_center = vec3(0.0, 0.0, 0.0);                                                 \n"
    "       int ball = 0, ball_count = textureSize(u_metaballs_texture, 0);                 \n"
    "       while (ball != ball_count)                                                      \n"
    "       {                                                                               \n"
    "               vec4 metaball = texelFetch(u_metaballs_texture, ball, 0);               \n"
    "               vec3 center = metaball.xyz;                                             \n"
    "               float radius = metaball.w;                                              \n"
    "               vec3 vect = g_position - center;                                        \n"
    "               float tmp = (radius*radius)/dot(vect, vect);                            \n"
    "               g_value += tmp - 0.25;                                                  \n"
    "               float mul = max(tmp - 0.10, 0.0);                                       \n"
    "               g_center += mul * center;                                               \n"
    "               sum += mul;                                                             \n"
    "               ++ball;                                                                 \n"
    "       }                                                                               \n"
    "       if (sum > 0.0) g_center = g_center / sum;                                       \n"
    "       g_inside = (g_value >= 0.0)?1:0;                                                \n"
    "       gl_Position = vec4(g_position, 1.0);                                            \n"
    "}                                                                                      \n"
};

const char *c_blob_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles_adjacency) in;                                                        \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "uniform usampler1D u_config_texture;                                                   \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "in vec3 g_center[];                                                                    \n"
    "in float g_value[];                                                                    \n"
    "flat in int g_inside[];                                                                \n"
    "out vec3 f_normal, f_light_dir, f_view_dir;                                            \n"
    "float find_t(const uint i1, const uint i2)                                             \n"
    "{                                                                                      \n"
    "       float d = g_value[i2] - g_value[i1];                                            \n"
    "       if (d == 0.0) return 0.5;                                                       \n"
    "       else return -g_value[i1]/d;                                                     \n"
    "}                                                                                      \n"
    "void make_vertex(const uint i1, const uint i2)                                         \n"
    "{                                                                                      \n"
    "       float t = find_t(i1, i2);                                                       \n"
    "       gl_Position = mix(                                                              \n"
    "               gl_in[i1].gl_Position,                                                  \n"
    "               gl_in[i2].gl_Position,                                                  \n"
    "               t                                                                       \n"
    "       );                                                                              \n"
    "       f_normal = normalize(                                                           \n"
    "               gl_Position.xyz -                                                       \n"
    "               mix(g_center[i1], g_center[i2], t)                                      \n"
    "       );                                                                              \n"
    "       f_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       f_view_dir = u_eye_position - gl_Position.xyz;                                  \n"
    "       gl_Position = u_worldview * gl_Position;                                        \n"
    "       EmitVertex();                                                                   \n"
    "}                                                                                      \n"
    "void make_triangle(const uvec4 v1, const uvec4 v2)                                     \n"
    "{                                                                                      \n"
    "       make_vertex(v1.x, v2.x);                                                        \n"
    "       make_vertex(v1.y, v2.y);                                                        \n"
    "       make_vertex(v1.z, v2.z);                                                        \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
    "void make_quad(const uvec4 v1, const uvec4 v2)                                         \n"
    "{                                                                                      \n"
    "       make_vertex(v1.x, v2.x);                                                        \n"
    "       make_vertex(v1.y, v2.y);                                                        \n"
    "       make_vertex(v1.z, v2.z);                                                        \n"
    "       make_vertex(v1.w, v2.w);                                                        \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
    "void process_tetrahedron(int a, int b, int c, int d)                                   \n"
    "{                                                                                      \n"
    "       ivec4 i = ivec4(                                                                \n"
    "               g_inside[a],                                                            \n"
    "               g_inside[b],                                                            \n"
    "               g_inside[c],                                                            \n"
    "               g_inside[d]                                                             \n"
    "       );                                                                              \n"
    "       int si = int(dot(i, ivec4(1, 1, 1, 1))) % 4;                                    \n"
    "       if (si != 0)                                                                    \n"
    "       {                                                                               \n"
    "               int iv = int(dot(i, ivec4(16, 8, 4, 2)));                               \n"
    "               uvec4 v1 = texelFetch(u_config_texture, iv+0, 0);                       \n"
    "               uvec4 v2 = texelFetch(u_config_texture, iv+1, 0);                       \n"
    "               if (si % 2 == 0) make_quad(v1, v2);                                     \n"
    "               else make_triangle(v1, v2);                                             \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       process_tetrahedron(0, 2, 4, 1);                                                \n"
    "}                                                                                      \n"
};

const char *c_blob_frag = {
    "#version 330                                                                           \n"
    "in vec3 f_normal, f_light_dir, f_view_dir;                                             \n"
    "out vec4 final_color;                                                                  \n"
    "const vec3 light_color = vec3(1.0, 1.0, 0.9);                                          \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 color = vec3(1.0, 1.0, 0.9);                                               \n"
    "       vec3 normal = normalize(f_normal);                                              \n"
    "       vec3 light_dir = normalize(f_light_dir);                                        \n"
    "       vec3 view_dir = normalize(f_view_dir);                                          \n"
    "       vec3 light_refl = reflect(-light_dir, normal);                                  \n"
    "       float specular = pow(max(dot(light_refl,view_dir), 0.0), 64);                   \n"
    "       float light_hit = dot(normal, light_dir);                                       \n"
    "       float diffuse = pow(1.05*max(light_hit-0.05, 0.0), 2.0);                        \n"
    "       float view_light = max(0.3-dot(view_dir, light_dir), 0.0);                      \n"
    "       float translucent =                                                             \n"
    "               view_light*                                                             \n"
    "               pow(0.4*exp(sin(-0.3-light_hit*(1.2+1.2*view_light))),3.0)*             \n"
    "               0.2;                                                                    \n"
    "       float ambient = 0.5;                                                            \n"
    "       final_color = vec4(                                                             \n"
    "               color * ambient +                                                       \n"
    "               light_color * color * (diffuse + translucent) +                         \n"
    "               light_color * specular,                                                 \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_metal_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal, a_tangent;                                                           \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 g_normal, g_tangent, g_bitangent;                                             \n"
    "out vec3 g_light_dir, g_view_dir;                                                      \n"
    "out vec2 g_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position - vec4(0.0, 1.0, 0.0, 0.0);                            \n"
    "       g_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       g_view_dir = u_eye_position - gl_Position.xyz;                                  \n"
    "       g_normal = a_normal;                                                            \n"
    "       g_tangent = a_tangent;                                                          \n"
    "       g_bitangent = cross(g_normal, g_tangent);                                       \n"
    "       g_texcoord = a_texcoord * 9.0;                                                  \n"
    "       gl_Position = u_worldview * gl_Position;                                        \n"
    "}                                                                                      \n"
};

const char *c_metal_frag =  {
    "#version 330                                                                           \n"
    "const vec3 color1 = vec3(0.7, 0.6, 0.5);                                               \n"
    "const vec3 color2 = vec3(0.9, 0.8, 0.7);                                               \n"
    "uniform sampler2D u_metal_texture;                                                     \n"
    "in vec3 g_normal, g_tangent, g_bitangent;                                              \n"
    "in vec3 g_light_dir, g_view_dir;                                                       \n"
    "in vec2 g_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 sample = texture(u_metal_texture, g_texcoord).rgb;                         \n"
    "       vec3 light_color = vec3(1.0, 1.0, 0.9);                                         \n"
    "       vec3 normal = normalize(                                                        \n"
    "               2.0*g_normal +                                                          \n"
    "               (sample.r - 0.5)*g_tangent +                                            \n"
    "               (sample.g - 0.5)*g_bitangent                                            \n"
    "       );                                                                              \n"
    "       vec3 light_refl = reflect(                                                      \n"
    "               -normalize(g_light_dir),                                                \n"
    "               normal                                                                  \n"
    "       );                                                                              \n"
    "       float specular = pow(max(dot(                                                   \n"
    "               normalize(light_refl),                                                  \n"
    "               normalize(g_view_dir)                                                   \n"
    "       )+0.04, 0.0), 16+sample.b*48)*pow(0.4+sample.b*1.6, 4.0);                       \n"
    "       normal = normalize(g_normal*3.0 + normal);                                      \n"
    "       float diffuse = pow(max(dot(                                                    \n"
    "               normalize(normal),                                                      \n"
    "               normalize(g_light_dir)                                                  \n"
    "       ), 0.0), 2.0);                                                                  \n"
    "       float ambient = 0.5;                                                            \n"
    "       vec3 color = mix(color1, color2, sample.b);                                     \n"
    "       final_color =                                                                   \n"
    "               color * ambient +                                                       \n"
    "               light_color * color * diffuse +                                         \n"
    "               light_color * specular;                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class BlobArray : public shapes::Array {
public:
	SpuTexture m_configTexture;
	Mat4f u_worldview;
	Vec3f u_grid_offset;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	uint32_t u_metaballs_texture;
	uint32_t u_config_texture;

	explicit BlobArray(float quality)
	{
		Attrs shader_attrs = {
		        {"frag", c_blob_frag},
		        {"vert", c_blob_vert},
		        {"geom", c_blob_geom},
		};

		Attrs unif_attrs = {
		        {"u_worldview",         &u_worldview        },
		        {"u_grid_offset",       &u_grid_offset      },
		        {"u_eye_position",      &u_eye_position     },
		        {"u_light_position",    &u_light_position   },
		        {"u_metaballs_texture", &u_metaballs_texture},
		        {"u_config_texture",    &u_config_texture   },
		};

		Array::initShader(shader_attrs, unif_attrs);

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
		        {"target",      GL_TEXTURE_1D   },
                        {"iformat",     GL_RGBA32UI     },
                        {"width",       32              },
		        {"min_filter",  GL_NEAREST      },
                        {"mag_filter",  GL_NEAREST      },
                        {"wrap_s",      GL_CLAMP_TO_EDGE},
		        {"auto_mipmap", 0               },
                        {"data",        tex_data        },
		};
		m_configTexture.init(attrs);
		u_config_texture = m_configTexture.id();

		shapes::Tetrahedrons shape(1.0, 12 + quality * 24);
		Array::initArray<shapes::Tetrahedrons, shapes::Shape::WithAdjacencyTag>(shape, {"position"});
	}
};

class MetalArray : public shapes::Array {
public:
	Mat4f u_worldview;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	uint32_t u_metal_texture;

	MetalArray()
	{
		Attrs shader_attrs = {
		        {"frag", c_metal_frag},
		        {"vert", c_metal_vert},
		};

		Attrs unif_attrs = {
		        {"u_worldview",      &u_worldview     },
		        {"u_eye_position",   &u_eye_position  },
		        {"u_light_position", &u_light_position},
		        {"u_metal_texture",  &u_metal_texture },
		};
		Array::initShader(shader_attrs, unif_attrs);
	}
};

class App : public SpuPage {
public:
	BlobArray m_blob;
	MetalArray m_plane;

	SpuTexture m_metaballTexture;
	SpuTexture m_metalTexture;
	std::vector<CubicBezierLoop<Vec4f, double>> m_ballPaths;

	static constexpr float c_quality = 0.5;
	App(const char *name) : SpuPage(name, true, {0.8, 0.7, 0.6, 0.0}), m_blob(c_quality) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		auto shape = shapes::Plane(Vec3f(9, 0, 0), Vec3f(0, 0, -9));

		m_plane.initArray(shape, {"position", "normal", "tangent", "texcoord"});

		RandomGenerator<float> frand;
		for (auto i = 0u; i != 24; ++i) {
			auto j = 0u;

			auto n = 3 + uint32_t(frand() * 3.0);

			std::vector<Vec4f> points(n);
			auto ball_size = 0.15f * frand() + 0.25f;
			while (j != n) {
				points[j] = {
				        Vec2f(1.2 * frand() - 0.6),
				        1.2f * frand() - 0.6f,
				        ball_size,
				};
				++j;
			}

			CubicBezierLoop<Vec4f, double> ball_path;
			ball_path.init(points);
			m_ballPaths.emplace_back(ball_path);
		}

		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_1D     },
			        {"iformat",     GL_RGBA32F        },
			        {"width",       m_ballPaths.size()},
			        {"min_filter",  GL_NEAREST        },
			        {"mag_filter",  GL_NEAREST        },
			        {"wrap_s",      GL_CLAMP_TO_EDGE  },
			        {"auto_mipmap", 0                 },
			};
			m_metaballTexture.init(attrs);
			m_blob.u_metaballs_texture = m_metaballTexture.id();
		}

		{
			auto image = images::BrushedMetalUByte(512, 512, 5120, -3, +3, 32, 128);

			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D          },
			        {"iformat",    GL_RGB8                }, // need functional
			        {"width",      image.width()          },
			        {"height",     image.height()         },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_REPEAT              },
			        {"wrap_t",     GL_REPEAT              },
			        {"data",       image.data()           },
			};
			m_metalTexture.init(attrs);
			m_plane.u_metal_texture = m_metalTexture.id();
		}

		const auto light_position = Vec3f(12.0, 1.0, 8.0);
		m_blob.u_light_position = light_position;
		m_plane.u_light_position = light_position;

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.flags.ccw = false;
		renderstate.cull_face = GL_BACK;
	}

	void render() override
	{
		auto esec = getSeconds().current();

		// update material
		{
			auto metaball_count = m_ballPaths.size();
			std::vector<Vec4f> metaballs(metaball_count);
			for (auto ball = 0u; ball != metaball_count; ++ball) {
				metaballs[ball] = m_ballPaths[ball].position(esec / 21.0);
			}
			spu_texture_send(m_blob.u_metaballs_texture, metaballs.data(), GL_RGBA32F);
		}

		// render
		{
			auto viewscreen = math::perspective(viewport(0), 48, 1, 100);
			auto worldview = Mat4f::orbiting(ezero(), esec, 4, -1, 14, 0, 26, 45, 40, 17);
			auto eye_position = worldview.unitary_inverse().c[3];

			m_plane.u_eye_position = eye_position;
			m_plane.u_worldview = viewscreen * worldview;
			m_plane.draw(nullptr);

			m_blob.u_eye_position = eye_position;
			m_blob.u_worldview = viewscreen * worldview;

			auto side = 1;
			for (auto z = -side; z != side; ++z) {
				for (auto y = -side; y != side; ++y) {
					for (auto x = -side; x != side; ++x) {
						m_blob.u_grid_offset = Vec3f(x, y, z);
						m_blob.draw(nullptr);
					}
				}
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("031_blob");
}  // namespace
}  // namespace spu::oglplus
