//
// App :
//
#include "base_app.h"
namespace spu::mirrorclampedge {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;

private:
	SpuShader m_shader;
	uint32_t u_tex;
	int32_t m_wrapMode = GL_REPEAT;
	int32_t m_wrapModeIndex = 3;  // REPEAT
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	u_tex = sb6::ktx::load("flare.ktx");
	Attrs unif_attrs = {
	        {"tex", &u_tex},
	};
	loadShader(m_shader, "mirrorclampedge/drawquad.us", Attrs(), unif_attrs);
}

void App::menu()
{
	ImGui::Combo(
	        "mode", &m_wrapModeIndex, "CLAMP_TO_BORDER\0MIRROR_CLAMP_TO_EDGE\0CLAMP_TO_EDGE\0REPEAT\0\0");

	const int32_t wrap_indices[] = {
	        GL_CLAMP_TO_BORDER,
	        GL_MIRROR_CLAMP_TO_EDGE,
	        GL_CLAMP_TO_EDGE,
	        GL_REPEAT,
	};
	m_wrapMode = wrap_indices[m_wrapModeIndex];
}

void App::render()
{
	Attrs attrs = {
	        {"wrap_s", m_wrapMode},
	        {"wrap_t", m_wrapMode},
	};
	spu_texture_set(u_tex, attrs);
	m_shader.use();
	drawFullscreenQuad();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("mirrorclampedge");
}  // namespace spu::mirrorclampedge
