//
// App :
//
#include "base_app.h"
namespace spu::ktxview {
/* clang-format off */
const char *vert = {
    "#version 420 core                                                                      \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec4 vertices[] = vec4[](                                                    \n"
    "             vec4(-1.0, -1.0, 0.5, 1.0),                                               \n"
    "             vec4( 1.0, -1.0, 0.5, 1.0),                                               \n"
    "             vec4(-1.0,  1.0, 0.5, 1.0),                                               \n"
    "             vec4( 1.0,  1.0, 0.5, 1.0));                                              \n"
    "                                                                                       \n"
    "    gl_Position = vertices[gl_VertexID];                                               \n"
    "    f_texcoord = gl_Position.xy * 0.5 + 0.5;                                           \n"
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
    "   color = texture(u_texture, f_texcoord) * u_exposure;                                \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	SpuShader m_shader;
	uint32_t u_texture;
	float u_exposure;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	u_texture = sb6::ktx::load("Tree.ktx");
	// shader
	{
		Attrs shader_attrs = {
		        {"vert", vert},
		        {"frag", frag},
		};
		Attrs unif_attrs = {
		        {"u_texture",  &u_texture },
		        {"u_exposure", &u_exposure},
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
	}
}

void App::render()
{
	auto t = getSeconds().current();
	u_exposure = sinf(t) * 16.0 + 16.0;
	m_shader.use();
	drawFullscreenQuad();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("ktxview");
}  // namespace spu::ktxview
