//
// App :
//
#include "base_app.h"
#include "gl_320_transform_feedback.h"

namespace spu {
namespace {

class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t m_arrayId;
	uint32_t m_queryId;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert_feed},
			        {"frag", c_frag     },
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// query
		{
			Attrs attrs = {
			        {"target", GL_PRIMITIVES_SUBMITTED},
			};
			m_queryId = spu_query_new(attrs);
		}

		// aray #1
		{
			const std::vector<Vec4f> c_vertices = squareTriangles<Vec4f>();
			Attrs attr = {
			        {"shader_id",    m_shader.id()    },
			        {"a.a_position", 4                },
			        {"data",         c_vertices.data()},
			        {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attr);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		uint64_t primitive_written = 0;

		m_shader.use();
		spu_query_begin(m_queryId);
		spu_array_draw(m_arrayId, GL_TRIANGLES);
		spu_query_end(m_queryId, &primitive_written, true);
		spu_printf(0, "PrimitiveWritten = %d\n", primitive_written);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_transform_feedback_separated");
}  // namespace
}  // namespace spu
