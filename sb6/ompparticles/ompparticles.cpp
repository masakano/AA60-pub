//
// App :
//
#include "base_app.h"
#include <omp.h>
namespace spu::ompparticles {
/* clang-format off */
const char* vert = {
    "#version 440 core                                                                      \n"
    "layout (location = 0) in vec3 position;                                                \n"
    "out vec4 particle_color;                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    particle_color = vec4(0.6, 0.8, 0.8, 1.0) *                                        \n"
    "            (smoothstep(-10.0, 10.0, position.z) * 0.6 + 0.4);                         \n"
    "    gl_Position = vec4(position * 0.2, 1.0);                                           \n"
    "}                                                                                      \n"
};

const char* frag = {
    "#version 440 core                                                                      \n"
    "layout (location = 0) out vec4 o_color;                                                \n"
    "in vec4 particle_color;                                                                \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    o_color = particle_color;                                                          \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char* name) : BaseApp(name) {}
	void init(const Attrs& attrs) override;
	void render() override;
	void shutdown();
	enum { e_particle_count = 2048 };
	struct Particle {
		Vec3f position;
		Vec3f velocity;
	};

protected:
	std::vector<Particle> m_particles[2];
	int32_t m_index = 0;
	uint32_t m_arrayId;
	uint32_t m_shaderId;

	void iniitializeParticles();
	void updateParticles(float delta_time);
	void onKey(int) {}
};

void App::init(const Attrs& attrs)
{
	BaseApp::init(attrs);

	m_particles[0].resize(e_particle_count);
	m_particles[1].resize(e_particle_count);

	// Create GPU buffer
	Attrs array_attrs = {
	        {"a.0",   4               },
	        {"a.1",   4               },
	        {"nelem", e_particle_count},
	};
	m_arrayId = spu_array_new(array_attrs);
	iniitializeParticles();
	Attrs shader_attrs = {
	        {"frag", frag},
	        {"vert", vert},
	};
	m_shaderId = spu_shader_new(shader_attrs);

	auto max_threads = omp_get_max_threads();

	// printf("max_threads = %d\n", max_threads);
	omp_set_num_threads(max_threads);
	auto& renderstate = getRenderstate();
	renderstate.point_size = 8.0;
	// renderstate.use();
}

// Random number generator
static auto seed = 0x13371337u;
static inline float random_float()
{
	union {
		float f;
		uint32_t ui;
	} res;

	seed *= 16807;
	auto tmp = seed ^ (seed >> 4) ^ (seed << 15);
	res.ui = (tmp >> 9) | 0x3F800000;
	return (res.f - 1.0f);
}

void App::iniitializeParticles()
{
	for (auto i = 0; i < e_particle_count; i++) {
		m_particles[0][i].position.f[0] = random_float() * 6.0f - 3.0f;
		m_particles[0][i].position.f[1] = random_float() * 6.0f - 3.0f;
		m_particles[0][i].position.f[2] = random_float() * 6.0f - 3.0f;
		m_particles[0][i].velocity = m_particles[0][i].position * 0.001f;
	}
	spu_array_send(m_arrayId, m_particles[0].data(), m_particles[0].size(), 0);
}

void App::updateParticles(float delta_time)
{
	// Double buffer source and destination
	auto src_index = m_index & 1;
	auto dst_index = 1 - src_index;
	const auto* const __restrict src = m_particles[src_index].data();
	auto* const __restrict dst = m_particles[dst_index].data();

	// For each particle in the system
	// #pragma omp parallel for schedule(dynamic, 16)
	for (auto i = 0; i < e_particle_count; i++) {
		// Get my own data
		const auto& me = src[i];
		auto delta_v = ezero();

		// For all the other particles
		for (auto j = 0; j < e_particle_count; j++) {
			if (i != j)  // ... not me!
			{
				//  Get the vector to the other particle
				auto delta_pos = src[j].position - me.position;
				auto distance = spu::distance(src[j].position, me.position);
				auto delta_dir = delta_pos / distance;
				// This clamp stops the system from blowing up if particles get
				// too close...
				distance = distance < 0.005f ? 0.005f : distance;
				// Update velocity
				delta_v += (delta_dir / (distance * distance));
			}
		}
		// Add my current velocity to my position.
		dst[i].position = me.position + me.velocity;
		// Produce new velocity from my current velocity plus the calculated delta
		dst[i].velocity = me.velocity + delta_v * delta_time * 0.01f;
	}
	spu_array_send(m_arrayId, m_particles[dst_index].data(), m_particles[dst_index].size(), 0);
	m_index++;
}

void App::render()
{
	static auto previous_time = 0.0f;

	auto current_time = getSeconds().current();
	auto delta_time = current_time - previous_time;

	previous_time = current_time;
	updateParticles(delta_time * 0.001f);
	spu_shader_use(m_shaderId);
	spu_array_draw(m_arrayId, GL_POINTS);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("ompparticles");
}  // namespace spu::ompparticles
