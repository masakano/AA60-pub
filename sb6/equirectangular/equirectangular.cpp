//
// App :
//
#include "base_app.h"
namespace spu::equirectangular {
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_shader;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	uint32_t u_envmap_texture;
	uint32_t m_envmaps[3];
	int32_t m_index = 0;
	sb6::Object m_object;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// texture
	{
		Attrs tex_attrs = {
		        {"wrap_s", GL_REPEAT},
		        {"wrap_t", GL_REPEAT},
		};
		m_envmaps[0] = sb6::ktx::load("envmaps/equirectangularmap1.ktx", tex_attrs);
		u_envmap_texture = m_envmaps[m_index];
	}
	// shader
	{
		Attrs unif_attrs = {
		        {"u_modelview",      &u_modelview     },
		        {"u_viewscreen",     &u_viewscreen    },
		        {"u_envmap_texture", &u_envmap_texture},
		};
		loadShader(m_shader, "equirectangular/render.us", Attrs(), unif_attrs);
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
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		// renderstate.use();
	}
	const Vec4f c_gray = {0.2, 0.2, 0.2, 1.0};
	spu_frame_set(-1, "bgcolor0", c_gray);
}

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 60.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	u_modelview
	        = c_unit.trans({0.0, -4.0, 0.0}).rot("YX", -t * 1.1 * 8.0, -t * 8.0).trans({0.0, 0.0, -15.0});
	m_shader.use();
	m_object.draw();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("equirectangular");
}  // namespace spu::equirectangular
