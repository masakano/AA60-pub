//
// App :
//
#include "base_app.h"
namespace spu::noise {
// #include <string>
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_shader;
	float u_time;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	Attrs unif_attrs = {
	        {"u_time", &u_time},
	};
	loadShader(m_shader, "noise/noise.us", Attrs(), unif_attrs);
}

void App::render()
{
	auto t = getSeconds().current();
	u_time = t * 0.00001;
	m_shader.use();
	drawFullscreenQuad();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("noise");
}  // namespace spu::noise
