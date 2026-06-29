//
// App :
//
#include "base_app.h"
namespace spu::tessmodes {
/* clang-format off */
const char *vs_source = {
    "#version 420 core                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec4 vertices[] = vec4[](vec4( 0.4, -0.4, 0.5,           1.0),               \n"
    "                                   vec4(-0.4, -0.4, 0.5,           1.0),               \n"
    "                                   vec4( 0.4,  0.4, 0.5,           1.0),               \n"
    "                                   vec4(-0.4,  0.4, 0.5,           1.0));              \n"
    "                                                                                       \n"
    "    gl_Position = vertices[gl_VertexID];                                               \n"
    "}                                                                                      \n"
};

const char *tcs_source_triangles = {
    "#version 420 core                                                                      \n"
    "layout (vertices = 3) out;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if (gl_InvocationID == 0)                                                          \n"
    "    {                                                                                  \n"
    "        gl_TessLevelInner[0] = 5.0;                                                    \n"
    "        gl_TessLevelOuter[0] = 8.0;                                                    \n"
    "        gl_TessLevelOuter[1] = 8.0;                                                    \n"
    "        gl_TessLevelOuter[2] = 8.0;                                                    \n"
    "    }                                                                                  \n"
    "    gl_out[gl_InvocationID].gl_Position =          gl_in[gl_InvocationID].gl_Position; \n"
    "}                                                                                      \n"
};

const char *tes_source_triangles = {
    "#version 420 core                                                                      \n"
    "layout (triangles) in;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position)          +                   \n"
    "                  (gl_TessCoord.y * gl_in[1].gl_Position)          +                   \n"
    "                  (gl_TessCoord.z *        gl_in[2].gl_Position);                      \n"
    "}                                                                                      \n"
};

const char *tes_source_triangles_as_points = {
    "#version 420 core                                                                      \n"
    "                                                                                       \n"
    "layout (triangles, point_mode) in;                                                     \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position)          +                   \n"
    "                  (gl_TessCoord.y * gl_in[1].gl_Position)          +                   \n"
    "                  (gl_TessCoord.z *        gl_in[2].gl_Position);                      \n"
    "}                                                                                      \n"
};

const char *tcs_source_quads = {
    "#version 420 core                                                                      \n"
    "layout (vertices = 4) out;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if (gl_InvocationID == 0)                                                          \n"
    "    {                                                                                  \n"
    "        gl_TessLevelInner[0] = 9.0;                                                    \n"
    "        gl_TessLevelInner[1] = 7.0;                                                    \n"
    "        gl_TessLevelOuter[0] = 3.0;                                                    \n"
    "        gl_TessLevelOuter[1] = 5.0;                                                    \n"
    "        gl_TessLevelOuter[2] = 3.0;                                                    \n"
    "        gl_TessLevelOuter[3] = 5.0;                                                    \n"
    "    }                                                                                  \n"
    "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;          \n"
    "}                                                                                      \n"
};

const char *tes_source_quads = {
    "#version 420 core                                                                      \n"
    "                                                                                       \n"
    "layout (quads) in;                                                                     \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);         \n"
    "    vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);         \n"
    "    gl_Position = mix(p1, p2, gl_TessCoord.y);                                         \n"
    "}                                                                                      \n"
};

const char *tcs_source_isolines = {
    "#version 420 core                                                                      \n"
    "                                                                                       \n"
    "layout (vertices = 4) out;                                                             \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    if (gl_InvocationID == 0)                                                          \n"
    "    {                                                                                  \n"
    "        gl_TessLevelOuter[0] = 5.0;                                                    \n"
    "        gl_TessLevelOuter[1] = 5.0;                                                    \n"
    "    }                                                                                  \n"
    "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;          \n"
    "}                                                                                      \n"
};

const char *tes_source_isolines = {
    "#version 420 core                                                                      \n"
    "layout (isolines) in;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    float r = (gl_TessCoord.y + gl_TessCoord.x / gl_TessLevelOuter[0]);                \n"
    "    float t = gl_TessCoord.x * 2.0 * 3.14159;                                          \n"
    "    gl_Position = vec4(sin(t) * r, cos(t) * r, 0.5, 1.0);                              \n"
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
	SpuShader m_shaders[4];
	int32_t m_index;
	SpuArray m_array;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	const char *tcs_sources[]
	        = {tcs_source_quads, tcs_source_triangles, tcs_source_triangles, tcs_source_isolines};
	const char *tes_sources[]
	        = {tes_source_quads, tes_source_triangles, tes_source_triangles_as_points, tes_source_isolines};
	for (auto i = 0; i < 4; i++) {
		Attrs shader_attrs = {
		        {"frag", fs_source     },
		        {"vert", vs_source     },
		        {"tesc", tcs_sources[i]},
		        {"tese", tes_sources[i]},
		};
		loadShader(m_shaders[i], shader_attrs, Attrs());
	}
	Attrs array_attrs = {
	        {"nelem", 64}
        };

	m_array.init(array_attrs);
	m_array.set("patch_vertices", 4);
	auto &renderstate = getRenderstate();
	renderstate.flags.fill = false;
}

void App::menu() { ImGui::Combo("mode", &m_index, "mode0\0mode1\0mode2\0mode3\0\0"); }

void App::render()
{
	m_shaders[m_index].use();
	m_array.draw(GL_PATCHES, 0, 4);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("tessmodes");
}  // namespace spu::tessmodes
