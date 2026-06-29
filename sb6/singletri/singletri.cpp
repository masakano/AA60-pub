//
// App :
//
#include "base_app.h"
namespace spu::singletri {
/* clang-format off */
const char *vs_source = {
    "#version 420 core                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec4 vertices[] = vec4[](vec4( 0.25, -0.25, 0.5, 1.0),                       \n"
    "                                   vec4(-0.25, -0.25, 0.5, 1.0),                       \n"
    "                                   vec4( 0.25,  0.25, 0.5, 1.0));                      \n"
    "                                                                                       \n"
    "    gl_Position = vertices[gl_VertexID];                                               \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 420 core                                                                      \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(0.0, 0.8, 1.0, 1.0);                                                  \n"
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
	        {"frag", fs_source},
	        {"vert", vs_source},
	};
	loadShader(m_shader, shader_attrs, Attrs());
	m_array.init({
	        {"nelem", 4}
        });
}

void App::render()
{
	m_shader.use();
	m_array.draw(GL_TRIANGLES, 0, 3);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("singletri");
}  // namespace spu::singletri
