//
// App :
//
#include "base_app.h"
namespace spu::tessellatedtri {
/* clang-format off */
const char *vs_source = {
    "#version 410 core                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec4 vertices[] = vec4[](vec4( 0.25, -0.25,      0.5, 1.0),                  \n"
    "                                   vec4(-0.25, -0.25,      0.5, 1.0),                  \n"
    "                                   vec4( 0.25,  0.25,      0.5, 1.0));                 \n"
    "                                                                                       \n"
    "    gl_Position = vertices[gl_VertexID];                                               \n"
    "}                                                                                      \n"
};

const char *tcs_source = {
    "#version 410 core                                                                      \n"
    "layout (vertices = 3) out;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if (gl_InvocationID == 0)                                                          \n"
    "    {                                                                                  \n"
    "        gl_TessLevelInner[0] = 5.0;                                                    \n"
    "        gl_TessLevelOuter[0] = 5.0;                                                    \n"
    "        gl_TessLevelOuter[1] = 5.0;                                                    \n"
    "        gl_TessLevelOuter[2] = 5.0;                                                    \n"
    "    }                                                                                  \n"
    "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;          \n"
    "}                                                                                      \n"
};

const char *tes_source = {
    "#version 410 core                                                                      \n"
    "layout (triangles, equal_spacing, cw) in;                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +                            \n"
    "                  (gl_TessCoord.y * gl_in[1].gl_Position) +                            \n"
    "                  (gl_TessCoord.z * gl_in[2].gl_Position);                             \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 410 core                                                                      \n"
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
	        {"frag", fs_source },
	        {"vert", vs_source },
	        {"tesc", tcs_source},
	        {"tese", tes_source},
	};
	loadShader(m_shader, shader_attrs, Attrs());

	Attrs init_attrs = {
	        {"nelem", 64}
        };
	m_array.init(init_attrs);
	m_array.set("patch_vertices", 3);
	auto &renderstate = getRenderstate();
	renderstate.flags.fill = false;
	// renderstate.use();
}

void App::render()
{
	m_shader.use();
	m_array.draw(GL_PATCHES, 0, 3);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("tessellatedtri");
}  // namespace spu::tessellatedtri
