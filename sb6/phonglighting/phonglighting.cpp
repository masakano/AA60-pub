//
// App :
//
#include "base_app.h"
namespace spu::phonglighting {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;
	struct {
		uint32_t color;
		uint32_t normals;
	} textures;
	struct {
		Mat4f u_modelview;
		Mat4f u_worldview;
		Mat4f u_viewscreen;
	} ub_constants;
	sb6::Object m_object;
	bool m_perVertex = 0;
	SpuShader m_perFragmentShader;
	SpuShader m_perVertexShader;
	Vec3f u_albedo;
	Vec3f u_specular_albedo;
	float u_specular_power;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs unif_attrs = {
		        {"diffuse_albedo",  &u_albedo         },
		        {"specular_albedo", &u_specular_albedo},
		        {"specular_power",  &u_specular_power },
		        {"constants",       &ub_constants     },
		};
		loadShader(m_perFragmentShader, "phonglighting/per-fragment-phong.us", Attrs(), unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"diffuse_albedo",  &u_albedo         },
		        {"specular_albedo", &u_specular_albedo},
		        {"specular_power",  &u_specular_power },
		        {"constants",       &ub_constants     }, // shoud be shared!!
		};
		loadShader(m_perVertexShader, "phonglighting/per-vertex-phong.us", Attrs(), unif_attrs);
	}
	// model
	{
		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};

		m_object.load("sphere.sbm", Attrs(), sym, m_perFragmentShader.id());
	}

	// environment
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.cull_face = true;
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		spu_frame_set(-1, "bgcolor0", c_gray);
	}
}
void App::menu() { ImGui::Checkbox("per vertex", &m_perVertex); }

void App::render()
{
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	composition.lookat(Vec3f(0.0, 0.0, 20.0), ezero(), ey());
	auto worldview = composition.worldview();
	auto viewscreen = composition.viewscreen();

	for (auto j = 0; j < 7; j++) {
		for (auto i = 0; i < 7; i++) {
			auto model_matrix = c_unit.trans({i * 2.75f - 8.25f, 6.75f - float(j) * 2.25f, 0.0f});
			ub_constants.u_modelview = worldview * model_matrix;
			ub_constants.u_worldview = worldview;
			ub_constants.u_viewscreen = viewscreen;
			u_albedo = {0.5, 0.2, 0.7};
			u_specular_albedo = Vec3f(i / 9.0 + 1.0 / 9.0);
			u_specular_power = powf(2.0, float(j) + 2.0);
			m_perVertex ? m_perVertexShader.use() : m_perFragmentShader.use();
			m_object.draw();
		}
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("phonglighting");
}  // namespace spu::phonglighting
