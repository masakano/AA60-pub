//
// App :
//
#include "base_app.h"
namespace spu::dispmap {

class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void menu() override;
	void init(const Attrs &attrs) override;
	void render() override;
	SpuArray m_array;
	uint32_t u_displacement_texture;
	uint32_t u_color_texture;
	float u_dmap_depth = 5.0;
	bool m_wireframe = 0;
	bool m_fog = 1;
	SpuShader m_shader;
	Mat4f u_modelscreen;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	int32_t u_enable_fog = true;
	Vec4f u_fog_color = {0.7, 0.8, 0.9, 0.0};
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs unif_attrs = {
		        {"u_modelview",            &u_modelview           },
		        {"u_modelscreen",          &u_modelscreen         },
		        {"u_viewscreen",           &u_viewscreen          },
		        {"u_dmap_depth",           &u_dmap_depth          },
		        {"u_enable_fog",           &u_enable_fog          },
		        {"u_fog_color",            &u_fog_color           },
		        {"u_color_texture",        &u_color_texture       },
		        {"u_displacement_texture", &u_displacement_texture},
		};
		loadShader(m_shader, "dispmap/dispmap.us", Attrs(), unif_attrs);
	}
	// array
	{
		Attrs array_attrs = {
		        {"shader_id",  m_shader.id()},
		        {"a.position", 1            },
		};
		m_array.init(array_attrs);
		int32_t vert[] = {0, 0, 0, 0};
		m_array.send(vert, 4);
		m_array.set("patch_vertices", 4);
	}
	// texture
	{
		u_displacement_texture = sb6::ktx::load("terragen1.ktx");
		u_color_texture = sb6::ktx::load("terragen_color.ktx");
	}
	// environment
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.cull_face = true;
		// renderstate.use();
		const Vec4f c_sky = {0.85, 0.95, 1.0, 1.0};
		Attrs frame_attrs = {
		        {"bgcolor0", c_sky},
		        {"bgdepgh",  1.0  },
		};
		BaseApp::set(frame_attrs);
	}
}

void App::menu()
{
	ImGui::Checkbox("fog", &m_fog);
	ImGui::Checkbox("wireframe", &m_wireframe);
	ImGui::SliderFloat("map depth", &u_dmap_depth, 0.0, 10.0);
}

void App::render()
{
	auto t = getSeconds().current() * 0.03;  // 0.03sec
	auto r = sinf(t * 5.37f) * 15.0f + 16.0f;
	auto h = cosf(t * 4.79f) * 2.0f + 3.2f;
	// uniform
	sb6::Composition composition;
	composition.lookat(Vec3f(sinf(t) * r, h, cosf(t) * r), ezero(), ey());
	composition.perspective(viewport(0), 60.0, 0.1, 1000.0);
	u_modelview = c_unit.rot("Y", -t * 30 * 6.0) * composition.worldview();
	u_viewscreen = composition.viewscreen();
	u_modelscreen = u_viewscreen * u_modelview;
	u_enable_fog = m_fog;
	m_shader.use();
	// renderstate
	{
		auto &renderstate = getRenderstate();
		if (m_wireframe) {
			renderstate.flags.fill = false;
		}
		else {
			renderstate.flags.fill = true;
		}
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		renderstate.use();
	}
	m_array.draw(GL_PATCHES, 0, 4, 64 * 64);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("dispmap");
}  // namespace spu::dispmap
