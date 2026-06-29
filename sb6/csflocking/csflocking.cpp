//
// App :
//
#include <ssys/random_generator.h>
#include "base_app.h"
namespace spu::csflocking {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;

private:
	const int32_t def_local_size = 256;
	const int32_t c_flock_size = 256 * 64;

	struct FlockMember {
		Vec4f position;
		Vec4f velocity;
	};
	SpuShader m_computeShader;
	SpuShader m_renderShader;
	SpuArray m_computeArrays[2];
	SpuArray m_renderArrays[2];
	Mat4f u_modelscreen;
	Vec4f u_goal;
	uint32_t m_index = 0;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs shader_attrs = {
		        {"def_local_size", def_local_size}
                };

		Attrs unif_attrs = {
		        {"goal", &u_goal}
                };
		loadShader(m_computeShader, "csflocking/flocking.us", shader_attrs, unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"u_modelscreen", &u_modelscreen}
                };
		loadShader(m_renderShader, "csflocking/render.us", Attrs(), unif_attrs);
	}
	// arrays
	{
		// paper airplane position + normal
		const Vec4f geometry[][2] = {
		        {{-5.0, 1.0, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}      },
		        {{-1.0, 1.5, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}      },
		        {{-1.0, 1.5, 7.0, 1.0}, {0.107, -0.859, 0.00, 0.0}},
		        {{0.0, 0.0, 0.0, 1.0},  {0.832, 0.554, 0.00, 0.0} },
		        {{0.0, 0.0, 10.0, 1.0}, {-0.59, -0.395, 0.00, 0.0}},
		        {{1.0, 1.5, 0.0, 1.0},  {-0.832, 0.554, 0.00, 0.0}},
		        {{1.0, 1.5, 7.0, 1.0},  {0.295, -0.196, 0.00, 0.0}},
		        {{5.0, 1.0, 0.0, 1.0},  {0.124, 0.992, 0.00, 0.0} },
		};
		for (auto &array: m_computeArrays) {
			auto nelem = c_flock_size * sizeof(FlockMember);
			Attrs attrs0 = {
			        {"a.+0",  0    }, // shader storage
			        {"nelem", nelem},
			};
			array = SpuArray(attrs0);
			Attrs attrs1 = {
			        {"a.+1",  0    }, // shader storage
			        {"nelem", nelem},
			};
			array.aux(attrs1, 1);
		}
		m_computeArrays[1].link(0, m_computeArrays[0], 1);
		m_computeArrays[1].link(1, m_computeArrays[0], 0);
		for (auto &array: m_renderArrays) {
			Attrs attrs0 = {
			        {"shader_id",    m_renderShader.id()},
			        {"a.a_position", 4                  }, // position
			        {"a.a_normal",   4                  }, // normal
			        {"nelem",        8                  },
			        {"data",         &geometry          },
			};
			array = SpuArray(attrs0);
			Attrs attrs1 = {
			        {"divisor",           1},
			        {"a.a_bird_position", 4}, // position
			        {"a.a_bird_velocity", 4}, // velocity
			};
			array.aux(attrs1, 1);
			array.link(1, m_computeArrays[0], 1);
		}
	}
	// initial data
	{
		RandomGenerator<float> frand;
		auto *ptr = m_computeArrays[0].map<FlockMember *>("w", 0);
		for (auto i = 0; i < c_flock_size; i++) {
			auto r0 = Vec4f(frand(), frand(), frand(), frand());
			auto r1 = Vec4f(frand(), frand(), frand(), frand());
			ptr[i].position = (r0 - 0.5) * 300.0;
			ptr[i].velocity = (r1 - 0.5);
		}
		m_computeArrays[0].unmap(0);
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		// renderstate.use();
	}
}

void App::render()
{
	// update
	{
		u_goal = {
		        sinf(getSeconds().current() * 0.34),
		        cosf(getSeconds().current() * 0.29),
		        sinf(getSeconds().current() * 0.12) * cosf(getSeconds().current() * 0.5),
		        1.0,
		};
		u_goal = u_goal * Vec4f(35.0, 25.0, 60.0, 1.0);
		m_computeShader.use();
		m_computeArrays[m_index].draw(0xffff, c_flock_size / def_local_size, 1, 1);
	}
	// render
	sb6::Composition composition;
	composition.lookat(Vec3f(0.0, 0.0, -400.0), ezero(), ey());
	composition.perspective(viewport(0), 60.0, 0.1, 3000.0);
	u_modelscreen = composition.worldscreen();
	m_renderShader.use();
	m_renderArrays[m_index].draw(GL_TRIANGLE_STRIP, 0, 8, c_flock_size);
	m_index ^= 1;
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("csflocking");
}  // namespace spu::csflocking
