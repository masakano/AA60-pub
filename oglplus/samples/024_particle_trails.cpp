//
// ParticleSystem :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in float a_age;                                                                        \n"
    "out float g_age;                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_worldview * vec4(a_position.xyz,1);                             \n"
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
    "       if (g_age[0] > 1.0) return;                                                     \n"
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
class ParticleSystem {
public:
	void init(const std::vector<Vec3f> &path_points, double path_time, double part_per_sec)
	{
		m_cycleTime = path_time;
		m_lifetime = 10.0;
		m_spawnInterval = 1.0 / part_per_sec;
		m_spawnTime = 0.0;
		m_path.init(path_points);
		assert(m_cycleTime > 0.0);
	}

	void update(const Seconds &seconds)
	{
		auto esec = seconds.current();
		auto dsec = seconds.delta();

		assert(m_positions.size() == m_directions.size());
		assert(m_positions.size() == m_ages.size());

		auto time_diff = dsec;
		auto drag = 0.1 * time_diff;
		if (drag > 1.0) {
			drag = 1.0;
		}

		for (uint32_t i = 0u, n = m_positions.size(); i != n; ++i) {
			m_ages[i] += time_diff / m_lifetime;
			if (m_ages[i] > 1.0) {
				spawnParticle(esec, m_positions[i], m_directions[i], m_ages[i]);
			}
			else {
				m_directions[i] *= (1.0 - drag);
				m_positions[i] += m_directions[i] * time_diff;
			}
		}
		Vec3f position;
		Vec3f direction;
		float age;

		while (spawnParticle(esec, position, direction, age)) {
			m_positions.push_back(position);
			m_directions.push_back(direction);
			m_ages.push_back(age);
		}
	}

	void upload(std::vector<Vec3f> &pos, std::vector<float> &age)
	{
		pos.insert(end(pos), begin(m_positions), end(m_positions));
		age.insert(end(age), begin(m_ages), end(m_ages));
	}

private:
	CubicBezierLoop<Vec3f, double> m_path;
	double m_cycleTime;
	double m_lifetime;
	double m_spawnInterval;
	double m_spawnTime;

	std::vector<Vec3f> m_positions;
	std::vector<Vec3f> m_directions;
	std::vector<float> m_ages;

	// Creates a directional vector for a new particle
	Vec3f newDirection()
	{
		RandomGenerator<float> frand = {0.1, 0.9};

		float dx = frand();
		float dy = frand();
		float dz = frand();

		return {dx, dy, dz};
	}

	// Spawns a new particle if the time is right
	bool spawnParticle(double time, Vec3f &position, Vec3f &direction, float &age)
	{
		float new_age = time - m_spawnTime - m_spawnInterval;
		if (new_age >= 0.0) {
			m_spawnTime += m_spawnInterval;
			direction = newDirection();
			Vec3f emitter_pos = m_path.position(m_spawnTime / m_cycleTime);
			position = emitter_pos + direction;
			age = new_age;
			return true;
		}
		return false;
	}
};

class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;

	std::vector<ParticleSystem> m_emitters;
	std::vector<Vec3f> m_positions;
	std::vector<float> m_ages;

	Mat4f u_viewsceen;
	Mat4f u_worldview;

	App(const char *name) : SpuPage(name, true, {0.9, 0.9, 0.9, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_emitters.resize(3);

		m_emitters[0].init(
		        {
		                {-20.0, -10.0, 10.0 },
		                {+20.0, +0.0,  -20.0},
		                {+20.0, +10.0, 20.0 },
		                {-20.0, +0.0,  -10.0},
                },
		        5.0, 200.0);

		m_emitters[1].init(
		        {
		                {+30.0, 0.0,   0.0},
		                {-30.0, 0.0,   0.0},
		                {-20.0, +20.0, 0.0},
		                {+20.0, -10.0, 0.0},
                },
		        3.0, 200.0);

		m_emitters[2].init(
		        {
		                {+5.0, +20.0, +20.0},
		                {-5.0, +20.0, -20.0},
		                {+5.0, -20.0, -20.0},
		                {-5.0, -20.0, +20.0},
                },
		        20.0, 100.0);

		Attrs shader_attrs = {
		        {"vert", c_vert},
		        {"geom", c_geom},
		        {"frag", c_frag},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen", &u_viewsceen},
		        {"u_worldview", &u_worldview},
		};
		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		Attrs pos_attrs = {
		        {"shader_id",    m_shader.id()},
		        {"a.a_position", 4            },
		};

		Attrs age_attrs = {
		        {"a.a_age", 1},
		};

		m_array.init(pos_attrs);
		m_array.aux(age_attrs, 1);

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
		// reshape
		u_viewsceen = math::perspective(viewport(0), 60, 1, 100);

		m_positions.clear();
		m_ages.clear();

		// update the emitters and get the particle data
		for (auto i = std::begin(m_emitters), e = std::end(m_emitters); i != e; ++i) {
			i->update(getSeconds());
			i->upload(m_positions, m_ages);
		}
		assert(m_positions.size() == m_ages.size());

		// make a camera matrix
		auto esec = getSeconds().current();
		u_worldview = Mat4f::orbiting(ezero(), esec, 38, -17, 6, 0, 10, 0, 60, 20);

		std::vector<float> depths(m_positions.size());
		std::vector<uint32_t> indices(m_positions.size());

		// calculate the depths of the particles
		for (uint32_t i = 0u, n = m_positions.size(); i != n; ++i) {
			depths[i] = (u_worldview
			             * Vec4f(m_positions[i].x, m_positions[i].y, m_positions[i].z, 1.0f))
			                    .z;
			indices[i] = i;
		}

		// sort the indices by the depths
		std::sort(std::begin(indices), std::end(indices), [&depths](uint32_t i, uint32_t j) {
			return depths[i] < depths[j];
		});

		m_shader.use();

		m_array.send(&m_positions[0], m_positions.size(), 0);
		m_array.send(&m_ages[0], m_ages.size(), 1);
		m_array.send(indices, -1, sizeof(indices[0]));
		m_array.draw(GL_POINTS, 0, indices.size());
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("024_particle_trails");
}  // namespace
}  // namespace spu::oglplus
