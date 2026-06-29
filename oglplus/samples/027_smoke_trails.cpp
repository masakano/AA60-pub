//
// ParticleSystem :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/cloud.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in float a_age;                                                                        \n"
    "in int a_id;                                                                           \n"
    "out float g_age;                                                                       \n"
    "out int g_id;                                                                          \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
        //"     gl_Position = u_worldview * a_position;"
    "       gl_Position = u_worldview * vec4(a_position.xyz, 1);                            \n"
    "       g_age = a_age;                                                                  \n"
    "       g_id = a_id;                                                                    \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "layout(points) in;                                                                     \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "uniform vec3 u_light_cam_pos;                                                          \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "in float g_age[];                                                                      \n"
    "in int g_id[];                                                                         \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out float f_age;                                                                       \n"
    "out float f_light_value;                                                               \n"
    "out float f_light_bias;                                                                \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       if (g_age[0] > 1.0) return;                                                     \n"
    "       vec3 pos = gl_in[0].gl_Position.xyz;                                            \n"
    "       vec3 light_dir = normalize(u_light_cam_pos - pos);                              \n"
    "       float s = 0.8, g = 3.0;                                                         \n"
    "       float yo[2] = float[2](-1.0, 1.0);                                              \n"
    "       float xo[2] = float[2](-1.0, 1.0);                                              \n"
    "       float angle = g_id[0];                                                          \n"
    "       float cx = cos(angle), sx = sin(angle);                                         \n"
    "       mat2 rot = mat2(cx, sx, -sx, cx);                                               \n"
    "       for (int j=0;j!=2;++j)                                                          \n"
    "       for (int i=0;i!=2;++i)                                                          \n"
    "       {                                                                               \n"
    "               float xoffs = xo[i]*(1.0+g_age[0]*g)*s;                                 \n"
    "               float yoffs = yo[j]*(1.0+g_age[0]*g)*s;                                 \n"
    "               vec2 offs = rot*vec2(xoffs, yoffs);                                     \n"
    "               gl_Position = u_viewsceen * vec4(                                       \n"
    "                       pos.x-offs.x,                                                   \n"
    "                       pos.y-offs.y,                                                   \n"
    "                       pos.z,                                                          \n"
    "                       1.0                                                             \n"
    "               );                                                                      \n"
    "               f_texcoord = vec2(float(i), float(j));                                  \n"
    "               f_age = g_age[0];                                                       \n"
    "               f_light_value = light_dir.z;                                            \n"
    "               f_light_bias = -dot(                                                    \n"
    "                       normalize(vec3(offs, 0.0)),                                     \n"
    "                       light_dir                                                       \n"
    "               );                                                                      \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_texture;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "in float f_age;                                                                        \n"
    "in float f_light_value;                                                                \n"
    "in float f_light_bias;                                                                 \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 c = texture(u_texture, f_texcoord).rgb;                                    \n"
    "       float depth = c.g - c.r;                                                        \n"
    "       if (depth == 0.0) discard;                                                      \n"
    "       float density = min(depth * c.b * 2.0, 1.0);                                    \n"
    "       float intensity = min(                                                          \n"
    "               max(                                                                    \n"
    "                       f_light_value*0.5+                                              \n"
    "                       f_light_bias,                                                   \n"
    "                       0.0                                                             \n"
    "               )+max(                                                                  \n"
    "                       -f_light_value*                                                 \n"
    "                       (1.0 - density)*                                                \n"
    "                       f_light_bias * 5.0,                                             \n"
    "                       0.0                                                             \n"
    "               ),                                                                      \n"
    "               1.0                                                                     \n"
    "       ) + 0.1;                                                                        \n"
    "       final_color = vec4(                                                             \n"
    "               intensity,                                                              \n"
    "               intensity,                                                              \n"
    "               intensity,                                                              \n"
    "               (1.0 - f_age)*density                                                   \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class ParticleSystem {
public:
	void init(const std::initializer_list<Vec3f> &path_points, double path_time, double part_per_sec)
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
		assert(m_positions.size() == m_ids.size());

		auto time_diff = dsec;
		auto drag = 0.1 * time_diff;
		if (drag > 1.0) {
			drag = 1.0;
		}

		// go through the existing particles
		for (size_t i = 0, n = m_positions.size(); i != n; ++i) {
			// update the age
			m_ages[i] += time_diff / m_lifetime;
			// if the particle is "too old"
			if (m_ages[i] > 1.0) {
				// try to m_spawn a new one in its place
				spawnParticle(esec, m_positions[i], m_directions[i], m_ages[i], m_ids[i]);
			}
			else {
				// otherwise just update its motion
				m_directions[i] *= (1.0 - drag);
				m_positions[i] += m_directions[i] * time_diff;
			}
		}
		Vec3f position;
		Vec3f direction;
		float age;
		int32_t id;
		// spawn new particles if necessary
		while (spawnParticle(esec, position, direction, age, id)) {
			m_positions.push_back(position);
			m_directions.push_back(direction);
			m_ages.push_back(age);
			m_ids.push_back(id);
		}
	}

	void upload(std::vector<Vec3f> &pos, std::vector<float> &age, std::vector<int32_t> &id)
	{
		pos.insert(end(pos), begin(m_positions), end(m_positions));
		age.insert(end(age), begin(m_ages), end(m_ages));
		id.insert(end(id), begin(m_ids), end(m_ids));
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
	std::vector<int32_t> m_ids;

	Vec3f newDirection()
	{
		RandomGenerator<float> frand = {-0.1, 1.1};

		float dx = frand();
		float dy = frand();
		float dz = frand();
		return {dx, dy, dz};
	}

	int32_t newId() { return rand(); }

	bool spawnParticle(double time, Vec3f &position, Vec3f &direction, float &age, int32_t &id)
	{
		float new_age = time - m_spawnTime - m_spawnInterval;
		if (new_age >= 0.0) {
			m_spawnTime += m_spawnInterval;
			direction = newDirection();
			Vec3f emitter_pos = m_path.position(m_spawnTime / m_cycleTime);
			position = emitter_pos + direction;
			age = new_age;
			id = newId();
			return true;
		}
		return false;
	}
};

class App : public SpuPage {
public:
	std::vector<ParticleSystem> m_emitters;

	std::vector<Vec3f> m_positions;
	std::vector<float> m_ages;
	std::vector<int32_t> m_ids;

	SpuShader m_shader;
	SpuArray m_array;
	SpuTexture m_texture;

	Mat4f u_viewsceen;
	Mat4f u_worldview;

	Vec3f u_light_cam_pos;
	uint32_t u_texture;

	App(const char *name) : SpuPage(name, true, {0.0, 0.1, 0.2, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_emitters.resize(2);
		m_emitters[0].init(
		        {
		                {-20.0, -10.0, 10.0 },
		                {20.0,  0.0,   -20.0},
		                {20.0,  10.0,  20.0 },
		                {-20.0, 0.0,   -10.0}
                },
		        15.0, 200.0);

		m_emitters[1].init(
		        {
		                {30.0,  0.0,   5.0 },
                                {-30.0, 0.0,   -5.0},
                                {-20.0, 20.0,  5.0 },
                                {20.0,  -10.0, -5.0}
                },
		        17.0, 200.0);

		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"geom", c_geom},
			        {"frag", c_frag},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen",     &u_viewsceen    },
			        {"u_worldview",     &u_worldview    },
			        {"u_light_cam_pos", &u_light_cam_pos},
			        {"u_texture",       &u_texture      },
			};

			shapes::loadShader(m_shader, shader_attrs, unif_attrs);
		}

		{
			Attrs pos_attrs = {
			        {"shader_id",    m_shader.id()     },
			        {"a.a_position", 4                 },
			        {"nelem",        m_positions.size()},
			        {"data",         m_positions.data()},
			};
			m_array.init(pos_attrs);
		}
		{
			Attrs age_attrs = {
			        {"a.a_age", 1            },
			        {"nelem",   m_ages.size()},
			        {"data",    m_ages.data()},
			};
			m_array.aux(age_attrs, 1);
		}
		{
			Attrs id_attrs = {
			        {"format",  GL_INT      },
                                {"oformat", GL_INT      },
                                {"a.a_id",  1           },
			        {"nelem",   m_ids.size()},
                                {"data",    m_ids.data()},
			};
			m_array.aux(id_attrs, 2);
		}

		{
			images::Image image
			        = images::Cloud2D(images::Cloud(128, 128, 128, Vec3f(0.1, -0.5, 0.3), 0.5));

			Vec4f border_color = {0, 0, 0, 0};

			Attrs tex_attrs = {
			        {"target",     GL_TEXTURE_2D          },
			        {"iformat",    GL_RGB8                },
			        {"width",      image.width()          },
			        {"height",     image.height()         },
			        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter", GL_LINEAR              },
			        {"wrap_s",     GL_CLAMP_TO_BORDER     },
			        {"wrap_t",     GL_CLAMP_TO_BORDER     },
			        {"border",     border_color           },
			        {"data",       image.data()           },
			};
			m_texture.init(tex_attrs);
			u_texture = m_texture.id();
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
		// renderstate.use();
	}

	void render() override
	{
		// reshape
		u_viewsceen = math::perspective(viewport(0), 75, 1, 100);

		m_positions.clear();
		m_ages.clear();
		m_ids.clear();

		// update the emitters and get the particle data
		for (auto i = std::begin(m_emitters), e = std::end(m_emitters); i != e; ++i) {
			i->update(getSeconds());
			i->upload(m_positions, m_ages, m_ids);
		}
		assert(m_positions.size() == m_ages.size());
		assert(m_positions.size() == m_ids.size());

		// make a camera matrix

		auto esec = getSeconds().current();
		auto worldview = Mat4f::orbiting(ezero(), esec, 30, 10, 6, 0, 10, 0, 60, 20);

		std::vector<float> depths(m_positions.size());
		std::vector<uint32_t> indices(m_positions.size());

		// calculate the depths of the particles
		for (uint32_t i = 0u, n = m_positions.size(); i != n; ++i) {
			depths[i] = (worldview * Vec4f(m_positions[i], 1.0)).z;
			indices[i] = i;
		}

		// sort the indices by the depths
		sort(std::begin(indices), std::end(indices),
		     [&depths](uint32_t i, uint32_t j) { return depths[i] < depths[j]; });

		u_light_cam_pos = (worldview * Vec3f(30.0, 30.0, 30.0));
		u_worldview = worldview;

		m_shader.use();

		m_array.send(&m_positions[0], m_positions.size(), 0);
		m_array.send(&m_ages[0], m_ages.size(), 1);
		m_array.send(&m_ids[0], m_ids.size(), 2);
		m_array.send(indices, -1, 4);

		m_array.draw(GL_POINTS);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("027_smoke_trails");
}  // namespace
}  // namespace spu::oglplus
