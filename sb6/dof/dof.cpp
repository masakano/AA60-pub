//
// App :
//
#include "base_app.h"
namespace spu::dof {
class App : public BaseApp {
public:
	enum { def_local_size = 1024 };
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;

private:
	class DofObject : public sb6::Object {
	public:
		Mat4f model_matrix;
		Vec4f diffuse_albedo;
	};
	std::vector<DofObject> m_objects;

	SpuShader m_viewShader;
	SpuShader m_filterShader;
	SpuShader m_displayShader;
	uint32_t m_arrayId = 0;
	uint32_t m_computeId = 0;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	Vec3f u_albedo = {0.9, 0.8, 1.0};
	Vec3f u_specular_albedo = {0.7, 0.7, 0.7};
	float u_specular_power = 300.0;
	int32_t u_full_shading = true;
	float u_focal_distance = 40.0;
	float u_focal_depth = 50.0;
	uint32_t u_input_image = 0;
	uint32_t u_output_image = 0;

	SpuFrame m_depthFrame;
	SpuTexture m_depthTexture;
	SpuTexture m_colorTexture;
	SpuTexture m_tempTexture;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// view_shader
	{
		Attrs unif_attrs = {
		        {"u_viewscreen",      &u_viewscreen     },
                        {"u_modelview",       &u_modelview      },
		        {"u_full_shading",    &u_full_shading   },
                        {"u_albedo",          &u_albedo         },
		        {"u_specular_albedo", &u_specular_albedo},
                        {"u_specular_power",  &u_specular_power },
		};
		loadShader(m_viewShader, "dof/render.us", Attrs(), unif_attrs);
	}
	// filter shader
	{
		Attrs shader_attrs = {
		        {"def_local_size", def_local_size},
		};
		Attrs unif_attrs = {
		        {"u_input_image",  &u_input_image },
		        {"u_output_image", &u_output_image},
		};
		loadShader(m_filterShader, "dof/gensat.us", shader_attrs, unif_attrs);
	}
	// display shader
	{
		Attrs unif_attrs = {
		        {"u_focal_distance", &u_focal_distance},
		        {"u_focal_depth",    &u_focal_depth   },
		        {"u_input_image",    &u_input_image   },
		};
		loadShader(m_displayShader, "dof/display.us", Attrs(), unif_attrs);
	}
	// models
	{
		const char *const object_names[]
		        = {"dragon.sbm", "sphere.sbm", "cube.sbm", "cube.sbm", "cube.sbm"};
		static const Vec4f c_object_colors[] = {
		        {1.0, 0.7, 0.8, 1.0},
                        {0.7, 0.8, 1.0, 1.0},
                        {0.3, 0.9, 0.4, 1.0},
		        {0.6, 0.4, 0.9, 1.0},
                        {0.8, 0.2, 0.1, 1.0},
		};

		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};

		m_objects.resize(5);
		for (auto i = 0u; i < m_objects.size(); i++) {
			m_objects[i].load(object_names[i], Attrs(), sym, m_viewShader.id());
			m_objects[i].diffuse_albedo = c_object_colors[i];
		}
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		// renderstate.use();
	}
	// arrays
	{
		m_arrayId = spu_array_new({
		        {"nelem", 4}
                });
		m_computeId = spu_array_new(Attrs());
	}
	auto fbo_size = int32_t(std::max(viewport(0).sx, viewport(0).sy));

	// depth_tex
	{
		Attrs attrs = {
		        {"target",      GL_TEXTURE_2D        },
                        {"iformat",     GL_DEPTH_COMPONENT32F},
		        {"width",       fbo_size             },
                        {"height",      fbo_size             },
		        {"min_filter",  GL_LINEAR            },
                        {"mag_filter",  GL_LINEAR            },
		        {"wrap_s",      GL_CLAMP_TO_EDGE     },
                        {"wrap_t",      GL_CLAMP_TO_EDGE     },
		        {"auto_mipmap", 0                    },
		};
		m_depthTexture.init(attrs);
	}
	// color_tex
	{
		Attrs attrs = {
		        {"target",      GL_TEXTURE_2D   },
                        {"iformat",     GL_RGBA32F      },
                        {"width",       fbo_size        },
		        {"height",      fbo_size        },
                        {"min_filter",  GL_LINEAR       },
                        {"mag_filter",  GL_LINEAR       },
		        {"wrap_s",      GL_CLAMP_TO_EDGE},
                        {"wrap_t",      GL_CLAMP_TO_EDGE},
                        {"auto_mipmap", 0               },
		};
		m_colorTexture.init(attrs);
		m_tempTexture.init(attrs);
	}
	// fbo
	{
		Attrs attrs = {
		        {"viewport0", viewport(0)        },
		        {"color0",    m_colorTexture.id()},
		        {"depth",     m_depthTexture.id()},
		};
		m_depthFrame.init(attrs);
		spu_frame_set(m_depthFrame.id(), "bgcolor0", c_gray);
	}
}

void App::menu()
{
	ImGui::SliderFloat("focul distance", &u_focal_distance, 5.0, 100.0);
	ImGui::SliderFloat("focul depth", &u_focal_depth, 5.0, 100.0);
}

void App::render()
{
	const auto f = getSeconds().current();
	auto view_position = Vec3f(0.0, 0.0, 40.0);

	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 2.0, 300.0);
	composition.lookat(view_position, ezero(), ey());

	auto modelworld = c_unit.trans({0.0, -4.0, 0.0}).rot("XY", -20.0, -f * 14.5);

	m_objects[0].model_matrix = modelworld.trans({+5.0, 0.0, 20.0});
	m_objects[1].model_matrix = modelworld.trans({-5.0, 0.0, 0.0});
	m_objects[2].model_matrix = modelworld.trans({-15.0, 0.0, -20.0});
	m_objects[3].model_matrix = modelworld.trans({-25.0, 0.0, -40.0});
	m_objects[4].model_matrix = modelworld.trans({-35.0, 0.0, -60.0});

	auto &renderstate = getRenderstate();

	renderstate.flags.depth_test = true;
	renderstate.use();

	// render
	{
		m_depthFrame.begin();
		m_depthFrame.clear();
		u_viewscreen = composition.viewscreen();
		for (auto i = 0u; i < m_objects.size(); i++) {
			u_modelview = composition.worldview() * m_objects[i].model_matrix;
			u_albedo = m_objects[i].diffuse_albedo;
			u_full_shading = 1;
			m_viewShader.use();
			m_objects[0].draw();
		}
		m_depthFrame.end();
	}

	// filter (1)
	{
		u_input_image = m_colorTexture.id();
		u_output_image = m_tempTexture.id();
		m_filterShader.use();

		auto gx = viewport(0).sy;
		auto gy = std::max(1, int(viewport(0).sx) / def_local_size);

		spu_array_draw(m_computeId, 0xffff, gx, gy, 1);
		spu_graphics_memory_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	// filter (2)
	{
		u_input_image = m_tempTexture.id();
		u_output_image = m_colorTexture.id();
		m_filterShader.use();

		auto gx = viewport(0).sx;
		auto gy = std::max(1, int(viewport(0).sy) / def_local_size);

		spu_array_draw(m_computeId, 0xffff, gx, gy, 1);
		spu_graphics_memory_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		renderstate.flags.depth_test = false;
		renderstate.use();
	}
	// resolve
	{
		u_input_image = m_colorTexture.id();
		m_displayShader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLE_STRIP, 0, 4);
	}
}
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("dof");
}  // namespace spu::dof
