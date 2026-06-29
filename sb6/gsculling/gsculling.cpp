//
// App :
//
#include "base_app.h"
namespace spu::gsculling {
/* clang-format off */
const char *vert = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "// Incoming per vertex... position and normal                                          \n"
    "layout (location = 0) in vec4 vVertex;                                                 \n"
    "layout (location = 1) in vec3 vNormal;                                                 \n"
    "                                                                                       \n"
    "out Vertex                                                                             \n"
    "{                                                                                      \n"
    "    vec3 normal;                                                                       \n"
    "    vec4 color;                                                                        \n"
    "} vertex;                                                                              \n"
    "                                                                                       \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform mat4 u_modelview;                                                              \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    // Get surface normal in eye coordinates                                           \n"
    "    vec3 vEyeNormal = mat3(u_modelview) * normalize(vNormal);                          \n"
    "                                                                                       \n"
    "    // Get vertex position in eye coordinates                                          \n"
    "    vec4 vPosition4 = u_modelview * vVertex;                                           \n"
    "    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;                                   \n"
    "                                                                                       \n"
    "    // Get vector to light source                                                      \n"
    "    vec3 vLightDir = normalize(u_light_position - vPosition3);                         \n"
    "                                                                                       \n"
    "    // Dot product gives us diffuse intensity                                          \n"
    "    vertex.color = vec4(0.7, 0.6, 1.0, 1.0) * abs(dot(vEyeNormal, vLightDir));         \n"
    "                                                                                       \n"
    "    gl_Position = vVertex;                                                             \n"
    "    vertex.normal = vNormal;                                                           \n"
    "}                                                                                      \n"
};

const char *geom = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "layout (triangles) in;                                                                 \n"
    "layout (triangle_strip, max_vertices = 3) out;                                         \n"
    "                                                                                       \n"
    "in Vertex                                                                              \n"
    "{                                                                                      \n"
    "    vec3 normal;                                                                       \n"
    "    vec4 color;                                                                        \n"
    "} vertex[];                                                                            \n"
    "                                                                                       \n"
    "out vec4 color;                                                                        \n"
    "                                                                                       \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform mat4 u_modelscreen;                                                            \n"
    "uniform mat4 u_modelview;                                                              \n"
    "uniform vec3 u_viewpoint;                                                              \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    int n;                                                                             \n"
    "                                                                                       \n"
    "    vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;                     \n"
    "    vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;                     \n"
    "    vec3 normal = normalize(cross(ab, ac));                                            \n"
    "    vec3 transformed_normal = (mat3(u_modelview) * normal);                            \n"
    "    vec4 worldspace = /* u_modelview * */ gl_in[0].gl_Position;                        \n"
    "    vec3 vt = normalize(u_viewpoint - worldspace.xyz);                                 \n"
    "                                                                                       \n"
    "    if (dot(normal, vt) > 0.0) {                                                       \n"
    "        for (n = 0; n < 3; n++) {                                                      \n"
    "            gl_Position = u_modelscreen * gl_in[n].gl_Position;                        \n"
    "            color = vertex[n].color;                                                   \n"
    "            EmitVertex();                                                              \n"
    "        }                                                                              \n"
    "        EndPrimitive();                                                                \n"
    "    }                                                                                  \n"
    "}                                                                                      \n"
};

const char *frag = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "in vec4 color;                                                                         \n"
    "                                                                                       \n"
    "out vec4 output_color;                                                                 \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    output_color = color;                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	SpuShader m_shader;
	Vec3f u_viewpoint;
	Mat4f u_modelview;
	Mat4f u_modelscreen;
	Vec3f u_light_position = {-10.0, 40.0, 200.0};
	sb6::Object m_object;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	Attrs shader_attrs = {
	        {"vert", vert},
	        {"geom", geom},
	        {"frag", frag},
	};
	Attrs unif_attrs = {
	        {"u_modelview",   &u_modelview  },
	        {"u_modelscreen", &u_modelscreen},
	        {"u_viewpoint",   &u_viewpoint  },
	        {"u_light_position",   &u_light_position  },
	};
	loadShader(m_shader, shader_attrs, unif_attrs);
	m_object.load("bunny_1k.sbm");

	auto &renderstate = getRenderstate();
	renderstate.flags.cull_face = false;
	renderstate.flags.depth_test = true;
	renderstate.depth_func = GL_LEQUAL;
	// renderstate.use();
}

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	auto u_viewscreen = composition.viewscreen();
	auto u_modelview = c_unit.rot("XY", -t * 20.0, -t * 1.0).trans({0.0, 0.0, -1.5});
	u_modelscreen = u_viewscreen * u_modelview;
	//u_modelview = u_modelview;
	u_viewpoint = {sinf(t * 2.1) * 70.0f, cosf(t * 1.4) * 70.0f, sinf(t * 0.7) * 70.0f};
	m_shader.use();
	m_object.draw();
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gsculling");
}  // namespace spu::gsculling
