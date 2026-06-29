//
// App :
//
#include "base_app.h"
namespace spu::sdfdemo {
class App : public BaseApp {
public:
	enum RENDER_MODE { e_mode_logo = 0, e_mode_text };
	SpuShader m_sdfShader;
	SpuArray m_array;
	uint32_t m_texture = 0;
	uint32_t m_logoTexture = 0;
	int32_t m_mode = 0;
	std::vector<float> u_uv_transform;
	Mat4f u_modelscreen;
	uint32_t u_texture;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// texture
	{
		Attrs attrs = {
		        {"mag_filter", GL_LINEAR              },
		        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
		};
		m_logoTexture = sb6::ktx::load("gllogodistsmarray.ktx", attrs);
		m_texture = sb6::ktx::load("chars-df-array.ktx", attrs);
	}
	// shader
	{
		u_uv_transform = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
		Attrs unif_attrs = {
		        {"u_uv_transform", u_uv_transform.data()},
		        {"u_modelscreen",  &u_modelscreen       },
		        {"u_texture",      &u_texture           },
		};
		loadShader(m_sdfShader, "sdfdemo/sdf.us", Attrs(), unif_attrs);
	}
	// array
	m_array.init({
	        {"nelem", 4}
        });
}

void App::menu() { ImGui::Combo("mode", &m_mode, "log\0text\0\0"); }

void App::render()
{
	auto t = getSeconds().current();
	auto scale = cosf(t * 0.2f) * sinf(t * 0.15f) * 6.0f + 3.001f;
	auto cos_t = cosf(t) * scale;
	auto sin_t = sinf(t) * scale;
	auto num_chars = 1;
	sb6::Composition composition;
	composition.frustum(viewport(0), 1.0, 100.0);
	auto viewscreen = composition.viewscreen();

	switch (m_mode) {
	case e_mode_logo: {
		num_chars = 1;
		u_texture = m_logoTexture;
		auto modelview = c_unit.scale({6.0, 6.0, 1.0})
		                         .rot("YX", sin_t * 15.0, cos_t * 12.0)
		                         .trans({0.0f, 0.0f, -6.0f + cos_t * 0.25f});

		u_modelscreen = viewscreen * modelview;
		break;
	}
	case e_mode_text: {
		num_chars = 24;
		u_texture = m_texture;
		auto tz = -10.0f;
		auto rs = 4.0f;
		auto modelview = c_unit.trans({-float((num_chars - 1)) + sin_t, 0.0, 0.0})
		                         .rot("ZX", sin_t * 3.0 * rs, cos_t * 4.0 * rs)
		                         .trans({0.0, 0.0, tz + cos_t});

		u_modelscreen = viewscreen * modelview;
		break;
	}
	}
	m_sdfShader.use();
	m_array.draw(GL_TRIANGLE_STRIP, 0, 4, num_chars);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("sdfdemo");
}  // namespace spu::sdfdemo
