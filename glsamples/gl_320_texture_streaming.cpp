//
// App :
//
#include "base_app.h"
#include "gl_320_texture_2d.h"
namespace spu {
namespace {

class App : public BaseApp {
public:
	SpuShader m_shader;
	uint32_t m_arrayId;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// program
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
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>();

			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
		// texture
		{
			const std::string filename = "kueken7_bgra8_srgb.dds";
			dds::Image image(filename.c_str(), true);

			auto width = image.width();
			auto height = image.height();

			Attrs attrs = {
			        {"iformat",    GL_SRGB8_ALPHA8},
			        {"pformat",    GL_BGRA        },
			        {"width",      width          },
			        {"height",     height         },
			        {"min_filter", GL_NEAREST     },
			        {"mag_filter", GL_NEAREST     },
			};
			u_diffuse = loadDDS("kueken7_bgra8_srgb.dds", attrs);
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

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_texture_streaming");
}  // namespace
}  // namespace spu
