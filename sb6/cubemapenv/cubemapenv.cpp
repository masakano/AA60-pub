//
// App :
//
#include "base_app.h"
namespace spu::cubemapenv {

class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	SpuShader m_renderShader;
	SpuShader m_skyboxShader;
	sb6::Object m_object;

	uint32_t u_cubemap_texture;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	Mat4f u_worldview;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	{
		Attrs attrs = {
		        {"wrap_s", GL_CLAMP_TO_EDGE},
		        {"wrap_t", GL_CLAMP_TO_EDGE},
		};
		u_cubemap_texture = sb6::ktx::load("envmaps/mountaincube.ktx", attrs);
	}
	{
		Attrs unif_attrs = {
		        {"u_modelview",       &u_modelview      },
		        {"u_viewscreen",      &u_viewscreen     },
		        {"u_cubemap_texture", &u_cubemap_texture},
		};
		loadShader(m_renderShader, "cubemapenv/render.us", Attrs(), unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"u_worldview",       &u_worldview      },
		        {"u_cubemap_texture", &u_cubemap_texture},
		};
		loadShader(m_skyboxShader, "cubemapenv/skybox.us", Attrs(), unif_attrs);
	}
	{
		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};
		m_object.load("dragon.sbm", Attrs(), sym, m_renderShader.id());
	}
	{
	}
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.cube_map_seamless = true;
		renderstate.depth_func = GL_LEQUAL;
		// renderstate.use();
	}
}

void App::render()
{
	const auto t = getSeconds().current() * 0.1;  // 0.1sec
	auto &renderstate = getRenderstate();
	sb6::Composition composition;
	composition.perspective(viewport(0), 60.0, 0.1, 1000.0);
	composition.lookat(Vec3f(15.0 * sinf(t), 0.0, 15.0 * cosf(t)), ezero(), ey());
	u_viewscreen = composition.viewscreen();
	u_worldview = composition.worldview();
	u_modelview = u_worldview * c_unit.trans({0.0, -4.0, 0.0}).rot("YX", -t * 130.1, -t);
	// skybox
	{
		m_skyboxShader.use();
		renderstate.flags.depth_test = false;
		renderstate.use();
		drawFullscreenQuad();
	}
	// object
	{
		m_renderShader.use();
		renderstate.flags.depth_test = true;
		renderstate.use();
		m_object.draw();
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("cubemapenv");
}  // namespace spu::cubemapenv
