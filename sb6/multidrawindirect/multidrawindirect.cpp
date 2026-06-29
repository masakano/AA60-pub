//
// ArrayCommand :
//
#include "base_app.h"
namespace spu::multidrawindirect {

enum { e_num_draws = 3000 };

struct ArrayCommand {
	uint32_t count;
	uint32_t instance_count;
	uint32_t base_vertex;
	uint32_t base_instance;
};

class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_shader;
	float u_time;
	Mat4f u_worldview;
	Mat4f u_viewscreen;
	Mat4f u_worldscreen;
	sb6::Object m_object;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs unif_attrs = {
		        {"time",          &u_time       },
		        {"u_worldview",   &u_worldview  },
		        {"u_viewscreen",  &u_viewscreen },
		        {"u_worldscreen", &u_worldscreen},
		};
		loadShader(m_shader, "multidrawindirect/render.us", Attrs(), unif_attrs);
	}
	// model
	{
		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};
		m_object.load("asteroids.sbm", Attrs(), sym, m_shader.id());
	}
	// array
	{
		auto array_id = m_object.getArray().id();
		ArrayCommand cmd[e_num_draws];
		const auto nprim = 4;
		for (auto i = 0; i < e_num_draws; i++) {
			m_object.getModelInfo(i % m_object.modelCount(), cmd[i].base_vertex, cmd[i].count);
			cmd[i].instance_count = nprim;
			cmd[i].base_instance = i;
		}
		spu_array_send(array_id, cmd, e_num_draws, -2);
		Attrs attr10 = {
		        {"format",      GL_INT},
		        {"oformat",     GL_INT},
		        {"divisor",     1     },
		        {"a.a_draw_id", 1     },
		};
		spu_array_aux(array_id, attr10, 10);
		uint32_t draw_index[e_num_draws];
		for (auto i = 0; i < e_num_draws; i++) {
			draw_index[i] = i * nprim;
		}
		spu_array_send(array_id, draw_index, e_num_draws, 10);
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		renderstate.flags.cull_face = true;
		// renderstate.use();
	}
}

void App::render()
{
	auto t = getSeconds().current();
	u_time = t;
	sb6::Composition composition;
	composition.lookat(
	        Vec3f(100.0 * cosf(t * 0.023), 100.0 * cosf(t * 0.023), 300.0 * sinf(t * 0.037) - 600.0),
	        Vec3f(0.0, 0.0, 260.0), normalize(Vec3f(0.1 - cosf(t * 0.1) * 0.3, 1.0, 0.0)));
	composition.perspective(viewport(0), 50.0, 1.0, 2000.0);
	u_worldview = composition.worldview();
	u_viewscreen = composition.viewscreen();
	u_worldscreen = composition.worldscreen();

	m_shader.use();
	m_object.draw(e_num_draws);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("multidrawindirect");
}  // namespace spu::multidrawindirect
