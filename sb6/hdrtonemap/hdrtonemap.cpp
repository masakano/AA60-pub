//
// App :
//
#include "base_app.h"
namespace spu::hdrtonemap {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;

	SpuShader m_naiveShader;
	SpuShader m_adaptiveShader;
	SpuShader m_exposureShader;
	float u_exposure = 1.0;
	uint32_t u_hdr_image = 0;
	int32_t m_mode = 0;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shaders
	{
		Attrs unif_attrs = {
		        {"u_hdr_image", &u_hdr_image},
		};
		loadShader(m_naiveShader, "hdrtonemap/tonemap_naive.us", Attrs(), unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"u_hdr_image", &u_hdr_image},
		};
		loadShader(m_adaptiveShader, "hdrtonemap/tonemap_adaptive.us", Attrs(), unif_attrs);
	}
	{
		Attrs unif_attrs = {
		        {"u_exposure",  &u_exposure },
		        {"u_hdr_image", &u_hdr_image},
		};
		loadShader(m_exposureShader, "hdrtonemap/tonemap_exposure.us", Attrs(), unif_attrs);
	}
	u_hdr_image = sb6::ktx::load("treelights_2k.ktx");
	{
	}
}

void App::menu()
{
	ImGui::Combo("type", &m_mode, "naive\0exposure\0adaptive\0\0");
	if (m_mode == 1) {
		ImGui::SliderFloat("exposure", &u_exposure, 0.0, 10.0);
	}
}

void App::render()
{
	switch (m_mode) {
	case 0: m_naiveShader.use(); break;
	case 1: m_exposureShader.use(); break;
	case 2: m_adaptiveShader.use(); break;
	}
	drawFullscreenQuad();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("hdrtonemap");
}  // namespace spu::hdrtonemap
