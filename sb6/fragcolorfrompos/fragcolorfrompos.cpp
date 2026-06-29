//
// App :
//
#include "base_app.h"
namespace spu {
/* clang-format on */
#ifndef INTERPOLATE_COLOR
const char *vert
        = {"#version 420 core                                         	                  \n"
           "in float position;                                                               \n"
           "void main()                                                                  \n"
           "{                                                                                \n"
           "    const vec4 vertices[] = vec4[](vec4( 0.25, -0.25, 0.5, 1.0),                 \n"
           "                                   vec4(-0.25, -0.25, 0.5, 1.0),                 \n"
           "                                   vec4( 0.25,  0.25, 0.5, 1.0));                \n"
           "                                                                                 \n"
           "    gl_Position = vertices[gl_VertexID] + position;                              \n"
           "}                                                                                \n"};

const char *frag
        = {"#version 420 core                                                                \n"
           "out vec4 color;                                                                  \n"
           "void main()                                                                      \n"
           "{                                                                                \n"
           "    color = vec4(sin(gl_FragCoord.x * 0.25) * 0.5 + 0.5,                         \n"
           "                 cos(gl_FragCoord.y * 0.25) * 0.5 + 0.5,                         \n"
           "                 sin(gl_FragCoord.x * 0.15) * cos(gl_FragCoord.y * 0.1),         \n"
           "                 1.0);                                                           \n"
           "}                                                                                \n"};

#else
const char *vert
        = {"#version 420 core                                                                \n"
           "in float position;                                                               \n"
           "out vec4 vs_color;                                                               \n"
           "void main()                                                                      \n"
           "{                                                                                \n"
           "    const vec4 vertices[] = vec4[](vec4( 0.25, -0.25, 0.5, 1.0),                 \n"
           "                                   vec4(-0.25, -0.25, 0.5, 1.0),                 \n"
           "                                   vec4( 0.25,  0.25, 0.5, 1.0));                \n"
           "    const vec4 colors[] = vec4[](vec4(1.0, 0.0, 0.0, 1.0),                       \n"
           "                                 vec4(0.0, 1.0, 0.0, 1.0),                       \n"
           "                                 vec4(0.0, 0.0, 1.0, 1.0));                      \n"
           "                                                                                 \n"
           "    gl_Position = vertices[gl_VertexID] + position;                              \n"
           "    vs_color = colors[gl_VertexID];                                              \n"
           "}                                                                                \n"};

const char *frag
        = {"#version 420 core                                                                \n"
           "in vec4 vs_color;                                                                \n"
           "out vec4 color;                                                                  \n"
           "void main()                                                                      \n"
           "{                                                                                \n"
           "    color = vs_color;                                                            \n"
           "}                                                                                \n"};

#endif
/* clang-format off */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
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
		{"shader_id", m_shader.id()},
		{"a.position", 1},
	};
	int32_t dummy_vert[] = {0, 0, 0, 0};
	m_array.init(array_attrs);
	m_array.send(dummy_vert, 4);
	spu_frame_set(-1, "bgcolor0", c_green);

}

void App::render()
{
	m_shader.use();
	m_array.draw(GL_TRIANGLES, 0, 3);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("fragcolorfrompos");
} // spu
