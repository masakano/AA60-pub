//
// App :
//
#include "base_app.h"
namespace spu::perpixelgloss {
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_shader;
	sb6::Object m_object;
	uint32_t u_envmap_texture;
	uint32_t u_glossmap_texture;
	Mat4f u_viewscreen;
	Mat4f u_modelview;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// texture
	{
		Attrs envmap_attrs = {
		        {"min_filter", GL_LINEAR       },
                        {"mag_filter", GL_LINEAR       },
		        {"wrap_s",     GL_CLAMP_TO_EDGE},
                        {"wrap_t",     GL_CLAMP_TO_EDGE},
		        {"wrap_r",     GL_CLAMP_TO_EDGE},
		};
		u_envmap_texture = sb6::ktx::load("envmaps/mountaincube.ktx", envmap_attrs);
		u_glossmap_texture = sb6::ktx::load("pattern1.ktx");
	}
	// shader
	{
		Attrs unif_attrs = {
		        {"u_modelview",        &u_modelview       },
		        {"u_viewscreen",       &u_viewscreen      },
		        {"u_envmap_texture",   &u_envmap_texture  },
		        {"u_glossmap_texture", &u_glossmap_texture},
		};
		loadShader(m_shader, "perpixelgloss/perpixelgloss.us", Attrs(), unif_attrs);
	}
	// object
	{
		const char *syms[] = {
		        "a.a_position", "a.a_normal", "a.a_tangent", "a.a_bitangent", "a.a_texcoord", nullptr,
		};
		m_object.load("torus_nrms_tc.sbm", Attrs(), syms, m_shader.id());
	}

	// environment
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		spu_frame_set(-1, "bgcolor0", c_gray);
	}
}

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 60.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	u_modelview = c_unit.rot("XZY", -t * 15.3, -t * 7.75, -t * 13.75).trans({0.0, 0.0, -3.0});
	m_shader.use();
	m_object.draw();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("perpixelgloss");
}  // namespace spu::perpixelgloss
