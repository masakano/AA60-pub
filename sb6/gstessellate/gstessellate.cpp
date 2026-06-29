//
// App :
//
#include "base_app.h"
namespace spu::gstessellate {
/* clang-format off */
const char *vs_source = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "// Incoming per vertex... position and normal                                          \n"
    "in vec4 vVertex;                                                                       \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vVertex;                                                             \n"
    "}                                                                                      \n"
};

const char *gs_source = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "                                                                                       \n"
    "layout (triangles) in;                                                                 \n"
    "layout (triangle_strip, max_vertices = 12) out;                                        \n"
    "                                                                                       \n"
    "uniform float stretch = 0.7;                                                           \n"
    "                                                                                       \n"
    "flat out vec4 color;                                                                   \n"
    "                                                                                       \n"
    "uniform mat4 u_modelscreen;                                                            \n"
    "uniform mat4 u_modelview;                                                              \n"
    "                                                                                       \n"
    "void make_face(vec3 a, vec3 b, vec3 c)                                                 \n"
    "{                                                                                      \n"
    "    vec3 face_normal = normalize(cross(c - a, c - b));                                 \n"
    "    vec4 face_color = vec4(1.0, 0.2, 0.4, 1.0) * (mat3(u_modelview) * face_normal).z;  \n"
    "    gl_Position = u_modelscreen * vec4(a, 1.0);                                        \n"
    "    color = face_color;                                                                \n"
    "    EmitVertex();                                                                      \n"
    "                                                                                       \n"
    "    gl_Position = u_modelscreen * vec4(b, 1.0);                                        \n"
    "    color = face_color;                                                                \n"
    "    EmitVertex();                                                                      \n"
    "                                                                                       \n"
    "    gl_Position = u_modelscreen * vec4(c, 1.0);                                        \n"
    "    color = face_color;                                                                \n"
    "    EmitVertex();                                                                      \n"
    "                                                                                       \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    int n;                                                                             \n"
    "    vec3 a = gl_in[0].gl_Position.xyz;                                                 \n"
    "    vec3 b = gl_in[1].gl_Position.xyz;                                                 \n"
    "    vec3 c = gl_in[2].gl_Position.xyz;                                                 \n"
    "                                                                                       \n"
    "    vec3 d = (a + b) * stretch;                                                        \n"
    "    vec3 e = (b + c) * stretch;                                                        \n"
    "    vec3 f = (c + a) * stretch;                                                        \n"
    "                                                                                       \n"
    "    a *= (2.0 - stretch);                                                              \n"
    "    b *= (2.0 - stretch);                                                              \n"
    "    c *= (2.0 - stretch);                                                              \n"
    "    make_face(a, d, f);                                                                \n"
    "    make_face(d, b, e);                                                                \n"
    "    make_face(e, c, f);                                                                \n"
    "    make_face(d, e, f);                                                                \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 410 core                                                                      \n"
    "                                                                                       \n"
    "flat in vec4 color;                                                                    \n"
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
	SpuArray m_array;

	Mat4f u_modelview;
	Mat4f u_modelscreen;
	float u_stretch;
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
	        {"u_modelview",   &u_modelview  },
	        {"u_modelscreen", &u_modelscreen},
	        {"stretch",       &u_stretch    },
	};
	loadShader(m_shader, shader_attrs, unif_attrs);
	const float vertices[]
	        = {0.000, 0.000, 1.000, 0.943, 0.000, -0.333, -0.471, 0.816, -0.333, -0.471, -0.816, -0.333};
	const uint16_t indices[] = {0, 1, 2, 0, 2, 3, 0, 3, 1, 3, 2, 1};
	Attrs array_attrs = {
	        {"a.0", 3},
	};
	m_array.init(array_attrs);
	m_array.send(vertices, 4);
	m_array.send(indices, 12, -1, 2);
	auto &renderstate = getRenderstate();
	renderstate.flags.cull_face = true;
	renderstate.flags.depth_test = true;
	renderstate.depth_func = GL_LEQUAL;
	// renderstate.use();
}

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	auto viewscreen = composition.viewscreen();

	u_modelview = c_unit.rot("XY", -t * 10.0, -t * 71.0).trans({0.0, 0.0, -10.5});
	u_modelscreen = viewscreen * u_modelview;
	u_stretch = sinf(t * 3.7) * 0.75 + 1.0;
	m_shader.use();
	m_array.draw(GL_TRIANGLES, 0, 12);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gstessellate");
}  // namespace spu::gstessellate
