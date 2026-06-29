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
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture
		{
			const std::string filename = "kueken7_bgra8_srgb.dds";
			dds::Image image(filename.c_str(), true);

			auto width = image.width();
			auto height = image.height();

			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D  },
                                {"iformat",     GL_SRGB8_ALPHA8},
			        {"pformat",     GL_BGRA        },
                                {"width",       width / 2      },
			        {"height",      height / 2     },
                                {"min_filter",  GL_NEAREST     },
			        {"mag_filter",  GL_NEAREST     },
                                {"auto_mipmap", 0              },
			};
			u_diffuse = spu_texture_new(attrs);

			int32_t dst_loc[4] = {-int32_t(width) / 4, -int32_t(height) / 4, 0, 0};
			uint32_t size[4] = {width, height, 0, 0};

			spu_texture_send(u_diffuse, image.pixels(0), GL_BGRA, dst_loc, size);
		}
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
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_texture_pixel_store");
}  // namespace
}  // namespace spu
