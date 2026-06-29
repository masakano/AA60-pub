//
// App :
//
#include "base_app.h"
namespace spu::toonshading {
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	uint32_t u_toon_texture;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	SpuShader m_shader;
	sb6::Object m_object;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// texture
	{
		const std::vector<uint8_t> toon_tex_data = {0x44, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00,
		                                            0xCC, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00};

		Attrs attrs = {
		        {"target",     GL_TEXTURE_1D            },
		        {"iformat",    GL_RGBA8                 },
		        {"width",      sizeof(toon_tex_data) / 4},
		        {"mag_filter", GL_NEAREST               },
		        {"min_filter", GL_NEAREST               },
		        {"wrap_s",     GL_CLAMP_TO_EDGE         },
		        {"data",       toon_tex_data.data()     },
		};
		u_toon_texture = spu_texture_new(attrs);
	}
	// shader
	{
		Attrs unif_attrs = {
		        {"u_modelview",    &u_modelview   },
		        {"u_viewscreen",   &u_viewscreen  },
		        {"u_toon_texture", &u_toon_texture},
		};
		loadShader(m_shader, "toonshading/toonshading.us", Attrs(), unif_attrs);
	}
	// model
	{
		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};
		m_object.load("torus_nrms_tc.sbm", Attrs(), sym, m_shader.id());
	}

	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
	}

	// background
	{
		const Vec4f c_gray = {0.2, 0.2, 0.2, 1.0};
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

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("toonshading");
}  // namespace spu::toonshading
