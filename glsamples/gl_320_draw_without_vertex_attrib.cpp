//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "const int vertexCount = 6;                                                             \n"
    "const vec2 positions[vertexCount] = vec2[](                                            \n"
    "    vec2(-1.0,-1.0),                                                                   \n"
    "    vec2( 1.0,-1.0),                                                                   \n"
    "    vec2( 1.0, 1.0),                                                                   \n"
    "    vec2(-1.0,-1.0),                                                                   \n"
    "    vec2( 1.0, 1.0),                                                                   \n"
    "    vec2(-1.0, 1.0));                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * vec4(positions[gl_VertexID], 0.0, 1.0);               \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0, 0.5, 0.0, 1.0);                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	uint32_t m_arrayId;
	Mat4f u_worldscreen;

	App(const char *name) : BaseApp(name, true, c_black) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// shader
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			Attrs attrs = {
			        {"nelem", 6},
			};
			m_arrayId = spu_array_new(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();
		m_shader.use();

		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_draw_without_vertex_attrib");
}  // namespace
}  // namespace spu
