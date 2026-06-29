//
// App :
//
#include "math.h"
#include "base_app.h"
namespace spu::movingtri {
/* clang-format off */
const char *vert = {
    "#version 410 core                                                                      \n"
    "layout (location = 0) in vec4 offset;                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec4 vertices[] = vec4[](vec4( 0.25, -0.25, 0.5, 1.0),                       \n"
    "                                   vec4(-0.25, -0.25, 0.5, 1.0),                       \n"
    "                                   vec4( 0.25,  0.25, 0.5, 1.0));                      \n"
    "                                                                                       \n"
    "    // Add 'offset' to our hard-coded vertex positio                                   \n"
    "    gl_Position = vertices[gl_VertexID] + offset;                                      \n"
    "}                                                                                      \n"
};

const char *frag = {
    "#version 410 core                                                                      \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(0.0, 0.8, 1.0, 0.0);                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_shader;
	SpuArray m_array;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	Attrs shader_attrs = {
	        {"vert", vert},
	        {"frag", frag},
	};
	loadShader(m_shader, shader_attrs, Attrs());
	Attrs array_attrs = {
	        {"a.0", 4},
	};
	m_array.init(array_attrs);
}

void App::render()
{
	auto t = getSeconds().current();
	Vec4f offset = {sinf(t) * 0.5f, cosf(t) * 0.6f, 0.0f, 0.0f};
	Vec4f attrib[4] = {
	        offset,
	        offset,
	        offset,
	        offset,
	};
	m_shader.use();
	m_array.send(attrib, 4);
	m_array.draw(GL_TRIANGLES, 0, 3);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("movingtri");
}  // namespace spu::movingtri
