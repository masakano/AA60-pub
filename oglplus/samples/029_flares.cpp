//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/brushed_metal.hpp>
#include <images/load.hpp>
#include <shapes/spiral_sphere.hpp>
#include "replace_text.h"

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_shape_vert =         {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec3 a_tangent;                                                                     \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 g_position;                                                                   \n"
    "out vec3 g_view_dir;                                                                   \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_tangent;                                                                    \n"
    "out vec3 g_bitangent;                                                                  \n"
    "out vec2 g_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       g_position = gl_Position.xyz;                                                   \n"
    "       g_view_dir = u_eye_position - gl_Position.xyz;                                  \n"
    "       g_normal =  (u_nodeworld * vec4(a_normal, 0.0)).xyz;                            \n"
    "       g_tangent =  (u_nodeworld * vec4(a_tangent, 0.0)).xyz;                          \n"
    "       g_bitangent = cross(g_normal, g_tangent);                                       \n"
    "       g_texcoord = mat2(0.0, 2.0, 1.0, 0.0) * a_texcoord;                             \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               gl_Position;                                                            \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =         {
    "#version 330                                                                           \n"
    "uniform vec3 u_color1;                                                                 \n"
    "uniform vec3 u_color2;                                                                 \n"
    "uniform vec4 u_light_position[c_light_count];                                          \n"
    "uniform sampler2D u_metal_texture;                                                     \n"
    "in vec3 g_position;                                                                    \n"
    "in vec3 g_view_dir;                                                                    \n"
    "in vec3 g_normal;                                                                      \n"
    "in vec3 g_tangent;                                                                     \n"
    "in vec3 g_bitangent;                                                                   \n"
    "in vec2 g_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "const vec3 light_color = vec3(1.0, 1.0, 1.0);                                          \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec4 sample = texture(u_metal_texture, g_texcoord);                             \n"
    "       vec3 frag_normal = normalize(                                                   \n"
    "               (sample.b + 0.5)*g_normal +                                             \n"
    "               (sample.r - 0.5)*g_tangent +                                            \n"
    "               (sample.g - 0.5)*g_bitangent                                            \n"
    "       );                                                                              \n"
    "       float specular = 0.0, diffuse = 0.0;                                            \n"
    "       for (int l=0; l!=c_light_count; ++l)                                            \n"
    "       {                                                                               \n"
    "               vec3 light_dir = normalize(u_light_position[l].xyz-g_position);         \n"
    "               vec3 light_refl = reflect(                                              \n"
    "                       -light_dir,                                                     \n"
    "                       frag_normal                                                     \n"
    "               );                                                                      \n"
    "               specular += pow(max(dot(                                                \n"
    "                       normalize(light_refl),                                          \n"
    "                       normalize(g_view_dir)                                           \n"
    "               )+0.04, 0.0), 32+sample.b*32)*pow(0.4+sample.b*1.6, 4.0);               \n"
    "               diffuse += pow(max(dot(                                                 \n"
    "                       light_dir,                                                      \n"
    "                       normalize(g_normal*2.0 + frag_normal)                           \n"
    "               ), 0.0), 2.0);                                                          \n"
    "       }                                                                               \n"
    "       float ambient = 0.1;                                                            \n"
    "       diffuse /= c_light_count;                                                       \n"
    "       specular /= sqrt(float(c_light_count));                                         \n"
    "       vec3 u_color = mix(u_color1, u_color2, sample.b);                               \n"
    "       final_color =                                                                   \n"
    "               u_color * ambient +                                                     \n"
    "               u_color * diffuse +                                                     \n"
    "               light_color * specular;                                                 \n"
    "}                                                                                      \n"
};

const char *c_light_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_viewsceen * u_worldview * vec4(a_position.xyz,1);               \n"
    "       gl_PointSize = 9.0;                                                             \n"
    "}                                                                                      \n"
};

const char *c_light_frag =  {
    "#version 330                                                                           \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec3(1.0, 1.0, 1.0);                                              \n"
    "}                                                                                      \n"
};

const char *c_flare_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_worldview * vec4(a_position.xyz,1);                             \n"
    "       gl_PointSize = 9.0;                                                             \n"
    "}                                                                                      \n"
};

const char *c_flare_geom =  {
    "#version 330                                                                           \n"
    "layout(points) in;                                                                     \n"
    "layout(triangle_strip, max_vertices = 72) out;                                         \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform int u_samples;                                                                 \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       for (int l=0; l!=3; ++l)                                                        \n"
    "       {                                                                               \n"
    "               int i = 0, n = 8 + l*4;                                                 \n"
    "               float step = (2.0 * 3.1415)/float(n-1);                                 \n"
    "               float a = length(gl_in[0].gl_Position)*(0.3+l*0.4);                     \n"
    "               float radius =                                                          \n"
    "                       sqrt(u_samples)*0.01 +                                          \n"
    "                       u_samples*sqrt(float(l))*0.001;                                 \n"
    "               while (i != n)                                                          \n"
    "               {                                                                       \n"
    "                       vec4 offs = vec4(cos(a)*(1.0+l*0.2),sin(a),0,0);                \n"
    "                       gl_Position =                                                   \n"
    "                               u_viewsceen *                                           \n"
    "                               gl_in[0].gl_Position;                                   \n"
    "                       f_texcoord = vec2(float(i), 0.0);                               \n"
    "                       EmitVertex();                                                   \n"
    "                       gl_Position =                                                   \n"
    "                               u_viewsceen *                                           \n"
    "                               (gl_in[0].gl_Position + offs*radius);                   \n"
    "                       f_texcoord = vec2(float(i), 1.05-l*0.05);                       \n"
    "                       EmitVertex();                                                   \n"
    "                       ++i;                                                            \n"
    "                       a += step;                                                      \n"
    "               }                                                                       \n"
    "               EndPrimitive();                                                         \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_flare_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_flare_texture;                                                     \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec4 sample = texture(u_flare_texture, f_texcoord);                             \n"
    "       final_color = vec4(sample.rgb, sample.a*0.4);                                   \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_eye_position;
	Vec3f u_light_position[32];
	Vec3f u_color1;
	Vec3f u_color2;
	uint32_t u_metal_texture;
	uint32_t u_flare_texture;
	uint32_t u_samples;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen",      &u_viewsceen     },
		        {"u_worldview",      &u_worldview     },
		        {"u_nodeworld",      &u_nodeworld     },
		        {"u_eye_position",   &u_eye_position  },
		        {"u_light_position", &u_light_position},
		        {"u_color1",         &u_color1        },
		        {"u_color2",         &u_color2        },
		        {"u_metal_texture",  &u_metal_texture },
		        {"u_flare_texture",  &u_flare_texture },
		        {"u_samples",        &u_samples       },
		};
	}
	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class App : public SpuPage {
public:
	static constexpr auto c_light_count = 32;
	const Attrs replace_attrs = {
		{"c_light_count", c_light_count},
	};

	std::vector<uint32_t> m_queryIds;
	std::vector<uint64_t> m_sampleCounts;

	Uniforms m_unifs;

	shapes::Array m_shapeArray;

	SpuShader m_lightShader;
	SpuArray m_lightArray;

	SpuShader m_flareShader;
	SpuArray m_flareArray;

	SpuTexture m_metalTexture;
	SpuTexture m_flareTexture;

	App(const char *name) : SpuPage(name, true, {0.1, 0.1, 0.1, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shape_shader_attrs = {
		        {"frag", replaceText(c_shape_frag, replace_attrs)},
                        {"vert", c_shape_vert}
                };

		Attrs light_shader_attrs = {
		        {"frag", c_light_frag},
                        {"vert", c_light_vert}
                };

		Attrs flare_shader_attrs = {
		        {"frag", c_flare_frag},
                        {"vert", c_flare_vert},
                        {"geom", c_flare_geom}
                };

		m_shapeArray.initShader(shape_shader_attrs, Attrs(m_unifs));

		shapes::loadShader(m_lightShader, light_shader_attrs, Attrs(m_unifs));
		shapes::loadShader(m_flareShader, flare_shader_attrs, Attrs(m_unifs));

		m_shapeArray.initArray(shapes::SpiralSphere(), {"position", "normal", "tangent", "texcoord"});

		// queries
		{
			for (auto i = 0u; i < c_light_count; i++) {
				Attrs attrs = {
				        {"target", GL_SAMPLES_PASSED},
				};
				m_queryIds.push_back(spu_query_new(attrs));
				m_sampleCounts.push_back(0);
			}
		}

		std::vector<Vec3f> light_positions(c_light_count);
		RandomGenerator<float> frand;

		for (auto i = 0u; i != c_light_count; ++i) {
			auto angle = frand() * math::two_pi();
			light_positions[i] = Vec3f(7.0 * cosf(angle), 0.2 * frand() - 0.1, 7.0 * sinf(angle));
		}
		memcpy(m_unifs.u_light_position, light_positions.data(), sizeof(m_unifs.u_light_position));
		m_unifs.u_color1 = {0.3, 0.3, 0.5};
		m_unifs.u_color2 = {0.8, 0.8, 1.0};

		{
			auto image = images::BrushedMetalUByte(512, 512, 5120, -12, +12, 32, 64);

			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
                                {"iformat",     GL_RGB8                },
			        {"data",        image.data()           },
                                {"width",       image.width()          },
			        {"height",      image.height()         },
                                {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
                                {"wrap_s",      GL_REPEAT              },
			        {"wrap_t",      GL_REPEAT              },
                                {"auto_mipmap", 1                      },
			};
			m_metalTexture.init(attrs);
			m_unifs.u_metal_texture = m_metalTexture.id();
		}

		{
			auto image = images::LoadTexture("flare_1");

			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
                                {"iformat",     GL_RGBA8               },
			        {"data",        image.data()           },
                                {"width",       image.width()          },
			        {"height",      image.height()         },
                                {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
                                {"wrap_s",      GL_MIRRORED_REPEAT     },
			        {"wrap_t",      GL_REPEAT              },
                                {"auto_mipmap", 1                      },
			};
			m_flareTexture.init(attrs);
			m_unifs.u_flare_texture = m_flareTexture.id();
		}

		{
			Attrs attrs = {
			        {"shader_id",    m_lightShader.id()    },
			        {"a.a_position", 4                     },
			        {"data",         light_positions.data()},
			        {"nelem",        light_positions.size()},
			};
			m_lightArray.init(attrs);
		}
		{
			Attrs attrs = {
			        {"shader_id",    m_flareShader.id()    },
			        {"a.a_position", 4                     },
			        {"data",         light_positions.data()},
			        {"nelem",        light_positions.size()},
			};
			m_flareArray.init(attrs);
		}

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.program_point_size = true;
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_BACK;
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

		m_unifs.u_viewsceen = math::perspective(viewport(0), 65, 1, 20);
		m_unifs.u_worldview = Mat4f::orbiting(ezero(), esec, 4.5, 1.0, 25.0, 0, 30, 0, 20, 19);
		m_unifs.u_eye_position = m_unifs.u_worldview.unitary_inverse().c[3];
		m_unifs.u_nodeworld = math::unit().rot("x", -esec / 30.0 * math::two_pi());

		m_shapeArray.draw(nullptr);

		m_lightShader.use();
		for (auto l = 0u; l != c_light_count; ++l) {
			spu_query_begin(m_queryIds[l]);
			m_lightArray.draw(GL_POINTS, l, 1);
			spu_query_end(m_queryIds[l], &m_sampleCounts[l], true);
		}

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.blend = true;
		renderstate.flags.depth_test = false;
		renderstate.use();

		for (auto l = 0u; l != c_light_count; ++l) {
			if (m_sampleCounts[l] != 0) {
				m_unifs.u_samples = m_sampleCounts[l];
				m_flareShader.use();
				m_flareArray.draw(GL_POINTS, l, 1);
			}
		}

		renderstate.flags.depth_test = true;
		renderstate.flags.blend = false;
		renderstate.use();
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("029_flares");
}  // namespace
}  // namespace spu::oglplus
