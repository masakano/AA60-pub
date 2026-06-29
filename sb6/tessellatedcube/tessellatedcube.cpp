//
// App :
//
#include "base_app.h"
namespace spu::tessellatedcube {
/* clang-format off */
const char *vs_source = {
    "#version 420 core                                                                      \n"
    "in vec4 position;                                                                      \n"
    "out VS_OUT                                                                             \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} vs_out;                                                                              \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = position;                                                            \n"
    "   vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);                           \n"
    " }                                                                                     \n"
};

const char *tcs_source = {
    "#version 420 core                                                                      \n"
    "layout (vertices = 4) out;                                                             \n"
    "in VS_OUT                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color; // not used ??                                                         \n"
    "} vs_in[];                                                                             \n"
    "                                                                                       \n"
    "uniform mat4 u_modelview;                                                              \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 pos[4];                                                                       \n"
    "    float tf[4];                                                                       \n"
    "    float t = 0.0;                                                                     \n"
    "                                                                                       \n"
    "    int i;                                                                             \n"
    "                                                                                       \n"
    "    if (gl_InvocationID == 0)                                                          \n"
    "    {                                                                                  \n"
    "        for (i = 0; i < 4; i++)                                                        \n"
    "        {                                                                              \n"
    "            pos[i] = u_viewscreen * u_modelview * gl_in[i].gl_Position;                \n"
    "        }                                                                              \n"
    "                                                                                       \n"
    "        tf[0] = max(2.0, distance(pos[0].xy / pos[0].w,                                \n"
    "                                  pos[1].xy / pos[1].w) *      6.0);                   \n"
    "        tf[1] = max(2.0, distance(pos[1].xy / pos[1].w,                                \n"
    "                                  pos[3].xy / pos[3].w) *      6.0);                   \n"
    "        tf[2] = max(2.0, distance(pos[2].xy / pos[2].w,                                \n"
    "                                  pos[3].xy / pos[3].w) *      6.0);                   \n"
    "        tf[3] = max(2.0, distance(pos[2].xy / pos[2].w,                                \n"
    "                                  pos[0].xy / pos[0].w) *      6.0);                   \n"
    "        for (i = 0; i < 4; i++)                                                        \n"
    "        {                                                                              \n"
    "            t = max(t, tf[i]);                                                         \n"
    "        }                                                                              \n"
    "                                                                                       \n"
    "        gl_TessLevelInner[0] = t;                                                      \n"
    "        gl_TessLevelInner[1] = t;                                                      \n"
    "        gl_TessLevelOuter[0] = tf[0];                                                  \n"
    "        gl_TessLevelOuter[1] = tf[1];                                                  \n"
    "        gl_TessLevelOuter[2] = tf[2];                                                  \n"
    "        gl_TessLevelOuter[3] = tf[3];                                                  \n"
    "    }                                                                                  \n"
    "                                                                                       \n"
    "    gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;         \n"
    "}                                                                                      \n"
};

const char *tes_source = {
    "#version 420 core                                                                      \n"
    "layout (quads, fractional_odd_spacing, ccw) in;                                        \n"
    "uniform mat4 u_modelview;                                                              \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "out vec3 normal;                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 mid1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);       \n"
    "    vec4 mid2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);       \n"
    "    vec4 pos = mix(mid1, mid2, gl_TessCoord.y);                                        \n"
    "    pos.xyz = /* normalize*/(pos.xyz) * 0.25;                                          \n"
    "    normal = normalize(mat3(u_modelview) * pos.xyz);                                   \n"
    "    gl_Position = u_viewscreen * u_modelview * pos;                                    \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 420 core                                                                      \n"
    "out vec4 color;                                                                        \n"
    "in vec3 normal;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(abs(normal), 1.0);                                                    \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;
	SpuShader m_shader;
	Mat4f u_modelview;
	Mat4f u_viewscreen;
	SpuArray m_array;
	bool m_wireframe = 1;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shader
	{
		Attrs shader_attrs = {
		        {"frag", fs_source },
		        {"vert", vs_source },
		        {"tesc", tcs_source},
		        {"tese", tes_source},
		};
		Attrs unif_attrs = {
		        {"u_modelview",  &u_modelview },
		        {"u_viewscreen", &u_viewscreen},
		};
		loadShader(m_shader, shader_attrs, unif_attrs);
	}
	// array
	{
		const uint16_t indices[]
		        = {0, 1, 2, 3, 2, 3, 4, 5, 4, 5, 6, 7, 6, 7, 0, 1, 0, 2, 6, 4, 1, 7, 3, 5};
		const float positions[] = {
		        -0.25, -0.25, -0.25, -0.25, +0.25, -0.25, +0.25, -0.25, -0.25, +0.25, +0.25, -0.25,
		        +0.25, -0.25, +0.25, +0.25, +0.25, +0.25, -0.25, -0.25, +0.25, -0.25, +0.25, +0.25,
		};

		Attrs array_attrs = {
		        {"a.0", 3}
                };
		m_array.init(array_attrs);
		m_array.send(positions, 8);
		m_array.send(indices, 24, -1, 2);
		m_array.set("patch_vertices", 4);
	}
	// renderstate
	{
		auto &renderstate = getRenderstate();
		renderstate.flags.cull_face = true;
		renderstate.flags.depth_test = true;
		renderstate.depth_func = GL_LEQUAL;
	}
	// bg
	{
		Attrs attrs = {
		        {"bgcolor0", c_green},
		        {"bgdepth",  1.0    },
		};
		BaseApp::set(attrs);
	}
}

void App::menu() { ImGui::Checkbox("wireframe", &m_wireframe); }

void App::render()
{
	auto t = getSeconds().current();
	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	u_viewscreen = composition.viewscreen();
	auto &renderstate = getRenderstate();
	renderstate.flags.fill = m_wireframe ? 0 : 1;
	renderstate.use();
	for (auto i = 0; i < 100; i++) {
		auto f = i + t * 0.03f;
		u_modelview = c_unit.scale(8.0)
		                      .rot("YX", -t * 5.0, -t * 3.0)
		                      .trans({sinf(2.1 * f) * 4.0f, cosf(1.7 * f) * 4.0f,
		                              sinf(4.3 * f) * cosf(3.5 * f) * 30.0f})
		                      .trans({0.0, 0.0, -10.0});
		m_shader.use();
		m_array.draw(GL_PATCHES, 0, 24);
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("tessellatedcube");
}  // namespace spu::tessellatedcube
