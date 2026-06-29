//
// App :
//
#include "base_app.h"
namespace spu::rimlight {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;

	sb6::Object m_object;
	SpuShader m_shader;
	Vec3f m_color = {0.3, 0.3, 0.3};
	bool m_enable = 1;

	Mat4f u_modelview;
	Mat4f u_viewscreen;
	Vec3f u_light_position = Vec3f(100.0, 100.0, 100.0);

	Vec3f u_albedo = Vec3f(0.3, 0.5, 0.2);
	Vec3f u_specular_albedo = Vec3f(0.7);
	float u_specular_power = 128.0;
	Vec3f u_rim_color = Vec3f(0.1, 0.7, 0.2);
	float u_rim_power = 2.5;

};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// m_mat_rotation = c_unit;
	//  shader
	{
		Attrs unif_attrs = {
		        {"u_modelview",  &u_modelview },
		        {"u_viewscreen", &u_viewscreen},
		        {"u_rim_color",  &u_rim_color },
		        {"u_rim_power",  &u_rim_power },
		        {"u_light_position",  &u_light_position },
		        {"u_albedo",  &u_albedo },
		        {"u_specular_albedo",  &u_specular_albedo },
		        {"u_specular_power",  &u_specular_power },
		};
		loadShader(m_shader, "rimlight/render.us", Attrs(), unif_attrs);
	}
	// model
	{
		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};
		m_object.load("dragon.sbm", Attrs(), sym, m_shader.id());
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.cull_face = true;
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		// renderstate.use();
	}
}

void App::menu()
{
	ImGui::Checkbox("rim enable", &m_enable);
	if (m_enable) {
		ImGui::SliderFloat3("rim color", m_color.f, 0.0, 4.0);
		ImGui::SliderFloat("rim power", &u_rim_power, 0.1, 5.0);
	}
}

void App::render()
{
	auto f = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	u_modelview = c_unit.rot("Y", -f * 5.0).trans({0.0, -5.0, -20.0});
	u_rim_color = m_enable ? m_color : ezero();
	m_shader.use();
	m_object.draw();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("rimlight");
}  // namespace spu::rimlight
