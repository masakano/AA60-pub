//
// App :
//
#include "base_app.h"
namespace spu::simpletexcoords {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;
	SpuShader m_shader;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	uint32_t u_texture;
	SpuTexture m_textures[2];
	int32_t m_index = 0;
	sb6::Object m_object;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// texture
	{
#define e_b 0x00, 0x00, 0x00, 0x00
#define e_w 0xFF, 0xFF, 0xFF, 0xFF
		const std::vector<GLubyte> c_tex_datas = {
		        e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w,
		        e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b,
		        e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w,
		        e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b,
		        e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w,
		        e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b,
		        e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w,
		        e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b,
		        e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w,
		        e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b,
		        e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w,
		        e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b,
		        e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w,
		        e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b,
		        e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w,
		        e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b, e_w, e_b,
		};
#undef e_b
#undef e_w
		// const void *pixv[] = { (void *)c_tex_data, 0 };
		Attrs attr = {
		        {"target",     GL_TEXTURE_2D     },
		        {"iformat",    GL_RGBA8          },
		        {"width",      16                },
		        {"height",     16                },
		        {"min_filter", GL_NEAREST        },
		        {"mag_filter", GL_NEAREST        },
		        {"data",       c_tex_datas.data()},
		};
		m_textures[0].init(attr);
		m_textures[1].reset(sb6::ktx::load("pattern1.ktx"));
	}
	// shader
	{
		Attrs unif_attrs = {
		        {"u_modelview",  &u_modelview },
		        {"u_viewscreen", &u_viewscreen},
		        {"u_texture",    &u_texture   },
		};
		loadShader(m_shader, "simpletexcoords/render.us", Attrs(), unif_attrs);
	}
	// object
	{
		const char *sym[] = {
		        "a.a_position", "a.a_normal", "a.a_tangent", "a.a_bitangent", "a.a_texcoord", nullptr,
		};
		m_object.load("torus_nrms_tc.sbm", Attrs(), sym, m_shader.id());
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

void App::menu() { ImGui::Combo("pattern", &m_index, "pattern0\0pattern1\0\0"); }

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 60.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	u_modelview = c_unit.rot("ZY", -t * 21.1, -t * 19.3).trans({0.0, 0.0, -3.0});
	u_texture = m_textures[m_index].id();
	m_shader.use();
	m_object.draw();
	spu_printf(0, "m_index : [%d:T]\n", m_index);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("simpletexcoords");
}  // namespace spu::simpletexcoords
