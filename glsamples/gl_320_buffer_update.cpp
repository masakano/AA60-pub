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
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * a_position;                                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform vec4 u_diffuse;                                                                \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = u_diffuse;                                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Vec4f u_diffuse;
	uint32_t m_arrayId;

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
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// array
		{
			const std::vector<vec2sf_t> c_vertices = squareTriangles();

			Attrs src_attrs = {
			        {"shader_id",    m_shader.id()    },
			        {"a.a_position", 2                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			auto src_array_id = spu_array_new(src_attrs);

			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			};
			m_arrayId = spu_array_new(attrs);  // no data

			// Now copy the vertex data from the other array
			spu_array_copy(m_arrayId, src_array_id, 0, 0);
			spu_array_delete(src_array_id);
		}
	}

	void render() override
	{
		auto &camera = getCamera();
		camera.setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = camera.worldscreen();
		u_diffuse = Vec4f(1.0, 0.5, 0.0, +1.0);
		m_shader.use();

		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_buffer_update");
}  // namespace
}  // namespace spu
