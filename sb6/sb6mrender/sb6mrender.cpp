//
// App :
//
#include "base_app.h"
namespace spu::sb6mrender {
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_shader;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	Mat4f mat_rotation;
	uint32_t u_albedomap;
	uint32_t u_normalmap;
	sb6::Object m_object;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	mat_rotation = c_unit;
	// program
	{
		Attrs unif_attrs = {
		        {"u_modelview",  &u_modelview },
		        {"u_viewscreen", &u_viewscreen},
		        {"u_albedomap",  &u_albedomap },
		        {"u_normalmap",  &u_normalmap },
		};
		loadShader(m_shader, "sb6mrender/render.us", Attrs(), unif_attrs);
	}
	// object & texture
	{
		const char *sym[] = {
		        "a.a_position", "a.a_normal", "a.a_tangent", "a.a_bitangent", "a.a_texcoord", nullptr,
		};
		m_object.load("ladybug.sbm", Attrs(), sym, m_shader.id());
		u_albedomap = sb6::ktx::load("ladybug_co.ktx");
		u_normalmap = sb6::ktx::load("ladybug_nm.ktx");
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		// renderstate.use();
	}
	// bg
	{
		Attrs attrs = {
		        {"bgcolor0", c_green},
		        {"bgdetph",  1.0    },
		};
		BaseApp::set(attrs);
	}
}

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	u_modelview = c_unit.rot("Y", -t * 5.0).trans({0.0, -0.5, -7.0});
	m_shader.use();
	m_object.draw();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("sb6mrender");
}  // namespace spu::sb6mrender
