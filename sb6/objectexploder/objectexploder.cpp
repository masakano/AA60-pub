//
// App :
//
#include "base_app.h"
namespace spu::objectexploder {
/* clang-format off */
const char *vs_source = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "layout (location = 0) in vec4 position;                                                \n"
    "layout (location = 1) in vec3 normal;                                                  \n"
    "                                                                                       \n"
    "out VS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec3 normal;                                                                       \n"
    "    vec4 color;                                                                        \n"
    "} vs_out;                                                                              \n"
    "                                                                                       \n"
    "uniform mat4 u_modelview;                                                              \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_viewscreen * u_modelview * position;                               \n"
    "    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);                          \n"
    "    vs_out.normal = normalize(mat3(u_modelview) * normal);                             \n"
    "}                                                                                      \n"
};

const char *gs_source = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "layout (triangles) in;                                                                 \n"
    "layout (triangle_strip, max_vertices = 3) out;                                         \n"
    "                                                                                       \n"
    "in VS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec3 normal;                                                                       \n"
    "    vec4 color;                                                                        \n"
    "} gs_in[];                                                                             \n"
    "                                                                                       \n"
    "out GS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec3 normal;                                                                       \n"
    "    vec4 color;                                                                        \n"
    "} gs_out;                                                                              \n"
    "                                                                                       \n"
    "uniform float u_explode_factor;                                                        \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;                     \n"
    "    vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;                     \n"
    "    vec3 face_normal = -normalize(cross(ab, ac));                                      \n"
    "    for (int i = 0; i < gl_in.length(); i++)                                           \n"
    "    {                                                                                  \n"
    "        gl_Position = gl_in[i].gl_Position + vec4(face_normal * u_explode_factor, 0.0);\n"
    "        gs_out.normal = gs_in[i].normal;                                               \n"
    "        gs_out.color = gs_in[i].color;                                                 \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "out vec4 color;                                                                        \n"
    "                                                                                       \n"
    "in GS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec3 normal;                                                                       \n"
    "    vec4 color;                                                                        \n"
    "} fs_in;                                                                               \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0) * abs(normalize(fs_in.normal).z);                                \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_shader;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	float u_explode_factor = 0.2;
	sb6::Object m_object;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	Attrs shader_attrs = {
	        {"frag", fs_source},
	        {"vert", vs_source},
	        {"geom", gs_source},
	};
	Attrs unif_attrs = {
	        {"u_modelview",      &u_modelview     },
	        {"u_viewscreen",     &u_viewscreen    },
	        {"u_explode_factor", &u_explode_factor},
	};
	loadShader(m_shader, shader_attrs, unif_attrs);
	m_object.load("bunny_1k.sbm");
	auto &renderstate = getRenderstate();
	renderstate.flags.depth_test = true;
	renderstate.depth_func = GL_LEQUAL;
	// renderstate.use();
}

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	u_modelview = c_unit.rot("XY", -t * 81.0, -t * 45.0).trans({0.0, 0.0, -3.0});
	u_explode_factor = sinf(t * 8.0) * cosf(t * 6.0) * 0.7 + 0.1;
	m_shader.use();
	m_object.draw();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("objectexploder");
}  // namespace spu::objectexploder
