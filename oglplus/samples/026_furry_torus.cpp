//
// FurArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/spiral_sphere.hpp>
#include <shapes/torus.hpp>

namespace spu::oglplus {

namespace {
/* clang-format off */
const char *c_fur_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_new_nodeworld;                                                          \n"
    "uniform mat4 u_old_nodeworld;                                                          \n"
    "uniform sampler2D u_fur_tex;                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 g_offset;                                                                     \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_new_nodeworld *                                                       \n"
    "               a_position;                                                             \n"
    "       g_offset = (                                                                    \n"
    "               gl_Position -                                                           \n"
    "               u_old_nodeworld *                                                       \n"
    "               a_position                                                              \n"
    "       ).xyz;                                                                          \n"
    "       g_normal = mat3(u_new_nodeworld) * a_normal;                                    \n"
    "       g_color = texture(u_fur_tex, a_texcoord).rgb;                                   \n"
    "}                                                                                      \n"
};

const char *c_fur_geom =  {
    "#version 330                                                                           \n"
    "#define point_count 4                                                                  \n"
    "layout(points) in;                                                                     \n"
    "layout(line_strip, max_vertices = point_count) out;                                    \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform float u_time;                                                                  \n"
    "const float fur_length = 0.45;                                                         \n"
    "const float seg_part = 1.0 / (point_count - 1);                                        \n"
    "const float seg_len = fur_length * seg_part;                                           \n"
    "in vec3 g_offset[];                                                                    \n"
    "in vec3 g_normal[];                                                                    \n"
    "in vec3 g_color[];                                                                     \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_fur_dir;                                                                    \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_color;                                                                      \n"
    "out float f_fur_part;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_color = g_color[0];                                                           \n"
    "       f_normal = g_normal[0];                                                         \n"
    "       f_fur_part = 0.0;                                                               \n"
    "       vec4 vert_pos = gl_in[0].gl_Position;                                           \n"
    "       float wind = sin(4.0*(vert_pos.x+u_time));                                      \n"
    "       for (int i=0; i!=point_count; ++i)                                              \n"
    "       {                                                                               \n"
    "               f_light_dir = normalize(u_light_position - vert_pos.xyz);               \n"
    "               f_fur_dir = normalize(                                                  \n"
    "                       g_normal[0] * 0.1 -                                             \n"
    "                       g_offset[0] * i*i*0.3 +                                         \n"
    "                       vec3(wind*i*i*0.01, 0.0, 0.0)                                   \n"
    "               );                                                                      \n"
    "               gl_Position = u_worldview * vert_pos;                                   \n"
    "               EmitVertex();                                                           \n"
    "               f_fur_part += seg_part;                                                 \n"
    "               vert_pos += vec4(f_fur_dir, 0.0) * seg_len;                             \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_fur_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_fur_dir;                                                                     \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_color;                                                                       \n"
    "in float f_fur_part;                                                                   \n"
    //"out vec3 final_color;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "const vec3 light_color = vec3(1.0, 1.0, 1.0);                                          \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       const float ambient = 0.3;                                                      \n"
    "       float fur_light = 1.0 - abs(dot(f_fur_dir, f_light_dir));                       \n"
    "       float shape_light = max(dot(f_normal, f_light_dir)+0.4, 0.0);                   \n"
    "       float diffuse = fur_light * shape_light;                                        \n"
    "       vec3 u_color = mix(vec3(0.2, 0.2, 0.2), f_color, f_fur_part);                   \n"
    "       final_color.rgb =                                                               \n"
    "               ambient * u_color +                                                     \n"
    "               sqrt(diffuse) * f_fur_part * u_color;                                   \n"
    "       final_color.a = 1.0;                                                            \n"
        //"     final_color = vec3(Diffuse);"
    "}                                                                                      \n"
};

const char *c_torus_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_worldview * u_nodeworld * a_position;                           \n"
    "}                                                                                      \n"
};

const char *c_torus_frag =  {
    "#version 330                                                                           \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(0.05, 0.05, 0.01, 1.0);                                      \n"
    "}                                                                                      \n"
};

/* clang-format on */
class FurArray : public SpuArray {
public:
	const uint32_t c_hair_count = 96 * 1024;

	Mat4f u_worldview;
	Mat4f u_new_nodeworld;
	Mat4f u_old_nodeworld;
	Vec3f u_light_position;
	float u_time;
	uint32_t u_fur_tex;

	FurArray()
	{
		Attrs shader_attrs = {
		        {"vert", c_fur_vert},
		        {"geom", c_fur_geom},
		        {"frag", c_fur_frag},
		};

		Attrs unif_attrs = {
		        {"u_worldview",      &u_worldview     },
		        {"u_new_nodeworld",  &u_new_nodeworld },
		        {"u_old_nodeworld",  &u_old_nodeworld },
		        {"u_light_position", &u_light_position},
		        {"u_time",           &u_time          },
		        {"u_fur_tex",        &u_fur_tex       },
		};
		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		RandomGenerator<float> frand;

		std::vector<vec3f_t> pos(c_hair_count);
		std::vector<vec3f_t> nms(c_hair_count);
		std::vector<vec2f_t> tcs(c_hair_count);

		for (auto i = 0u; i < c_hair_count; i++) {
			auto u = frand();
			auto z = frand();

			auto v = z + powf(sin(math::two_pi() * z) / (2.0 * math::two_pi()), 3);

			auto phi = u * math::two_pi();
			auto rho = v * math::two_pi();

			pos[i].x = cos(phi) * (0.5 + 0.5 * (1.0 + cos(rho)));
			pos[i].y = sin(rho) * 0.5;
			pos[i].z = sin(phi) * (0.5 + 0.5 * (1.0 + cos(rho)));

			nms[i].x = cos(phi) * cos(rho);
			nms[i].y = sin(rho);
			nms[i].z = sin(phi) * cos(rho);

			tcs[i].x = u * 4.0;
			tcs[i].y = v * 2.0;
		}

		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"data",         pos.data()   },
			        {"nelem",        c_hair_count },
			        {"a.a_position", 3            },
			};
			SpuArray::aux(attrs, 0);
		}
		{
			Attrs attrs = {
			        {"data",       nms.data()  },
			        {"nelem",      c_hair_count},
			        {"a.a_normal", 3           },
			};
			SpuArray::aux(attrs, 1);
		}
		{
			Attrs attrs = {
			        {"data",         tcs.data()  },
			        {"nelem",        c_hair_count},
			        {"a.a_texcoord", 2           },
			};
			SpuArray::aux(attrs, 2);
		}
	}

	void draw()
	{
		m_shader.use();
		SpuArray::draw(GL_POINTS, 0, c_hair_count);
	}

private:
	SpuShader m_shader;
};

class TorusArray : public shapes::Array {
public:
	Mat4f u_worldview;
	Mat4f u_nodeworld;

	TorusArray()
	{
		Attrs shader_attrs = {
		        {"vert", c_torus_vert},
		        {"frag", c_torus_frag},
		};

		Attrs unif_attrs = {
		        {"u_worldview", &u_worldview},
		        {"u_nodeworld", &u_nodeworld},
		};

		Array::initShader(shader_attrs, unif_attrs);
		Array::initArray(shapes::Torus(), {"position"});
	}
};

class App : public SpuPage {
public:
	FurArray m_furArray;
	TorusArray m_torusArray;
	double m_accel = 0.0;
	double m_prevVel = 0.0;
	double m_currVel = 0.0;

	App(const char *name) : SpuPage(name, true, {0.5, 0.5, 0.4, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_furArray.u_fur_tex = spu_inventory_new("texture", "assets/textures/zebra_fur.png", Attrs());
		m_furArray.u_light_position = {5.0, 6.0, 4.0};

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.line_width = 2.0;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto delta = getSeconds().delta() * 0.01;

		auto viewscreen = math::perspective(viewport(0), 60, 1, 30);
		auto worldview = Mat4f::orbiting(ezero(), esec, 5.5, 0, 0, 0, 21, 0, 80, 15);

		switch (int(0.25 * esec) % 8) {
		case 0: m_accel += delta; break;
		case 4: m_accel -= delta; break;
		}
		m_currVel += m_accel;

		auto curr_model = math::unit().rot("z", -m_currVel / 4.0 * math::two_pi());
		auto prev_model = math::unit().rot("z", -m_prevVel / 4.0 * math::two_pi());

		m_prevVel += (m_currVel - m_prevVel) * 0.5;

		m_torusArray.u_nodeworld = curr_model;
		m_torusArray.u_worldview = viewscreen * worldview;
		m_torusArray.draw(nullptr);

		m_furArray.u_old_nodeworld = prev_model;
		m_furArray.u_new_nodeworld = curr_model;
		m_furArray.u_worldview = viewscreen * worldview;
		m_furArray.u_time = esec;
		m_furArray.draw();
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("026_furry_torus");
}  // namespace
}  // namespace spu::oglplus
