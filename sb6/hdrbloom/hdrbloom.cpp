//
// App :
//
#include "base_app.h"
namespace spu::hdrbloom {
class App : public BaseApp {
public:
	enum {
		e_sphere_count = 32,
	};

	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;

private:
	struct Material {
		Vec3f diffuse_color;
		Vec3f specular_color;
		Vec3f ambient_color;
		float specular_power;
	};
	struct Transforms {
		Mat4f viewscreen;
		Mat4f worldview;
		Mat4f modelworld[e_sphere_count];
	};
	sb6::Object m_object;
	SpuShader m_resolveShader;
	SpuShader m_filterShader;
	SpuShader m_renderShader;
	SpuFrame m_renderFrame;
	SpuFrame m_filterFrames[2];
	SpuTexture m_lutTexture;  // not used for now...
	SpuTexture m_sceneTexture;
	SpuTexture m_brightpassTexture;
	SpuTexture m_depthTexture;
	SpuTexture m_filterTextures[2];
	Material ub_material[e_sphere_count];
	Transforms ub_transforms;
	float u_exposure = 1.0;
	float u_scene_factor = 0.5;
	float u_bloom_factor = 0.5;
	float u_bloom_thresh_min = 0.8;
	float u_bloom_thresh_max = 1.2;
	Vec3f u_light_position = {100.0, 100.0, 100.0};
	uint32_t u_hdr_image;
	uint32_t u_bloom_image;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	auto alloc_texture = [](SpuTexture &texture, uint32_t format, uint32_t width, uint32_t height) {
		Attrs attrs = {
		        {"target",  GL_TEXTURE_2D},
		        {"iformat", format       },
		        {"width",   width        },
		        {"height",  height       },
		};
		texture.init(attrs);
	};
	// shader
	{
		Attrs unif_attrs = {
		        {"u_bloom_thresh_min", &u_bloom_thresh_min},
		        {"u_bloom_thresh_max", &u_bloom_thresh_max},
		        {"u_light_position",   &u_light_position  },
		        {"UB_MATERIAL",        ub_material        },
		        {"UB_TRANSFORMS",      &ub_transforms     },
		};
		loadShader(m_renderShader, "hdrbloom/hdrbloom-scene.us", Attrs(), unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"u_hdr_image", &u_hdr_image},
		};
		loadShader(m_filterShader, "hdrbloom/hdrbloom-filter.us", Attrs(), unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"u_exposure",     &u_exposure    },
                        {"u_bloom_factor", &u_bloom_factor},
		        {"u_scene_factor", &u_scene_factor},
                        {"u_hdr_image",    &u_hdr_image   },
		        {"u_bloom_image",  &u_bloom_image },
		};
		loadShader(m_resolveShader, "hdrbloom/hdrbloom-resolve.us", Attrs(), unif_attrs);
	}
	// must be square
	auto tex_size = std::max(viewport(0).sx, viewport(0).sy);
	auto tex_viewport = Rectf(0, 0, tex_size, tex_size);
	// texture
	{
		alloc_texture(m_sceneTexture, GL_RGBA16F, tex_size, tex_size);
		alloc_texture(m_brightpassTexture, GL_RGBA16F, tex_size, tex_size);
		alloc_texture(m_filterTextures[0], GL_RGBA16F, tex_size, tex_size);
		alloc_texture(m_filterTextures[1], GL_RGBA16F, tex_size, tex_size);
		alloc_texture(m_depthTexture, GL_DEPTH_COMPONENT32F, tex_size, tex_size);
	}
	// filter frame
	{
		Attrs attrs = {
		        {"viewport0", tex_viewport            },
		        {"color0",    m_filterTextures[0].id()},
		};
		m_filterFrames[0].init(attrs);
	}
	{
		Attrs attrs = {
		        {"viewport0", tex_viewport            },
		        {"color0",    m_filterTextures[1].id()},
		};
		m_filterFrames[1].init(attrs);
	}
	// render frame
	{
		Attrs attrs = {
		        {"viewport0", viewport(0)             },
		        {"color0",    m_sceneTexture.id()     },
		        {"color1",    m_brightpassTexture.id()},
		        {"depth",     m_depthTexture.id()     },
		};
		m_renderFrame.init(attrs);
		Attrs bg_attrs = {
		        {"bgcolor0", c_black},
		        {"bgcolor1", c_black},
		        {"bgdepth",  1.0    },
		};
		m_renderFrame.set(bg_attrs);
	}
	{
		const float exposure_lut[20] = {11.0, 6.0,  3.2,  2.8,  2.2,  1.90, 1.80, 1.80, 1.70, 1.70,
		                                1.60, 1.60, 1.50, 1.50, 1.40, 1.40, 1.30, 1.20, 1.10, 1.00};

		Attrs attr = {
		        {"target",     GL_TEXTURE_1D   },
                        {"iformat",    GL_R32F         },
                        {"width",      20              },
		        {"min_filter", GL_LINEAR       },
                        {"mag_filter", GL_LINEAR       },
                        {"wrap_s",     GL_CLAMP_TO_EDGE},
		        {"data",       exposure_lut    },
		};
		m_lutTexture.init(attr);  // not used now
	}
	// object
	{
		const char *sym[] = {
		        "a.a_position",
		        "a.a_normal",
		        nullptr,
		};

		m_object.load("sphere.sbm", Attrs(), sym, m_renderShader.id());
		float ambient = 0.002;
		for (auto i = 0; i < e_sphere_count; i++) {
			float fi = 3.14159267 * i / 8.0;
			ub_material[i].diffuse_color
			        = {sinf(fi) * 0.5f + 0.5f, sinf(fi + 1.345) * 0.5f + 0.5f,
			           sinf(fi + 2.567) * 0.5f + 0.5f};
			ub_material[i].specular_color = Vec3f(2.8, 2.8, 2.9);
			ub_material[i].specular_power = 30.0;
			ub_material[i].ambient_color = Vec3f(ambient * 0.025);
			ambient *= 1.5;
		}
	}
}

void App::menu()
{
	ImGui::SliderFloat("exposure", &u_exposure, 0.5, 2.0);
	ImGui::SliderFloat("bloom factor", &u_bloom_factor, 0.0, 1.0);
	ImGui::SliderFloat("scene factor", &u_scene_factor, 0.0, 1.0);
	ImGui::SliderFloat("thresh min", &u_bloom_thresh_min, 0.0, 4.0);
	ImGui::SliderFloat("thresh max", &u_bloom_thresh_max, 0.0, 4.0);
	u_bloom_thresh_max = std::max(u_bloom_thresh_max, u_bloom_thresh_min);
}

void App::render()
{
	const auto t = getSeconds().current();
	m_renderFrame.begin();
	m_renderFrame.clear();
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LESS;
		renderstate.use();
		sb6::Composition composition;
		composition.perspective(viewport(0), 50.0, 1.0, 1000.0);
		ub_transforms.viewscreen = composition.viewscreen();
		ub_transforms.worldview = c_unit.trans({0.0, 0.0, -20.0});
		for (auto i = 0; i < e_sphere_count; i++) {
			auto fi = 3.141592f * i / 16.0f;
			auto r = (i & 2) ? 0.6f : 1.5f;
			ub_transforms.modelworld[i] = c_unit.trans(
			        {cosf(t + fi) * 5.0f * r, sinf(t + fi * 4.0) * 4.0f, sinf(t + fi) * 5.0f * r});
		}
		m_renderShader.use();
		m_object.draw(e_sphere_count);
		renderstate.flags.depth_test = false;
		renderstate.use();
	}
	m_renderFrame.end();
	m_filterFrames[0].begin();
	{
		u_hdr_image = m_brightpassTexture.id();
		m_filterShader.use();
		drawFullscreenQuad();
	}
	m_filterFrames[0].end();
	m_filterFrames[1].begin();
	{
		u_hdr_image = m_filterTextures[0].id();
		m_filterShader.use();
		drawFullscreenQuad();
	}
	m_filterFrames[1].end();

	{
		if (u_bloom_thresh_min > u_bloom_thresh_max) {
			u_bloom_thresh_min = (u_bloom_thresh_min + u_bloom_thresh_max) / 2 - 0.1;
			u_bloom_thresh_max = (u_bloom_thresh_min + u_bloom_thresh_max) / 2 + 0.1;
		}
		u_bloom_image = m_filterTextures[1].id();
		u_hdr_image = m_sceneTexture.id();
		m_resolveShader.use();
		drawFullscreenQuad();
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("hdrbloom");
}  // namespace spu::hdrbloom
