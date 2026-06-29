//
// App :
//
#include "base_app.h"
namespace spu::deferredshading {

enum {
	e_max_display_width = 4096,
	e_max_display_height = 2048,
	e_num_lights = 64,
	e_num_instances = (15 * 15)
};

class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;

private:
	struct Light {
		Vec3f position;
		Vec3f color;
	};
	enum Mode { e_none = 0, e_normals, e_ws_coords, e_diffuse, e_meta };
	sb6::Object m_object;
	SpuShader m_renderShader;
	SpuShader m_normalShader;
	SpuShader m_lightShader;
	SpuShader m_visShader;
	SpuFrame m_frame;
	bool m_isUseNormal = 0;

	Light ub_light[e_num_lights];
	Mat4f ub_transform[2 + e_num_instances];
	uint32_t u_gbuffer_textures[3];
	uint32_t u_albedomap;
	uint32_t u_normalmap;
	int32_t u_vis_mode = e_none;
	int32_t u_num_lights = e_num_lights;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// path #1
	{
		Attrs unif_attrs = {
		        {"u_albedomap",  &u_albedomap},
		        {"u_normalmap",  &u_normalmap},
		        {"ub_transform", ub_transform},
		};
		loadShader(m_renderShader, "deferredshading/render.us", Attrs(), unif_attrs);
		loadShader(m_normalShader, "deferredshading/render-nm.us", Attrs(), unif_attrs);
	}
	// path #2
	{
		Attrs unif_attrs = {
		        {"u_gbuffer_texture0", &u_gbuffer_textures[0]},
		        {"u_gbuffer_texture1", &u_gbuffer_textures[1]},
		        {"ub_light",           ub_light              },
		        {"u_vis_mode",         &u_vis_mode           },
		        {"u_num_lights",       &u_num_lights         },
		};
		loadShader(m_lightShader, "deferredshading/light.us", Attrs(), unif_attrs);
		loadShader(m_visShader, "deferredshading/light-vis.us", Attrs(), unif_attrs);
	}
	// texture
	{
		Attrs tex_attrs = {
		        {"target",     GL_TEXTURE_2D       },
                        {"iformat",    GL_RGBA32UI         },
		        {"width",      e_max_display_width },
                        {"height",     e_max_display_height},
		        {"min_filter", GL_NEAREST          },
                        {"min_filter", GL_NEAREST          },
		};
		tex_attrs.replace("iformat", GL_RGBA32UI);
		u_gbuffer_textures[0] = spu_texture_new(tex_attrs);
		tex_attrs.replace("iformat", GL_RGBA32F);
		u_gbuffer_textures[1] = spu_texture_new(tex_attrs);
		tex_attrs.replace("iformat", GL_DEPTH_COMPONENT32F);
		u_gbuffer_textures[2] = spu_texture_new(tex_attrs);
	}
	// frame buffer
	{
		Attrs init_attrs = {
		        {"color0", u_gbuffer_textures[0]},
		        {"color1", u_gbuffer_textures[1]},
		        {"depth",  u_gbuffer_textures[2]},
		};
		m_frame.init(init_attrs);

		auto bgcolor = Vec4f(0, 0, 0, 0);
		auto bgdepth = 1.0f;
		Attrs set_attrs = {
		        {"bgcolor0", bgcolor},
		        {"bgcolor1", bgcolor},
		        {"bgdepth",  bgdepth},
		};
		m_frame.set(set_attrs);
	}

	// model
	{
		u_normalmap = sb6::ktx::load("ladybug_nm.ktx");
		u_albedomap = sb6::ktx::load("ladybug_co.ktx");

		const char *sym[] = {
		        "a.a_position", "a.a_normal", "a.a_tangent", "a.a_bitangent", "a.a_texcoord", nullptr,
		};
		m_object.load("ladybug.sbm", Attrs(), sym, m_renderShader.id());
	}
}

void App::menu()
{
	ImGui::Checkbox("use_normal", &m_isUseNormal);
	ImGui::Combo("type", &u_vis_mode, "none\0normals\0ws_coords\0diffuse\0meta\0\0");
}

void App::render()
{
	auto t = getSeconds().current();
	// render gbuffer
	{
		m_frame.begin();
		m_frame.clear();
		sb6::Composition composition;
		composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
		auto d = (sinf(t * 0.131) + 2.0) * 0.15;
		auto eye_pos = Vec3f(d * 120.0f * sinf(t * 0.11), 5.5f, d * 120.0f * cosf(t * 0.01));
		composition.lookat(eye_pos, -20.0 * ey(), ey());
		ub_transform[0] = composition.viewscreen();
		ub_transform[1] = composition.worldview();

		for (auto j = 0; j < 15; j++) {
			for (auto i = 0; i < 15; i++) {
				ub_transform[j * 15 + i + 2]
				        = c_unit.trans({(i - 7.5f) * 7.0f, 0.0f, (j - 7.5f) * 11.0f});
			}
		}
		m_isUseNormal ? m_normalShader.use() : m_renderShader.use();
		auto &renderstate = getRenderstate();

		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		renderstate.use();
		m_object.draw(e_num_instances);
		m_frame.end();
	}
	// render frame
	{
		for (auto i = 0; i < e_num_lights; i++) {
			auto f = (i - 7.5f) * 0.1f + 0.3f;
			ub_light[i].position
			        = {100.0f * sinf(t * 1.1 + (5.0 * f)) * cosf(t * 2.3 + (9.0 * f)), 15.0f,
			           100.0f * sinf(t * 1.5 + (6.0 * f)) * cosf(t * 1.9 + (11.0 * f))};
			ub_light[i].color
			        = {cosf(f * 14.0) * 0.5f + 0.8f, sinf(f * 17.0) * 0.5f + 0.8f,
			           sinf(f * 13.0) * cosf(f * 19.0) * 0.5f + 0.8f};
		}
		if (u_vis_mode == e_none) {
			m_lightShader.use();
		}
		else {
			m_visShader.use();
		}
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = false;
		renderstate.use();
		drawFullscreenQuad();
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("deferredshading");
}  // namespace spu::deferredshading
