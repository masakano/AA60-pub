//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/sphere.hpp>

namespace spu::oglplus {
namespace {

constexpr auto c_particle_count = 30u;
#define def_particle_count "30"

/* clang-format off */
const char *c_comp =  {
    "#version 440										                                                                \n"
    "layout(local_size_x = " def_particle_count", local_size_y = 1, local_size_z = 1) in;	 \n"
    "#define ParticleCount " def_particle_count "						                                    \n"
    "uniform float u_time;                                                                 \n"
    "uniform float u_interval;                                                             \n"
    "const float u_mass = 1.0;									                                                    \n"
    "layout(std140) uniform u_particle_force_block {						                                 \n"
    "       vec4 forces[ParticleCount];								                                            \n"
    "};												                                                                        \n"
    "layout(std140) uniform u_particle_position_block {						                              \n"
    "       vec4 positions[ParticleCount];							                                          \n"
    "};												                                                                        \n"
    "readonly buffer a_position { vec4 v[]; } b_position;					                             \n"
    "readonly buffer a_velocity { vec4 v[]; } b_velocity;					                             \n"
    "writeonly buffer a_xfb_position { vec4 v[]; } b_xfb_position;				                     \n"
    "writeonly buffer a_xfb_velocity { vec4 v[]; } b_xfb_velocity;				                     \n"
    "												                                                                          \n"
    "vec3 in_position()										                                                          \n"
    "{												                                                                         \n"
    "       uint index = gl_GlobalInvocationID.x;						                                    \n"
    "       return b_position.v[index].xyz;							                                         \n"
    "}												                                                                         \n"
    "												                                                                          \n"
    "vec3 in_velocity()										                                                          \n"
    "{												                                                                         \n"
    "       uint index = gl_GlobalInvocationID.x;						                                    \n"
    "       return b_velocity.v[index].xyz;							                                         \n"
    "}												                                                                         \n"
    "												                                                                          \n"
    "vec3 anchor_spring(vec3 v, float l, float k)						                                    \n"
    "{												                                                                         \n"
    "       float ds = (l - length(v));								                                            \n"
    "       return k * sign(ds) * min(abs(ds), l) * normalize(v);				                      \n"
    "}												                                                                         \n"
    "vec3 nuclear_force(vec3 v, float c, float ci)						                                   \n"
    "{												                                                                         \n"
    "       float coef = c*ci;									                                                    \n"
    "       if (coef > 0.0)									                                                       \n"
    "       {											                                                                   \n"
    "               float d = length(v);							                                            \n"
    "               float e = 1.3;								                                                 \n"
    "               if (d <= 1.0) return 20.0*v/log(d);						                              \n"
    "               if (d >= e) return 20.0*v*pow(d-e, 2.0)*exp(-d+e);				                 \n"
    "       }											                                                                   \n"
    "       return vec3(0.0, 0.0, 0.0);								                                            \n"
    "}												                                                                         \n"
    "vec3 electro_static_force(vec3 v, float c, float ci)					                             \n"
    "{												                                                                         \n"
    "       return -300.0*c*ci*normalize(v)/dot(v, v);						                               \n"
    "}												                                                                         \n"
    "vec3 pariticle_forces(vec3 p, vec3 pi, vec4 fc, vec4 fci)					                        \n"
    "{												                                                                         \n"
    "       vec3 v = pi - p;									                                                      \n"
    "       return nuclear_force(v, fc.z, fci.z)+						                                    \n"
    "               electro_static_force(v, fc.w, fci.w);					                             \n"
    "}												                                                                         \n"
    "vec3 Force()										                                                                \n"
    "{												                                                                         \n"
    "       uint index = gl_GlobalInvocationID.x;						                                    \n"
    "       vec3 position = in_position();							                                          \n"
    "												                                                                          \n"
    "       vec3 f = anchor_spring(								                                                \n"
    "               position,									                                                     \n"
    "               forces[index].x,								                                               \n"
    "               forces[index].y								                                                \n"
    "       );											                                                                  \n"
    "       for (uint i=0; i<index; ++i)							                                            \n"
    "       {											                                                                   \n"
    "               f += pariticle_forces(							                                          \n"
    "                       position,								                                              \n"
    "                       positions[i].xyz,							                                       \n"
    "                       forces[index],							                                          \n"
    "                       forces[i]								                                              \n"
    "               );										                                                           \n"
    "       }											                                                                   \n"
    "       for (uint i= index+1; i<ParticleCount; ++i)						                              \n"
    "       {											                                                                   \n"
    "               f += pariticle_forces(							                                          \n"
    "                       position,								                                              \n"
    "                       positions[i].xyz,							                                       \n"
    "                       forces[index],							                                          \n"
    "                       forces[i]								                                              \n"
    "               );										                                                           \n"
    "       }											                                                                   \n"
    "       return f;										                                                            \n"
    "}												                                                                         \n"
    "void main()										                                                                 \n"
    "{												                                                                         \n"
    "       uint index = gl_GlobalInvocationID.x;						                                    \n"
    "       float mass = index < 20? u_mass*1840: u_mass;					                             \n"
    "       vec3 velocity = in_velocity() + (Force()* u_interval) / u_mass;			             \n"
    "       b_xfb_velocity.v[index] = vec4(velocity, 0);					                              \n"
    "       b_xfb_position.v[index] = vec4(in_position() + velocity * u_interval, u_time);	\n"
    "}												                                                                         \n"
};

const char *c_particle_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec3 u_light_position;                                                         \n"
    "in vec3 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec3 a_offset;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "out vec3 f_normal_view;                                                                \n"
    "flat out int f_type;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(a_position+a_offset, 1.0);                                   \n"
    "       f_normal = a_normal;                                                            \n"
    "       f_normal_view = mat3(u_worldview) * a_normal;                                   \n"
    "       f_light = u_light_position - gl_Position.xyz;                                   \n"
    "       f_type = gl_InstanceID / 10;                                                    \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_particle_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "in vec3 f_normal_view;                                                                 \n"
    "flat in int f_type;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "bool neutron_sign()                                                                    \n"
    "{                                                                                      \n"
    "       return false;                                                                   \n"
    "}                                                                                      \n"
    "bool electron_sign()                                                                   \n"
    "{                                                                                      \n"
    "       return (                                                                        \n"
    "               abs(f_normal_view.x) < 0.5 &&                                           \n"
    "               abs(f_normal_view.y) < 0.2                                              \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
    "bool proton_sign()                                                                     \n"
    "{                                                                                      \n"
    "       return  electron_sign() || (                                                    \n"
    "               abs(f_normal_view.y) < 0.5 &&                                           \n"
    "               abs(f_normal_view.x) < 0.2                                              \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
    "vec3 neutron_color()                                                                   \n"
    "{                                                                                      \n"
    "       return vec3(0.5, 0.5, 0.5);                                                     \n"
    "}                                                                                      \n"
    "vec3 electron_color()                                                                  \n"
    "{                                                                                      \n"
    "       return vec3(0.0, 0.0, 1.0);                                                     \n"
    "}                                                                                      \n"
    "vec3 proton_color()                                                                    \n"
    "{                                                                                      \n"
    "       return vec3(1.0, 0.0, 0.0);                                                     \n"
    "}                                                                                      \n"
    "bool sign[3] = bool[3](                                                                \n"
    "       neutron_sign(),                                                                 \n"
    "       proton_sign(),                                                                  \n"
    "       electron_sign()                                                                 \n"
    ");                                                                                     \n"
    "vec3 color[3] = vec3[3](                                                               \n"
    "       neutron_color(),                                                                \n"
    "       proton_color(),                                                                 \n"
    "       electron_color()                                                                \n"
    ");                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float lighting = dot(                                                           \n"
    "               f_normal,                                                               \n"
    "               normalize(f_light)                                                      \n"
    "       );                                                                              \n"
    "       float intensity = clamp(                                                        \n"
    "               0.4 + lighting * 1.0,                                                   \n"
    "               0.0,                                                                    \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "       final_color = sign[f_type]?                                                     \n"
    "               vec4(1.0, 1.0, 1.0, 1.0):                                               \n"
    "               vec4(color[f_type] * intensity, 1.0);                                   \n"
    "}                                                                                      \n"
};

const char *c_trail_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform float u_time;                                                                  \n"
    "in vec4 a_position_and_time;                                                           \n"
    "out float f_age;                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec4 position = vec4(a_position_and_time.xyz, 1.0);                             \n"
    "       f_age = u_time - a_position_and_time.w;                                         \n"
    "       gl_Position = u_viewsceen * u_worldview * position;                             \n"
    "}                                                                                      \n"
};

const char *c_trail_frag =  {
    "#version 330                                                                           \n"
    "in float f_age;                                                                        \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float a = exp(-f_age);                                                          \n"
    "       final_color = vec4(a, a, a, a);                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Vec4f u_particle_force_block[c_particle_count];
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Vec3f u_light_position;
	float u_time;
	float u_interval;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen",            &u_viewsceen           },
		        {"u_worldview",            &u_worldview           },
		        {"u_light_position",       &u_light_position      },
		        {"u_time",                 &u_time                },
		        {"u_interval",             &u_interval            },
		        {"u_particle_force_block", &u_particle_force_block},
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class PhysicsSimulator {
public:
	SpuComputeArray m_compArray;

	SpuComputeArray &getArray() { return m_compArray; }

	explicit PhysicsSimulator(Uniforms &unifs)
	{
		// compute
		auto &array = m_compArray;
		auto &shader = array.getShader();

		Attrs shader_attrs = {
		        {"comp", c_comp},
		};
		shapes::loadShader(shader, shader_attrs, Attrs(unifs));

		Attrs array_attrs0 = {
		        {"shader_id",    shader.id()     },
		        {"a.a_position", 4               },
		        {"nelem",        c_particle_count},
		};
		Attrs array_attrs1 = {
		        {"a.a_velocity", 4               },
		        {"nelem",        c_particle_count},
		};
		Attrs array_attrs2 = {
		        {"a.a_xfb_position", 4               },
		        {"nelem",            c_particle_count},
		};
		Attrs array_attrs3 = {
		        {"a.a_xfb_velocity", 4               },
		        {"nelem",            c_particle_count},
		};

		array.aux(array_attrs0, 0);
		array.aux(array_attrs1, 1);
		array.aux(array_attrs2, 2);
		array.aux(array_attrs3, 3);

		int32_t buffer_id;
		array.get("0.buffer_id", &buffer_id);
		shader.set("u_particle_position_block.buffer_id", &buffer_id);

		auto neutron_orbital = 2.0f;
		auto proton_orbital = 1.8f;
		auto electron_orbital_1 = 30.0f;
		auto electron_orbital_2 = 45.0f;

		std::vector<Vec4f> positions(c_particle_count, ezero<Vec4f>());

		RandomGenerator<float> frand = {-0.5, 0.5};

		for (auto i = 0u; i < positions.size(); i++) {
			auto pos = Vec3f(frand(), frand(), frand());

			pos = normalize(pos) * (1.0f + frand() * 0.1f);
			if (i < 10) {
				pos = pos * neutron_orbital;
			}
			else if (i < 20) {
				pos = pos * proton_orbital;
			}
			else if (i < 22) {
				pos = pos * electron_orbital_1;
			}
			else if (i < 30) {
				pos = pos * electron_orbital_2;
			}
			positions[i] = Vec4f(pos, 1.0);
		}

		std::vector<Vec4f> velocities(c_particle_count, ezero<Vec4f>());
		for (auto i = 0u; i < velocities.size(); i++) {
			auto v = 0.5f;
			if (i < 10) {
				v *= neutron_orbital;
			}
			else if (i < 20) {
				v *= proton_orbital;
			}
			else if (i < 22) {
				v *= electron_orbital_1;
			}
			else if (i < 30) {
				v *= electron_orbital_2;
			}
			for (auto c = 0; c != 3; ++c) {
				velocities[i].f[c] = v * v * frand();
			}
		}

		std::vector<Vec4f> forces(c_particle_count, ezero<Vec4f>());
		// 0 : anchor spring length
		// 1 : anchor spring strength
		// 2 : nuclear
		// 3 : electrostatic charge

		// nuclear
		auto j = 0;
		for (auto i = 0u; i != 10; ++i) {
			forces[j].x = neutron_orbital;
			forces[j].y = 190.0;
			forces[j].z = 1.0;
			forces[j].w = 0.0;
			j++;
		}
		// protons
		for (auto i = 0u; i != 10; ++i) {
			forces[j].x = proton_orbital;
			forces[j].y = 300.0;
			forces[j].z = 1.0;
			forces[j].w = 1.0;
			j++;
		}
		// electrons (1)
		for (auto i = 0u; i != 2; ++i) {
			forces[j].x = electron_orbital_1;
			forces[j].y = 30.0;
			forces[j].z = 0.0;
			forces[j].w = -1.0;
			j++;
		}
		// electrons (2)
		for (auto i = 0u; i != 8; ++i) {
			forces[j].x = electron_orbital_2;
			forces[j].y = 25.0;
			forces[j].z = 0.0;
			forces[j].w = -1.0;
			j++;
		}

		memcpy(unifs.u_particle_force_block, forces.data(), sizeof(unifs.u_particle_force_block));
		m_compArray.send(positions.data(), c_particle_count, 0);
		m_compArray.send(velocities.data(), c_particle_count, 1);
	}

	void update(Uniforms &unifs, double time, double time_diff)
	{
		if (time_diff <= 0.0) {
			return;
		}
		const auto nsteps = 10 /*10*/;
		static_assert(nsteps % 2 == 0);

		unifs.u_time = time;
		unifs.u_interval = time_diff / nsteps;

		for (auto s = 0; s != nsteps; ++s) {
			m_compArray.compute();
			m_compArray.copy(0, m_compArray, 2);
			m_compArray.copy(1, m_compArray, 3);
		}
	}
};

class ParticleArray : public shapes::Array {
public:
	ParticleArray(const Uniforms &unifs, PhysicsSimulator &physics)
	{
		Attrs particle_shader_attrs = {
		        {"frag", c_particle_frag},
		        {"vert", c_particle_vert},
		};

		Array::initShader(particle_shader_attrs, Attrs(unifs));
		Array::initArray(shapes::Sphere(2.5, 18, 12), {"position", "normal"});

		uint32_t phys_dst_id;

		physics.getArray().get("0.buffer_id", &phys_dst_id);  // temporary
		Attrs attrs = {
		        {"buffer_id",  phys_dst_id},
		        {"divisor",    1          },
		        {"a.a_offset", 4          },
		};
		Array::aux(attrs, 2);
	}
};

class TrailArray : public SpuArray {
private:
	static constexpr auto c_electron_offset = size_t(20);
	static constexpr auto c_electron_count = size_t(10);
	static constexpr auto c_max_trail_points = size_t(128);

	uint32_t m_trailPoints = 0;
	uint32_t m_currTrailPoint = 0;

public:
	SpuShader m_shader;

	std::vector<Vec4f> m_vertices;
	std::vector<int16_t> m_indices;

	explicit TrailArray(const Uniforms &unifs)
	{
		Attrs shader_attrs = {
		        {"frag", c_trail_frag},
		        {"vert", c_trail_vert},
		};
		shapes::loadShader(m_shader, shader_attrs, Attrs(unifs));

		m_vertices.resize(c_electron_count * c_max_trail_points);
		m_indices.resize(c_electron_count * c_max_trail_points * 2);

		auto k = 0u;

		for (auto p = 1u; p != c_max_trail_points; ++p) {
			for (auto e = 0u; e != c_electron_count; ++e) {
				m_indices[k++] = e + c_electron_count * (p - 1);
				m_indices[k++] = e + c_electron_count * p;
			}
		}
		for (auto e = 0u; e != c_electron_count; ++e) {
			m_indices[k++] = e + c_electron_count * (c_max_trail_points - 1);
			m_indices[k++] = e;
		}
		assert(k == m_indices.size());

		Attrs attrs = {
		        {"shader_id",             m_shader.id()    },
		        {"a.a_position_and_time", 4                },
		        {"nelem",                 m_vertices.size()},
		        {"data",                  m_vertices.data()}, // clear for debug
		};

		SpuArray::init(attrs);
		SpuArray::send(m_indices, -1, sizeof(m_indices[0]));
	}

	void update(PhysicsSimulator &physics)
	{
		if (m_currTrailPoint >= c_max_trail_points) {
			m_currTrailPoint = 0;
		}

		copy(0,                                                        // dst slot
		     physics.getArray(),                                       // src array
		     0,                                                        // src slot
		     m_currTrailPoint * c_electron_count * 4 * sizeof(float),  // dst offset (byte)
		     c_electron_offset * 4 * sizeof(float),                    // src offset (byte)
		     c_electron_count * 4 * sizeof(float));                    // size (byte)

		if (m_trailPoints < c_max_trail_points) {
			++m_trailPoints;
		}
		++m_currTrailPoint;
	}

	void draw()
	{
		m_shader.use();
		if (m_currTrailPoint > 1) {
			SpuArray::draw(GL_LINES, 0, c_electron_count * (m_currTrailPoint - 1) * 2);
		}
		if (m_trailPoints >= c_max_trail_points) {
			SpuArray::draw(
			        GL_LINES, c_electron_count * m_currTrailPoint * 2,
			        c_electron_count * (c_max_trail_points - m_currTrailPoint) * 2);
		}
	}
};

class App : public SpuPage {
public:
	Uniforms m_unifs;
	PhysicsSimulator m_physics;
	ParticleArray m_particleArray;
	TrailArray m_trailArray;

	App(const char *name)
	        : SpuPage(name, true, {0.3, 0.3, 0.3, 0.0}), m_physics(m_unifs),
	          m_particleArray(m_unifs, m_physics), m_trailArray(m_unifs)
	{
	}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.line_width = 3.0;
		renderstate.flags.depth_test = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
		// renderstate.use();

		m_unifs.u_light_position = {80.0, 80.0, 80.0};
	}

	void render() override
	{
		// reshape
		auto esec = getSeconds().current();
		auto dsec = getSeconds().delta();
		auto worldview = Mat4f::orbiting(ezero(), esec, 205, 165, 21, 0, 15, 0, 45, 1);

		m_unifs.u_viewsceen = math::perspective(viewport(0), 60, 1, 1000);
		m_unifs.u_worldview = worldview;

		m_physics.update(m_unifs, esec, dsec);
		m_particleArray.setInstanceCount(c_particle_count);
		m_particleArray.draw(nullptr);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.blend = true;
		renderstate.write_mask.z = 0;
		renderstate.use();

		m_unifs.u_worldview = worldview;
		m_unifs.u_time = esec;

		m_trailArray.update(m_physics);
		m_trailArray.draw();

		renderstate.flags.blend = false;
		renderstate.write_mask.z = 1;
		renderstate.use();
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("031_neon");
}  // namespace
}  // namespace spu::oglplus
