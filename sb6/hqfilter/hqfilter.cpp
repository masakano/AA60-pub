//
// App :
//
#include <unistd.h>
#include "base_app.h"
namespace spu::hqfilter {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;

protected:
	SpuShader m_shader;
	uint32_t u_tex;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	Attrs unif_attrs = {
	        {"tex", &u_tex},
	};
	loadShader(m_shader, "hqfilter/hqfilter.us", Attrs(), unif_attrs);
	u_tex = sb6::ktx::load("baboon.ktx");
}

void App::render()
{
	m_shader.use();
	drawFullscreenQuad();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("hqfilter");
}  // namespace spu::hqfilter
