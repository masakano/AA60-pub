//
// App :
//
#include "base_app.h"
namespace spu::dflandscape {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	void loadShaders();

private:
	SpuArray m_array;
	SpuShader m_shader;
	Mat4f u_uv_transform;
	Mat4f u_modelscreen;
	uint32_t u_map_texture;
	uint32_t u_grass_texture;
	uint32_t u_rock_texture;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs unif_attrs = {
		        {"u_uv_transform",  &u_uv_transform },
		        {"u_modelscreen",   &u_modelscreen  },
		        {"u_map_texture",   &u_map_texture  },
		        {"u_grass_texture", &u_grass_texture},
		        {"u_rock_texture",  &u_rock_texture }
                };
		loadShader(m_shader, "dflandscape/dflandscape.us", Attrs(), unif_attrs);
	}
	// texture
	{
		Attrs tex_attrs = {
		        {"mag_filter",  GL_LINEAR              },
		        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
		        {"auto_mipmap", 1                      },
		};
		u_map_texture = sb6::ktx::load("psycho-map-df-sm.ktx", tex_attrs);
		u_grass_texture = sb6::ktx::load("mossygrass.ktx", tex_attrs);
		u_rock_texture = sb6::ktx::load("rocks.ktx", tex_attrs);
	}
	// array
	m_array.init({
	        {"nelem", 4}
        });
}

void App::render()
{
	auto t = getSeconds().current();
	auto scale = cosf(t * 0.20f) * sinf(t * 0.15f) * 3.0f + 3.2f;
	auto cos_t = cosf(t * 0.03f) * 0.25f;
	auto sin_t = sinf(t * 0.02f) * 0.25f;

	u_uv_transform = {
	        {cos_t * scale,  sin_t * scale, 0.0, 0.0},
	        {-sin_t * scale, cos_t * scale, 0.0, 0.0},
	        {cos_t,          sin_t,         1.0, 0.0},
	        {0.0,            0.0,           0.0, 1.0},
	};
	auto modelview = c_unit.scale({10.0, 10.0, 1.0}).trans({0.0, 0.0, -1.0});
	sb6::Composition composition;
	composition.frustum(viewport(0), 1.0, 100.0);
	auto viescreen = composition.viewscreen();

	u_modelscreen = viescreen * modelview;
	m_shader.use();
	m_array.draw(GL_TRIANGLE_STRIP, 0, 4, 1);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("dflandscape");
}  // namespace spu::dflandscape
