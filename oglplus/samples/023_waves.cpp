//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/squares.hpp>
#include <shapes/plane.hpp>
#include "replace_text.h"

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 410                                                                           \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "in vec3 a_position;                                                                    \n"
    "out vec3 tc_position;                                                                  \n"
    "out float tc_distance;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       tc_position = a_position;                                                       \n"
    "       tc_distance = distance(u_eye_position, a_position);                             \n"
    "}                                                                                      \n"
};

const char *c_tesc =  {
    "#version 410                                                                           \n"
    "layout(vertices = 3) out;                                                              \n"
    "in vec3 tc_position[];                                                                 \n"
    "in float tc_distance[];                                                                \n"
    "out vec3 te_position[];                                                                \n"
    "int tess_level(float dist)                                                             \n"
    "{                                                                                      \n"
    "       return clamp(int(150.0 / (dist+0.1)), 1, 10);                                   \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       te_position[gl_InvocationID] =                                                  \n"
    "               tc_position[gl_InvocationID];                                           \n"
    "       if (gl_InvocationID == 0)                                                       \n"
    "       {                                                                               \n"
    "               gl_TessLevelInner[0] = tess_level((                                     \n"
    "                       tc_distance[0]+                                                 \n"
    "                       tc_distance[1]+                                                 \n"
    "                       tc_distance[2]                                                  \n"
    "               )*0.333);                                                               \n"
    "               gl_TessLevelOuter[0] = tess_level((                                     \n"
    "                       tc_distance[1]+                                                 \n"
    "                       tc_distance[2]                                                  \n"
    "               )*0.5);                                                                 \n"
    "               gl_TessLevelOuter[1] = tess_level((                                     \n"
    "                       tc_distance[2]+                                                 \n"
    "                       tc_distance[0]                                                  \n"
    "               )*0.5);                                                                 \n"
    "               gl_TessLevelOuter[2] = tess_level((                                     \n"
    "                       tc_distance[0]+                                                 \n"
    "                       tc_distance[1]                                                  \n"
    "               )*0.5);                                                                 \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_tese = {
    "#version 410                                                                           \n"
    //"#define max_waves 5                                                                    \n"
    "layout(triangles, equal_spacing, ccw) in;                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "uniform float u_time;                                                                  \n"
    "uniform int u_wave_count;                                                              \n"
    "uniform vec4 u_wave_directions[max_waves];                                             \n"
    "uniform vec4 u_wave_dimensions[max_waves];                                             \n"
    "in vec3 te_position[];                                                                 \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out float f_distance;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       const vec3 up = vec3(0.0, 1.0, 0.0);                                            \n"
    "       vec3 position =                                                                 \n"
    "               gl_TessCoord.x * te_position[0]+                                        \n"
    "               gl_TessCoord.y * te_position[1]+                                        \n"
    "               gl_TessCoord.z * te_position[2];                                        \n"
    "       vec3 pos = position;                                                            \n"
    "       vec3 nml = up;                                                                  \n"
    "       int w;                                                                          \n"
    "       for (w=0; w!=u_wave_count; w++)                                                 \n"
    "       {                                                                               \n"
    "               vec3 dir = u_wave_directions[w].xyz;                                    \n"
    "               vec3 dim = u_wave_dimensions[w].xyz;                                    \n"
    "               float dist = dot(position, dir);                                        \n"
    "               float u = dim.y*sin(dist/dim.x + u_time*dim.z);                         \n"
    "               pos += up * u;                                                          \n"
    "               float w = (dim.y/dim.x)*cos(dist/dim.x + u_time*dim.z);                 \n"
    "               nml -= dir * w;                                                         \n"
    "               float d = -0.125*dim.x*sin(2.0*dist/dim.x + u_time*dim.z);              \n"
    "               pos += dir * d;                                                         \n"
    "       }                                                                               \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               vec4(pos, 1.0);                                                         \n"
    "       f_normal = normalize(nml);                                                      \n"
    "       f_light_dir = normalize(u_light_position - pos);                                \n"
    "       f_view_dir = normalize(u_eye_position - pos);                                   \n"
    "       f_distance = distance(u_eye_position, pos);                                     \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 410                                                                           \n"
    "uniform samplerCube u_texture;                                                         \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in float f_distance;                                                                   \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float dim = clamp(30.0/f_distance, 0.0, 1.0);                                   \n"
    "       float light_refl = dot(reflect(-f_light_dir, f_normal), f_view_dir);            \n"
    "       float light_hit = dot(f_normal, f_light_dir);                                   \n"
    "       float diffuse = clamp(light_hit+0.1, 0.0, 1.0);                                 \n"
    "       float specular = pow(clamp(light_refl, 0.0, 0.91), 32);                         \n"
    "       vec3 environ=texture(u_texture,reflect(-f_view_dir, f_normal)).rgb;             \n"
    "       vec3 water_color = vec3(0.4, 0.5, 0.5);                                         \n"
    "       vec3 light_color = vec3(1.0, 1.0, 1.0);                                         \n"
    "       vec3 fog_color = vec3(0.9, 0.9, 0.9);                                           \n"
    "       vec3 wave_color =                                                               \n"
    "               light_color*specular+                                                   \n"
    "               water_color*diffuse+                                                    \n"
    "               environ*0.02;                                                           \n"
    "       final_color = mix(wave_color, fog_color, 1.0-dim);                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	static constexpr int32_t c_max_waves = 5;
	const Attrs replace_attrs = {
		{"max_waves", c_max_waves},
	};

	int32_t m_prevPeriod = -1;

	shapes::Array m_array;
	SpuTexture m_texture;

	Vec3f u_wave_directions[c_max_waves];
	Vec3f u_wave_dimensions[c_max_waves];
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Vec3f u_light_position;
	Vec3f u_eye_position;
	float u_time;
	uint32_t u_texture;
	int32_t u_wave_count;

	App(const char *name) : SpuPage(name, true, {0.1, 0.1, 0.2, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			/*
			auto frag = replaceText(c_frag, replace_attrs);
			auto vert = replaceText(c_vert, replace_attrs);
			auto tesc = replaceText(c_tesc, replace_attrs);
			auto tese = replaceText(c_tese, replace_attrs);
			*/

			Attrs shader_attrs = {
			        {"frag", replaceText(c_frag, replace_attrs)},
			        {"vert", replaceText(c_vert, replace_attrs)},
			        {"tesc", replaceText(c_tesc, replace_attrs)},
			        {"tese", replaceText(c_tese, replace_attrs)},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen",       &u_viewsceen         },
			        {"u_worldview",       &u_worldview         },
			        {"u_eye_position",    &u_eye_position      },
			        {"u_light_position",  &u_light_position    },
			        {"u_time",            &u_time              },
			        {"u_wave_count",      &u_wave_count        },
			        {"u_wave_directions", &u_wave_directions[0]},
			        {"u_wave_dimensions", &u_wave_dimensions[0]},
			        {"u_texture",         &u_texture           },
			};
			m_array.initShader(shader_attrs, unif_attrs);

			u_light_position = {-100.0, 100.0, 20.0};
			u_wave_count = 5;

			u_wave_directions[0] = normalize(Vec3f(1, 0, 1));
			u_wave_dimensions[0] = {5.0, 1.5, 1.2};

			u_wave_directions[1] = normalize(Vec3f(1.0, 0.0, 0.5));
			u_wave_dimensions[1] = {4.0, 0.8, 1.2};

			u_wave_directions[2] = normalize(Vec3f(1.0, 0.0, 0.1));
			u_wave_dimensions[2] = {2.0, 0.5, 2.4};

			u_wave_directions[3] = normalize(Vec3f(1.0, 0.0, -0.1));
			u_wave_dimensions[3] = {1.5, 0.2, 3.7};

			u_wave_directions[4] = normalize(Vec3f(1.0, 0.0, 0.4));
			u_wave_dimensions[4] = {1.1, 0.2, 4.7};
		}
		{
			shapes::Plane plane_shape(
			        Vec3f(0, 0, 0), Vec3f(100.0, 0.0, 0.0), Vec3f(0.0, 0.0, -100.0), 50, 50);
			m_array.initArray<shapes::Plane, shapes::Shape::PatchesTag>(plane_shape, {"position"});
			m_array.setMode(GL_PATCHES);
			m_array.setCount(0);  // not trianble count
		}
		{
			auto image = images::Squares(512, 512, 0.9, 16, 16);

			auto unit = image.dataSize();
			std::vector<uint8_t> pix(unit * 6);

			for (auto i = 0; i < 6; i++) {
				memcpy(&pix[unit * i], image.data(), image.dataSize());
			}

			Attrs tex_attrs = {
			        {"target",     GL_TEXTURE_CUBE_MAP     },
                                {"iformat",    GL_R8                   },
			        {"width",      image.width()           },
                                {"height",     image.height()          },
			        {"min_filter", GL_LINEAR               },
                                {"mag_filter", GL_LINEAR               },
			        {"wrap_s",     GL_CLAMP_TO_EDGE        },
                                {"wrap_t",     GL_CLAMP_TO_EDGE        },
			        {"wrap_r",     GL_CLAMP_TO_EDGE        },
                                {"swizzle_g",  GL_RED                  },
			        {"swizzle_b",  GL_RED                  },
                                {"data",       (const void *)pix.data()},
			};
			m_texture.init(tex_attrs);
			u_texture = m_texture.id();
		}
	}

	void render() override
	{
		auto &renderstate = SpuPage::getRenderstate();
		auto esec = getSeconds().current();
		auto period = int(esec * 0.125);
		if (m_prevPeriod < period) {
			if ((period % 2) != 0) {
				renderstate.flags.fill = false;
			}
			else {
				renderstate.flags.fill = true;
			}
			m_prevPeriod = period;
			renderstate.use();
		}

		u_viewsceen = math::perspective(viewport(0), 75, 1, 150);
		u_worldview = Mat4f::orbiting(Vec3f(0, 2, 0), esec, 17, -10, 31, 0, 43, 45, -35, 29);
		u_eye_position = u_worldview.unitary_inverse().c[3];
		u_time = esec;
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("023_waves");
}  // namespace
}  // namespace spu::oglplus
