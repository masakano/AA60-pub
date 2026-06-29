//
// App :
//
#include "base_app.h"
namespace spu::shadowmapping {
class App : public BaseApp {
public:
	static constexpr auto c_depth_texture_size = 2048;
	enum { e_full = 0, e_light, e_depth };

	class ShadowedObject : public sb6::Object {
	public:
		Mat4f modelworld;
	};

	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;
	void renderScene(bool from_light);

	SpuShader m_lightShader;
	SpuShader m_viewShader;
	SpuShader m_showLightDepthShader;
	SpuFrame m_depthFrame;
	SpuTexture m_depthTexture;
	SpuTexture m_depthDebugTexture;
	Rectf m_depthViewport;
	std::vector<ShadowedObject> m_objects;

	int32_t m_mode;

	Mat4f u_modelscreen;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	Mat4f u_modelshadowtex;
	int32_t u_full_shading;
	uint32_t u_shadow_texture;
	uint32_t u_depth_texture;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	{
		const char *paths[] = {
		        "shadowmapping/shadowmapping-light.us",
		        "shadowmapping/shadowmapping-camera.us",
		        "shadowmapping/shadowmapping-light-view.us",
		};

		SpuShader *shaders[] = {&m_lightShader, &m_viewShader, &m_showLightDepthShader};

		for (auto &shader: shaders) {
			auto i = &shader - &shaders[0];
			Attrs unif_attrs = {
			        {"u_modelscreen",    &u_modelscreen   },
			        {"u_viewscreen",     &u_viewscreen    },
			        {"u_modelview",      &u_modelview     },
			        {"u_modelshadowtex", &u_modelshadowtex},
			        {"u_shadow_texture", &u_shadow_texture},
			        {"u_depth_texture",  &u_depth_texture },
			        {"u_full_shading",   &u_full_shading  },
			};
			loadShader(*shader, paths[i], Attrs(), unif_attrs);
		}
	}
	// object
	{
		std::vector<const char *> object_names = {"dragon.sbm", "sphere.sbm", "cube.sbm", "torus.sbm"};
		m_objects.resize(object_names.size());

		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};
		for (auto i = 0u; i < object_names.size(); i++) {
			m_objects[i].load(object_names[i], Attrs(), sym, m_lightShader.id());
		}
	}
	{
		Attrs attrs = {
		        {"target",       GL_TEXTURE_2D            },
                        {"iformat",      GL_DEPTH_COMPONENT32F    },
		        {"width",        c_depth_texture_size     },
                        {"height",       c_depth_texture_size     },
		        {"min_filter",   GL_LINEAR                },
                        {"mag_filter",   GL_LINEAR                },
		        {"mag_filter",   GL_LINEAR                },
                        {"compare_mode", GL_COMPARE_REF_TO_TEXTURE},
		        {"compare_func", GL_LEQUAL                },
		};
		m_depthTexture.init(attrs);
	}
	{
		Attrs attrs = {
		        {"target",     GL_TEXTURE_2D       },
                        {"iformat",    GL_R32F             },
		        {"width",      c_depth_texture_size},
                        {"height",     c_depth_texture_size},
		        {"min_filter", GL_LINEAR           },
                        {"mag_filter", GL_LINEAR           },
		};
		m_depthDebugTexture.init(attrs);
	}
	{
		m_depthViewport = Rectf(0, 0, c_depth_texture_size, c_depth_texture_size);

		Attrs attrs = {
		        {"viewport0", m_depthViewport         },
		        {"depth",     m_depthTexture.id()     },
		        {"color0",    m_depthDebugTexture.id()},
		};
		m_depthFrame.init(attrs);
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
	}
}

void App::menu() { ImGui::Combo("disp mode", &m_mode, "full\0light\0depth\0\0"); }

void App::render()
{
	auto f = getSeconds().current() + 30.0f;

	m_objects[0].modelworld = c_unit.trans({0.0, -4.0, 0.0}).rot("XY", -20.0, -f * 14.5);
	m_objects[1].modelworld = c_unit.scale(2.0)
	                                  .trans({sinf(f * 0.37) * 12.0f, cosf(f * 0.37) * 12.0f, 0.0f})
	                                  .rot("Y", -f * 3.7);
	m_objects[2].modelworld = c_unit.scale(2.0)
	                                  .rot("Z", -f * 99.0)
	                                  .trans({sinf(f * 0.25) * 10.0f, cosf(f * 0.25) * 10.0f, 0.0f})
	                                  .rot("Y", -f * 6.45);
	m_objects[3].modelworld = c_unit.scale(2.0)
	                                  .rot_axis(radians(f * 120.3), Vec3f(0.707106, 0.0, 0.707106))
	                                  .trans({sinf(f * 0.51) * 14.0f, cosf(f * 0.51) * 14.0f, 0.0f})
	                                  .rot("Y", -f * 5.25);
	renderScene(true);
	if (m_mode == e_depth) {
		SpuScopedRenderstate renderstate(1);
		renderstate.flags.depth_test = false;
		renderstate.use();
		u_depth_texture = m_depthDebugTexture.id();
		m_showLightDepthShader.use();
		// m_array.draw(nullptr);
		spu_array_draw(0, GL_TRIANGLE_STRIP);
	}
	else {
		renderScene(false);
	}
	spu_frame_set(-1, "bgcolor0", c_gray);
}

void App::renderScene(bool from_light)
{
	auto light_position = Vec3f(20.0, 20.0, 20.0);
	auto view_position = Vec3f(0.0, 0.0, 40.0);

	sb6::Composition light_composition;
	light_composition.frustum(m_depthViewport, 1.0, 200.0);
	light_composition.lookat(light_position, ezero(), ey());

	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 1.0, 200.0);
	composition.lookat(view_position, ezero(), ey());

	auto light_vp_matrix = light_composition.worldscreen();
	auto worldshadowtex = Mat4f::screentexc() * light_composition.worldscreen();

	if (from_light) {
		m_depthFrame.begin();
		m_depthFrame.clear();
		auto &renderstate = getRenderstate();
		renderstate.flags.fill_offset = true;
		renderstate.poly_offset.factor = 4.0;
		renderstate.poly_offset.units = 4.0;
		renderstate.use();
	}
	for (auto &object: m_objects) {
		Mat4f &modelworld = object.modelworld;
		if (from_light) {
			u_modelscreen = light_vp_matrix * object.modelworld;
			m_lightShader.use();
		}
		else {
			u_viewscreen = composition.viewscreen();
			u_modelshadowtex = worldshadowtex * modelworld;
			u_modelview = composition.worldview() * object.modelworld;
			u_full_shading = m_mode == e_full ? 1 : 0;
			u_shadow_texture = m_depthTexture.id();
			m_viewShader.use();
		}
		object.draw();
	}
	if (from_light) {
		auto &renderstate = getRenderstate();
		renderstate.flags.fill_offset = false;
		renderstate.use();
		m_depthFrame.end();
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("shadowmapping");
}  // namespace spu::shadowmapping
