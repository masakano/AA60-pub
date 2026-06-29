//
// App :
//
#include "base_app.h"
namespace spu::envmapsphere {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;

	SpuShader m_shader;
	sb6::Object m_object;
	uint32_t m_envmaps[3];
	int32_t m_index = 2;

	Mat4f u_modelview;
	Mat4f u_viewscreen;
	uint32_t u_envmap_texture;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs unif_attrs = {
		        {"u_modelview",      &u_modelview     },
		        {"u_viewscreen",     &u_viewscreen    },
		        {"u_envmap_texture", &u_envmap_texture},
		};
		loadShader(m_shader, "envmapsphere/render.us", Attrs(), unif_attrs);
	}
	// envmap
	{
		m_envmaps[0] = sb6::ktx::load("envmaps/spheremap1.ktx");
		m_envmaps[1] = sb6::ktx::load("envmaps/spheremap2.ktx");
		m_envmaps[2] = sb6::ktx::load("envmaps/spheremap3.ktx");
		u_envmap_texture = m_envmaps[m_index];
	}
	// object
	{
		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};
		m_object.load("dragon.sbm", Attrs(), sym, m_shader.id());
	}
	// environment
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		spu_frame_set(-1, "bgcolor0", c_gray);
	}
}

void App::menu() { ImGui::Combo("envmaps", &m_index, "spheremap1\0spheremap2\0spheremap3\0\0"); }

void App::render()
{
	auto t = getSeconds().current();

	sb6::Composition composition;
	composition.perspective(viewport(0), 60.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	u_modelview
	        = c_unit.trans({0.0, -4.0, 0.0}).rot("YX", -t * 1.1 * 8.0, -t * 8.0).trans({0.0, 0.0, -15.0});
	u_envmap_texture = m_envmaps[m_index];
	m_shader.use();
	m_object.draw();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("envmapsphere");
}  // namespace spu::envmapsphere
