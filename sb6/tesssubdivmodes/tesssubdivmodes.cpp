//
// App :
//
#include "base_app.h"
namespace spu::tesssubdivmodes {
/* clang-format off */
const char *vs_source = {
    "#version 420 core                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec4 vertices[] = vec4[](vec4( 0.8, -0.8, 0.5,       1.0),                   \n"
    "                                   vec4(-0.8, -0.8, 0.5,       1.0),                   \n"
    "                                   vec4( 0.8,  0.8, 0.5,       1.0),                   \n"
    "                                   vec4(-0.8,  0.8, 0.5,       1.0));                  \n"
    "                                                                                       \n"
    "    gl_Position = vertices[gl_VertexID];                                               \n"
    "}                                                                                      \n"
};

const char *tcs_source_triangles = {
    "#version 420 core                                                                      \n"
    "                                                                                       \n"
    "layout (vertices = 3) out;                                                             \n"
    "uniform float tess_level = 2.7;                                                        \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if (gl_InvocationID == 0)                                                          \n"
    "    {                                                                                  \n"
    "        gl_TessLevelInner[0] = tess_level;                                             \n"
    "        gl_TessLevelOuter[0] = tess_level;                                             \n"
    "        gl_TessLevelOuter[1] = tess_level;                                             \n"
    "        gl_TessLevelOuter[2] = tess_level;                                             \n"
    "    }                                                                                  \n"
    "    gl_out[gl_InvocationID].gl_Position =      gl_in[gl_InvocationID].gl_Position;     \n"
    "}                                                                                      \n"
};

const char *tes_source_equal = {
    "#version 420 core                                                                      \n"
    "layout (triangles) in;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position)      +                       \n"
    "                  (gl_TessCoord.y * gl_in[1].gl_Position)      +                       \n"
    "                  (gl_TessCoord.z * gl_in[2].gl_Position);                             \n"
    "}                                                                                      \n"
};

const char *tes_source_fract_even = {
    "#version 420 core                                                                      \n"
    "                                                                                       \n"
    "layout (triangles, fractional_even_spacing) in;                                        \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position)      +                       \n"
    "                  (gl_TessCoord.y * gl_in[1].gl_Position)      +                       \n"
    "                  (gl_TessCoord.z * gl_in[2].gl_Position);                             \n"
    "}                                                                                      \n"
};

const char *tes_source_fract_odd = {
    "#version 420 core                                                                      \n"
    "layout (triangles, fractional_odd_spacing) in;                                         \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position)      +                       \n"
    "                  (gl_TessCoord.y * gl_in[1].gl_Position)      +                       \n"
    "                  (gl_TessCoord.z * gl_in[2].gl_Position);                             \n"
    "}                                                                                      \n"
};

const char *fs_source = {
    "#version 420 core                                                                      \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0);                                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void menu() override;
	void render() override;
	SpuShader m_shaders[3];
	int32_t m_index = 0;
	SpuArray m_array;
	float u_tess_level = 2.7;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	const char *tcs_sources[] = {tcs_source_triangles, tcs_source_triangles, tcs_source_triangles};

	const char *tes_sources[] = {tes_source_equal, tes_source_fract_even, tes_source_fract_odd};

	for (auto i = 0; i < 3; i++) {
		Attrs shader_attrs = {
		        {"frag", fs_source     },
		        {"vert", vs_source     },
		        {"tesc", tcs_sources[i]},
		        {"tese", tes_sources[i]},
		};
		Attrs unif_attrs = {
		        {"tess_level", &u_tess_level},
		};
		loadShader(m_shaders[i], shader_attrs, unif_attrs);
	}
	Attrs array_attrs = {
	        {"nelem", 64}
        };
	m_array.init(array_attrs);
	m_array.set("patch_vertices", 4);

	auto &renderstate = getRenderstate();
	renderstate.flags.fill = false;
}

void App::menu() { ImGui::Combo("mode", &m_index, "mode0\0mode1\0mode2\0\0"); }

void App::render()
{
	u_tess_level = 5.3;
	m_shaders[m_index].use();
	m_array.draw(GL_PATCHES, 0, 4);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("tesssubdivmodes");
}  // namespace spu::tesssubdivmodes
