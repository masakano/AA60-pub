//
// App :
//
#include "base_app.h"
namespace spu::blinnphong {

class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	struct {
		Mat4f modelworld;
		Mat4f worldview;
		Mat4f viewscreen;
	} ub_constants[64];
	struct {
		Vec4f diffuse_albedo;
		Vec4f specular_albedo;  // w: power
	} ub_materials[64];
	sb6::Object m_object;
	SpuShader m_shader;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs shader_attrs = {
		        {"preface", "#version 410 core\n"},
		};
		Attrs unif_attrs = {
		        {"UB_CONSTANT", &ub_constants[0]},
		        {"UB_MATERIAL", &ub_materials[0]},
		};
		loadShader(m_shader, "blinnphong/blinnphong.us", shader_attrs, unif_attrs);
	}
	// object
	{
		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};
		m_object.load("torus.sbm", Attrs(), sym, m_shader.id());
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

void App::render()
{
	auto eye = Vec3f(0.0, 0.0, 20.0);
	sb6::Composition composition;
	composition.lookat(eye, ezero(), ey());
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	auto worldview = composition.worldview();
	auto viewscreen = composition.viewscreen();
	auto modelworld = c_unit.rot("XXy", -20.0, -180.0, radians(-getSeconds().current() * 14.5 * 8.0));

	auto n = 0;
	for (auto j = 0; j < 7; j++) {
		for (auto i = 0; i < 7; i++, n++) {
			ub_constants[n].worldview = worldview;
			ub_constants[n].viewscreen = viewscreen;
			ub_constants[n].modelworld = worldview
			                           * c_unit.trans({i * 2.75f - 8.25f, 6.75f - j * 2.25f, 0.0f})
			                           * modelworld;
			ub_materials[n].diffuse_albedo = {0.5, 0.2, 0.7, 1.0};
			ub_materials[n].specular_albedo = Vec3f(i / 9.0 + 1.0 / 9.0);
			ub_materials[n].specular_albedo.w = powf(2.0, j + 2.0);
		}
	}
	m_shader.use();
	m_object.draw(n);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("blinnphong");
}  // namespace spu::blinnphong
