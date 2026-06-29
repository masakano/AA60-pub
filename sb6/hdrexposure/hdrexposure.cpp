//
// App :
//
#include "base_app.h"
namespace spu::hdrexposure {
/* clang-format off */
const char *vert = {
    "#version 420 core                                                                      \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec4 vertices[] = vec4[](vec4(-1.0, -1.0, 0.5, 1.0),                         \n"
    "                                   vec4( 1.0, -1.0, 0.5, 1.0),                         \n"
    "                                   vec4(-1.0,  1.0, 0.5, 1.0),                         \n"
    "                                   vec4( 1.0,  1.0, 0.5, 1.0));                        \n"
    "                                                                                       \n"
    "    gl_Position = vertices[gl_VertexID];                                               \n"
    "    f_texcoord =  vertices[gl_VertexID].xy * 0.5 + 0.5;                                \n"
    "}                                                                                      \n"
};

const char *frag = {
    "#version 430 core                                                                      \n"
    "uniform sampler2D u_texture;                                                           \n"
    "uniform float u_exposure;                                                              \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 c = texture(u_texture, f_texcoord);                                           \n"
    "    c.xyz = vec3(1.0) - exp(-c.xyz * u_exposure);                                      \n"
    "    color = c;                                                                         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;

	float u_exposure = 1.0;

	uint32_t u_texture;
	SpuShader m_shader;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// texture
	u_texture = sb6::ktx::load("treelights_2k.ktx");
	// shader
	{
		Attrs shader_attrs = {
		        {"vert", vert},
		        {"frag", frag},
		};
		Attrs unif_attrs = {
		        {"u_exposure", &u_exposure},
		        {"u_texture",  &u_texture },
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
	}
}

void App::menu() { ImGui::SliderFloat("exposure", &u_exposure, 0.1, 10.0); }

void App::render()
{
	m_shader.use();
	drawFullscreenQuad();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("hdrexposure");
}  // namespace spu::hdrexposure
