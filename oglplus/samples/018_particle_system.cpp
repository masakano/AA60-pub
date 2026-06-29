//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in float a_age;                                                                        \n"
    "out float g_age;                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_worldview * u_nodeworld * vec4(a_position.xyz,1);               \n"
    "       g_age = a_age;                                                                  \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "layout(points) in;                                                                     \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "in float g_age[];                                                                      \n"
    "out float f_age;                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float s = 0.5;                                                                  \n"
    "       float yo[2] = float[2](-1.0, 1.0);                                              \n"
    "       float xo[2] = float[2](-1.0, 1.0);                                              \n"
    "       for (int j=0;j!=2;++j)                                                          \n"
    "       for (int i=0;i!=2;++i)                                                          \n"
    "       {                                                                               \n"
    "               float xoffs = xo[i]*(1.0+g_age[0])*s;                                   \n"
    "               float yoffs = yo[j]*(1.0+g_age[0])*s;                                   \n"
    "               gl_Position = u_viewsceen * vec4(                                       \n"
    "                       gl_in[0].gl_Position.x-xoffs,                                   \n"
    "                       gl_in[0].gl_Position.y-yoffs,                                   \n"
    "                       gl_in[0].gl_Position.z,                                         \n"
    "                       1.0                                                             \n"
    "               );                                                                      \n"
    "               f_age = g_age[0];                                                       \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in float f_age;                                                                        \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 u_color1 = vec3(1.0, 0.5, 0.5);                                            \n"
    "       vec3 u_color2 = vec3(0.3, 0.1, 0.1);                                            \n"
    "       final_color = vec4(                                                             \n"
    "               mix(u_color1, u_color2, f_age),                                         \n"
    "               1.0 - f_age                                                             \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	static constexpr auto c_particle_count = 100u;

	SpuShader m_shader;
	SpuArray m_array;

	std::vector<Vec4f> m_positions;
	std::vector<Vec4f> m_directions;
	std::vector<float> m_ages;
	// uint32_t m_textureId;

	double m_prevTime = 0.0;
	double m_prevSpawn = 0.0;
	;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;

	App(const char *name) : SpuPage(name, true, {0.9, 0.9, 0.9, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
                        {"vert", c_vert},
                        {"geom", c_geom}
                };

		Attrs unif_attrs = {
		        {"u_viewsceen", &u_viewsceen},
		        {"u_worldview", &u_worldview},
		        {"u_nodeworld", &u_nodeworld},
		};

		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		Attrs vert_attrs = {
		        {"shader_id",    m_shader.id()},
		        {"a.a_position", 4            },
		};

		m_array.init(vert_attrs);

		Attrs age_attrs = {
		        {"a.a_age", 1},
		};
		m_array.aux(age_attrs, 1);

		m_positions.reserve(c_particle_count);
		m_directions.reserve(c_particle_count);
		m_ages.reserve(c_particle_count);

		u_nodeworld = math::unit().trans({0.0, -10.0, 0.0});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.flags.blend = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
	}

	Vec4f newDirection()
	{
		auto disp = 2.0f;
		auto dx = (0.5f - frand()) * disp;
		auto dy = 5.0f + (0.5f - frand()) * disp;
		auto dz = (0.5f - frand()) * disp;
		return {dx, dy, dz, 0.0f};
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 40);

		auto i = 0u;
		auto time_diff = (esec - m_prevTime);
		auto age_mult = 0.2;
		while (i != m_positions.size()) {
			auto drag = 0.1 * (time_diff);
			if ((m_ages[i] += time_diff * age_mult) < 1.0) {
				m_directions[i] *= (1.0 - drag);
				m_positions[i] += m_directions[i] * time_diff;
			}
			else {
				m_ages[i] = 0.0;
				m_directions[i] = newDirection();
				m_positions[i] = Vec4f(0, 0, 0, 1);
			}
			++i;
		}
		// if there are not enough particles yet
		if (i != c_particle_count) {
			auto spawn_interval = 1.0 / (age_mult * c_particle_count);
			if (m_prevSpawn + spawn_interval < esec) {
				m_directions.push_back(newDirection());
				m_positions.emplace_back(0);
				m_ages.push_back(0.0);
				m_prevSpawn = esec;
			}
		}
		m_prevTime = esec;

		assert(m_positions.size() == m_directions.size());
		assert(m_positions.size() == m_ages.size());

		if (m_positions.empty()) {
			return;
		}
		if (m_ages.empty()) {
			return;
		}

		m_array.send(&m_positions[0], m_positions.size(), 0);
		m_array.send(&m_ages[0], m_ages.size(), 1);
		u_worldview = Mat4f::orbiting(ezero(), esec, 18.0, 0, 0, 0, 2.0, 45, 0, 0);
		m_shader.use();
		m_array.draw(GL_POINTS, 0, m_positions.size());
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("018_particle_system");
}  // namespace
}  // namespace spu::oglplus
