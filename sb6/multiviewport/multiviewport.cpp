//
// App :
//
#include "base_app.h"
namespace spu::multiviewport {
/* clang-format off */
const char *vs_source = {
    "#version 420 core                                                                      \n"
    "in vec4 position;                                                                      \n"
    "out VS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} vs_out;                                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = position;                                                            \n"
    "    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5,0.0);                           \n"
    "}                                                                                      \n"
};

const char *gs_source = {
    "#version 420 core                                                                      \n"
    "layout (triangles, invocations = 5) in;                                                \n"
    "layout (triangle_strip, max_vertices = 3) out;                                         \n"
    "layout (std140, binding = 0) uniform UB_TRANSFORM                                      \n"
    "{                                                                                      \n"
    "    mat4 u_modelscreen[5];                                                             \n"
    "};                                                                                     \n"
    "in VS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} gs_in[];                                                                             \n"
    "out GS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} gs_out;                                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    for (int i = 0; i < gl_in.length(); i++)                                           \n"
    "    {                                                                                  \n"
    "        gs_out.color = gs_in[i].color;                                                 \n"
    "        gl_Position = u_modelscreen[gl_InvocationID] *                                 \n"
    "                      gl_in[i].gl_Position;                                            \n"
    "        gl_ViewportIndex = gl_InvocationID;                                            \n"
    "        EmitVertex();                                                                  \n"
    "    }                                                                                  \n"
    "    EndPrimitive();                                                                    \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 420 core                                                                      \n"
    "out vec4 color;                                                                        \n"
    "in GS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} fs_in;                                                                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = fs_in.color;                                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuArray m_array;
	SpuShader m_shader;
	Mat4f ub_transform[5];
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs shader_attrs = {
		        {"frag", fs_source},
		        {"vert", vs_source},
		        {"geom", gs_source},
		};
		Attrs unif_attrs = {
		        {"UB_TRANSFORM", ub_transform},
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
	}
	// array
	{
		const uint16_t indices[] = {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7,
		                            6, 7, 0, 0, 7, 1, 6, 0, 2, 2, 4, 6, 7, 5, 3, 7, 3, 1};
		const float positions[] = {
		        -0.25, -0.25, -0.25, -0.25, 0.25, -0.25, 0.25,  -0.25, -0.25, 0.25,  0.25, -0.25,
		        0.25,  -0.25, 0.25,  0.25,  0.25, 0.25,  -0.25, -0.25, 0.25,  -0.25, 0.25, 0.25,
		};
		Attrs array_attrs = {
		        {"a.0", 3},
		};
		m_array.init(array_attrs);
		m_array.send(positions, 8);
		m_array.send(indices, 36, -1, 2);
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.cull_face = true;
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
		// renderstate.use();
	}
}

void App::render()
{
	auto t = getSeconds().current();
	// viewport
	{
		auto wx = viewport(0).sx;
		auto wy = viewport(0).sy;
		auto sx = 7 * wx / 16.0f;
		auto sy = 7 * wy / 16.0f;

		Vec4f viewports[] = {
		        {0,       0,       sx, sy},
		        {wx - sx, 0,       sx, sy},
		        {0,       wy - sy, sx, sy},
		        {wx - sx, wy - sy, sx, sy},
		};
		Attrs attrs = {
		        {"viewport1", viewports[0]},
		        {"viewport2", viewports[1]},
		        {"viewport3", viewports[2]},
		        {"viewport4", viewports[3]},
		};
		BaseApp::set(attrs);

		sb6::Composition composition;
		composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
		auto u_viewscreen = composition.viewscreen();
		for (auto i = 0; i < 5; i++) {
			ub_transform[i] = u_viewscreen
			                * c_unit.rot("XY", t * 81.0 * float(i + 1), t * 45.0 * float(i + 1))
			                          .trans({0.0, 0.0, -2.0});
		}
	}
	// draw
	{
		m_shader.use();
		m_array.draw(GL_TRIANGLES, 0, 36);
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("multiviewport");
}  // namespace spu::multiviewport
