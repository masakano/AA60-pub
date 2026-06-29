//
// App :
//
#include "base_app.h"
namespace spu::bumpmapping {

class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	Vec3f u_light_position;
	uint32_t u_albedomap;
	uint32_t u_normalmap;
	SpuShader m_shader;
	sb6::Object m_object;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs shader_attrs = {
		        {"preface", "#version 420\n"},
		};
		Attrs unif_attrs = {
		        {"u_modelview",      &u_modelview     },
                        {"u_viewscreen",     &u_viewscreen    },
		        {"u_light_position", &u_light_position},
                        {"u_albedomap",      &u_albedomap     },
		        {"u_normalmap",      &u_normalmap     },
		};
		loadShader(m_shader, "bumpmapping/bumpmapping.us", shader_attrs, unif_attrs);
	}
	// object
	{
		u_albedomap = sb6::ktx::load("ladybug_co.ktx");
		u_normalmap = sb6::ktx::load("ladybug_nm.ktx");

		const char *sym[] = {
		        "a.a_position", "a.a_normal", "a.a_tangent", "a.a_bitangent", "a.a_texcoord", nullptr,
		};
		m_object.load("ladybug.sbm", Attrs(), sym, m_shader.id());
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
	}
	spu_frame_set(-1, "bgcolor0", c_gray);
}

void App::render()
{
	auto f = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	u_modelview = c_unit.rot("YX", 20.0, -14.5).trans({0.0, -0.2, -5.5});
	u_light_position = Vec3f(40.0 * sinf(f), 30.0 + 20.0 * cosf(f), 40.0);
	m_shader.use();
	m_object.draw();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("bumpmapping");
}  // namespace spu::bumpmapping
