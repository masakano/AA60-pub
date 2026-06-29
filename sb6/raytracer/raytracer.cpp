//
// App :
//
#include "base_app.h"
namespace spu::raytracer {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;

private:
	static constexpr auto c_max_recursion_depth = 5;

	struct Sphere {
		vec3f_t center;
		float radius;
		Vec4f color;
	};
	struct Plane {
		vec3f_t normal;
		float d;
	};
	struct Light {
		vec3f_t position;
		unsigned int32_t: 32;  // pad
	};
	enum DebugMode { e_none = 0, e_reflected, e_refracted, e_reflected_color, e_refracted_color };
	SpuShader m_prepareShader;
	SpuShader m_traceShader;
	SpuShader m_blitShader;

	std::vector<SpuFrame> m_rayFrames;
	SpuTexture m_compositeTexture;
	std::vector<SpuTexture> m_positionTextures;
	std::vector<SpuTexture> m_reflectedTextures;
	std::vector<SpuTexture> m_reflectionIntensityTextures;
	std::vector<SpuTexture> m_refractedTextures;
	std::vector<SpuTexture> m_refractionIntensityTextures;

	float m_maxDepth = 1;
	float m_debugDepth = 0;
	int32_t m_debugMode = 0;
	Sphere ub_spheres[128];
	Plane ub_planes[128];
	Light ub_lights[128];
	Mat4f u_ray_lookat;
	Vec3f u_ray_origin;
	float u_aspect = 1.0;
	uint32_t u_orign_texture = 0;
	uint32_t u_direction_texture = 0;
	uint32_t u_color_texture = 0;
	uint32_t u_composite_texture = 0;

	int32_t u_num_spheres = 7;
	int32_t u_num_planes = 6;
	int32_t u_num_lights = 5;

	Vec3f u_direction_scale = Vec3f(1.9, 1.9, 1.0);
	Vec3f u_direction_bias = Vec3f(0.0, 0.0, 0.0);
	uint32_t selectTexture();
	void recurse(int32_t depth);
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs unif_attrs = {
		        {"u_ray_origin",      &u_ray_origin     },
		        {"u_ray_lookat",      &u_ray_lookat     },
		        {"u_aspect",          &u_aspect         },
		        {"u_direction_scale", &u_direction_scale},
		        {"u_direction_bias",  &u_direction_bias },
		};
		loadShader(m_prepareShader, "raytracer/trace-prepare.us", Attrs(), unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"UB_SPHERES",          ub_spheres          },
		        {"UB_PLANES",           ub_planes           },
		        {"UB_LIGHTS",           ub_lights           },
		        {"u_origin_texture",    &u_orign_texture    },
		        {"u_direction_texture", &u_direction_texture},
		        {"u_color_texture",     &u_color_texture    },
		        {"u_num_spheres",       &u_num_spheres      },
		        {"u_num_planes",        &u_num_planes       },
		        {"u_num_lights",        &u_num_lights       },
		};
		loadShader(m_traceShader, "raytracer/raytracer.us", Attrs(), unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"u_composite_texture", &u_composite_texture},
		};
		loadShader(m_blitShader, "raytracer/blit.us", Attrs(), unif_attrs);
	}

	Attrs texture_attrs = {
	        {"target",  GL_TEXTURE_2D          },
	        {"iformat", GL_RGB16F              },
	        {"width",   int32_t(viewport(0).sx)},
	        {"height",  int32_t(viewport(0).sy)},
	};
	m_compositeTexture.init(texture_attrs);

	m_rayFrames.resize(c_max_recursion_depth);
	m_positionTextures.resize(c_max_recursion_depth);
	m_reflectedTextures.resize(c_max_recursion_depth);
	m_reflectionIntensityTextures.resize(c_max_recursion_depth);
	m_refractedTextures.resize(c_max_recursion_depth);
	m_refractionIntensityTextures.resize(c_max_recursion_depth);

	// frame buffer
	for (auto i = 0; i < c_max_recursion_depth; i++) {
		Attrs position_texture_attrs = {
		        {"target",     GL_TEXTURE_2D          },
                        {"iformat",    GL_RGB32F              },
		        {"width",      int32_t(viewport(0).sx)},
                        {"height",     int32_t(viewport(0).sy)},
		        {"min_filter", GL_NEAREST             },
                        {"mag_filter", GL_NEAREST             },
		};
		m_positionTextures[i].init(position_texture_attrs);

		Attrs texture_attrs = {
		        {"target",     GL_TEXTURE_2D          },
                        {"iformat",    GL_RGB16F              },
		        {"width",      int32_t(viewport(0).sx)},
                        {"height",     int32_t(viewport(0).sy)},
		        {"min_filter", GL_NEAREST             },
                        {"mag_filter", GL_NEAREST             },
		};
		m_reflectedTextures[i].init(texture_attrs);
		m_refractedTextures[i].init(texture_attrs);
		m_reflectionIntensityTextures[i].init(texture_attrs);
		m_refractionIntensityTextures[i].init(texture_attrs);

		Attrs frame_attrs = {
		        {"viewport0", viewport(0)                          },
		        {"color0",    m_compositeTexture.id()              },
		        {"color1",    m_positionTextures[i].id()           },
		        {"color2",    m_reflectedTextures[i].id()          },
		        {"color3",    m_refractedTextures[i].id()          },
		        {"color4",    m_reflectionIntensityTextures[i].id()},
		        {"color5",    m_refractionIntensityTextures[i].id()},
		};
		m_rayFrames[i].init(frame_attrs);
	}
}

void App::menu()
{
	ImGui::Combo(
	        "debug mode", &m_debugMode, "none\0reflected\0refracted\0reflect_color\0refracted_color\0\0");
	ImGui::SliderFloat("max depth", &m_maxDepth, 1.0, 4.9);
	ImGui::SliderFloat("debug depth", &m_debugDepth, 0.0, 4.9);
}

void App::render()
{
	auto f = getSeconds().current();
	u_ray_origin = {sinf(f * 0.3234) * 28.0f, cosf(f * 0.4234) * 28.0f, cosf(f * 0.1234) * 28.0f};

	Vec3f lookat_point = {sinf(f * 0.214) * 8.0f, cosf(f * 0.153) * 8.0f, sinf(f * 0.734) * 8.0f};

	sb6::Composition composition;
	composition.lookat(u_ray_origin, lookat_point, ey());
	u_ray_lookat = composition.worldview();

	for (auto i = 0; i < 128; i++) {
		auto fi = i / 128.0f;
		ub_spheres[i].center
		        = {sinf(fi * 123.0 + f) * 15.75f, cosf(fi * 456.0 + f) * 15.75f,
		           (sinf(fi * 300.0 + f) * cosf(fi * 200.0 + f)) * 20.0f};

		ub_spheres[i].radius = fi * 2.3 + 3.5;

		auto r = fi * 61.0f;
		auto g = r + 0.25f;
		auto b = g + 0.25f;

		r = (r - floorf(r)) * 0.8 + 0.2;
		g = (g - floorf(g)) * 0.8 + 0.2;
		b = (b - floorf(b)) * 0.8 + 0.2;
		ub_spheres[i].color = {r, g, b, 1.0};
	}
	ub_planes[0] = {
	        {+0.0, +0.0, -1.0},
                30.0
        };
	ub_planes[1] = {
	        {+0.0, +0.0, +1.0},
                30.0
        };
	ub_planes[2] = {
	        {-1.0, +0.0, +0.0},
                30.0
        };
	ub_planes[3] = {
	        {+1.0, +0.0, +0.0},
                30.0
        };
	ub_planes[4] = {
	        {+0.0, -1.0, +0.0},
                30.0
        };
	ub_planes[5] = {
	        {+0.0, +1.0, +0.0},
                30.0
        };

	for (auto i = 0; i < 128; i++) {
		auto fi = 3.33f - i;  //  / 35.0;
		ub_lights[i].position
		        = {sinf(fi * 2.0 - f) * 15.75f, cosf(fi * 5.0 - f) * 5.75f,
		           (sinf(fi * 3.0 - f) * cosf(fi * 2.5 - f)) * 19.4f};
	}
	m_rayFrames[0].begin();

	u_aspect = viewport(0).sy / viewport(0).sx;  // not sx / sy

	m_prepareShader.use();
	drawFullscreenQuad();
	m_traceShader.use();  // sometimes fail (with -message_level 1)
	recurse(0);
	m_rayFrames[0].end();
	u_composite_texture = selectTexture();
	m_blitShader.use();
	drawFullscreenQuad();
	m_debugDepth = std::min(m_debugDepth, m_maxDepth - 1);
}

uint32_t App::selectTexture()
{
	switch (m_debugMode) {
	case e_none: return m_compositeTexture.id();
	case e_reflected: return m_reflectedTextures[int32_t(m_debugDepth)].id();
	case e_refracted: return m_refractedTextures[int32_t(m_debugDepth)].id();
	case e_reflected_color: return m_reflectionIntensityTextures[int32_t(m_debugDepth)].id();
	case e_refracted_color: return m_refractionIntensityTextures[int32_t(m_debugDepth)].id();
	default: assert(0);
	}
}

void App::recurse(int32_t depth)
{
	SpuScopedRenderstate renderstate(1);
	assert(depth + 1 < c_max_recursion_depth);
	m_rayFrames[depth + 1].begin();
	renderstate.blend_func_channel = 0;
	renderstate.blend_func = {
	        GL_ONE,
	        GL_ONE,
	        GL_ONE,
	        GL_ONE,
	};
	renderstate.flags.blend = true;
	renderstate.use();
	u_orign_texture = m_positionTextures[depth].id();
	u_direction_texture = m_reflectedTextures[depth].id();
	u_color_texture = m_reflectionIntensityTextures[depth].id();
	// Render
	m_traceShader.use();
	drawFullscreenQuad();
	if (depth != (int32_t(m_maxDepth) - 1)) {
		recurse(depth + 1);
	}
	m_rayFrames[depth + 1].end();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("raytracer");
}  // namespace spu::raytracer
