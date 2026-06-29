//
// App :
//
#include <ssys/random_generator.h>
#include "base_app.h"

namespace spu::ssao {

class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;

private:
	struct {
		Vec3f point[256];
		Vec4f random_vectors[256];
	} m_pointData;

	sb6::Object m_object;
	sb6::Object m_cube;

	SpuFrame m_renderFrame;
	SpuShader m_renderShader;
	SpuShader m_ssaoShader;
	float m_ssaoRadius = 0.05;
	uint32_t m_array = 0;
	float m_pointCount = 10;
	bool m_isShading = 1;
	bool m_isAo = 1;
	bool m_isWeightByAngle = 1;
	bool m_isRandomizePoints = 1;

	Mat4f u_modelview;
	Mat4f u_viewscreen;
	float u_ssao_level = 1.0;
	float u_ssao_radius = 5.0;
	float u_object_level = 1.0;
	uint32_t u_point_count = 8;
	uint32_t u_color_texture;
	uint32_t u_normal_depth_texture;
	int32_t u_randomize_points = true;
	int32_t u_weight_by_angle = true;

	Vec3f u_light_position = Vec3f(100.0, 100.0, 100.0);

	Vec3f u_albedo = Vec3f(0.8, 0.8, 0.9);
	Vec3f u_specular_albedo = Vec3f(0.01);
	float u_specular_power = 128.0;
	float u_shading_level = 1.0;


};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// render m_shader
	{
		Attrs unif_attrs = {
		        {"u_modelview",     &u_modelview    },
		        {"u_viewscreen",    &u_viewscreen   },
		        {"u_shading_level", &u_shading_level},
		        {"u_light_position", &u_light_position},
		        {"u_albedo", &u_albedo},
		        {"u_specular_albedo", &u_specular_albedo},
		        {"u_specular_power", &u_specular_power},
		        {"u_shading_level", &u_shading_level},
		};
		loadShader(m_renderShader, "ssao/render.us", Attrs(), unif_attrs);
	}
	// ssao m_shader
	{
		Attrs unif_attrs = {
		        {"u_ssao_radius",          &u_ssao_radius         },
		        {"u_ssao_level",           &u_ssao_level          },
		        {"u_object_level",         &u_object_level        },
		        {"u_weight_by_angle",      &u_weight_by_angle     },
		        {"u_randomize_points",     &u_randomize_points    },
		        {"u_point_count",          &u_point_count         },
		        {"u_color_texture",        &u_color_texture       },
		        {"u_normal_depth_texture", &u_normal_depth_texture},
		        {"SAMPLE_POINTS",          &m_pointData           }, // UNIFORM BUFFER
		};
		loadShader(m_ssaoShader, "ssao/ssao.us", Attrs(), unif_attrs);
	}
	// FBO
	{
		const auto fbo_size = 2048;
		const Rectf viewport_frame = {0, 0, fbo_size, fbo_size};
		Attrs attrs = {
		        {"viewport0",         viewport_frame       },
		        {"color0.target",     GL_TEXTURE_2D        },
		        {"color0.iformat",    GL_RGB16F            },
		        {"color0.min_filter", GL_NEAREST           },
		        {"color0.mag_filter", GL_NEAREST           },
		        {"color1.target",     GL_TEXTURE_2D        },
		        {"color1.iformat",    GL_RGBA32F           },
		        {"color1.min_filter", GL_NEAREST           },
		        {"color1.mag_filter", GL_NEAREST           },
		        {"depth.target",      GL_RENDERBUFFER      },
		        {"depth.iformat",     GL_DEPTH_COMPONENT32F},
		        {"depth.min_filter",  GL_NEAREST           },
		        {"depth.mag_filter",  GL_NEAREST           },
		        {"depth.wrap_s",      GL_CLAMP_TO_EDGE     },
		        {"depth.wrap_t",      GL_CLAMP_TO_EDGE     },
		        {"bgcolor0",          c_black              },
		        {"bgcolor1",          c_black              },
		        {"bgdepth",           1.0                  },
		};
		m_renderFrame = SpuFrame(attrs);

		// shrink
		m_renderFrame.set("viewport0", viewport(0));
		u_color_texture = m_renderFrame.getBuffer("color0").id();
		u_normal_depth_texture = m_renderFrame.getBuffer("color1").id();
	}
	// array
	{
		m_array = spu_array_new({
		        {"nelem", 4}
                });

		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};
		m_object.load("dragon.sbm", Attrs(), sym, m_renderShader.id());
		m_cube.load("cube.sbm", Attrs(), sym, m_renderShader.id());
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
	}
	// point32_t data
	{
		RandomGenerator<float> frand;
		frand.seed(0);  // need parameterized
		for (auto &p: m_pointData.point) {
			do {
				p = {
				        frand() * 2.0f - 1.0f,
				        frand() * 2.0f - 1.0f,
				        frand(),
				};
			} while (length(p) > 1.0);
			p = normalize(p);
		}
		for (auto &v: m_pointData.random_vectors) {
			v = {frand(), frand(), frand(), frand()};
		}
	}
}

void App::menu()
{
	ImGui::Checkbox("weight by angle", &m_isWeightByAngle);
	ImGui::Checkbox("randomize points", &m_isRandomizePoints);
	ImGui::Checkbox("show shading", &m_isShading);
	ImGui::Checkbox("show ao", &m_isAo);

	ImGui::SliderFloat("point count", &m_pointCount, 1.0, 255.0);
	ImGui::SliderFloat("ssao radius", &m_ssaoRadius, 0.05, 0.5);
}

void App::render()
{
	auto f = getSeconds().current();
	{
		m_renderFrame.begin();
		m_renderFrame.clear();

		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.use();

		sb6::Composition composition;
		composition.lookat(Vec3f(0.0, 3.0, 15.0), ezero(), ey());
		composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
		auto lookat_matrix = composition.worldview();
		u_viewscreen = composition.viewscreen();

		u_modelview = lookat_matrix * c_unit.rot("Y", f * 5.0).trans({0.0, -5.0, 0.0});
		u_shading_level = m_isShading ? (m_isAo ? 0.7 : 1.0) : 0.0;
		m_renderShader.use();
		m_object.draw();
		u_modelview = lookat_matrix
		            * c_unit.scale({4000.0, 0.1, 4000.0}).rot("Y", f * 5.0).trans({0.0, -4.5, 0.0});
		m_renderShader.use();
		m_cube.draw();
		m_renderFrame.end();
	}
	{
		u_point_count = m_pointCount;
		u_ssao_radius = m_ssaoRadius * viewport(0).sx / 1000.0;
		u_ssao_level = m_isAo ? (m_isShading ? 0.3 : 1.0) : 0.0;
		u_object_level = 1.0;
		u_randomize_points = m_isRandomizePoints ? 1 : 0;
		u_weight_by_angle = m_isWeightByAngle ? 1 : 0;
		m_ssaoShader.use();

		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = false;
		renderstate.use();
		spu_array_draw(m_array, GL_TRIANGLE_STRIP, 0, 4);
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("ssao");
}  // namespace spu::ssao
