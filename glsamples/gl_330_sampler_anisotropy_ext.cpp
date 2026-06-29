//
// App :
//
#include "base_app.h"
#include "gl_330_texture_2d.h"
namespace spu {
namespace {

class App : public BaseApp {
public:
	uint32_t m_samplerId[4];
	std::vector<Rectf> m_scissors;

	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_sampler;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_sky) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_scissors = makeViewports(2, 2, 1);
		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			        {"u_diffuse",     &u_sampler    },
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
			static const auto c_size = 512;
			static const auto c_level_count = 7;

			// use "clear" instead..
			const std::vector<uint32_t> c_pixs = [=]() {
				std::vector<uint32_t> pixs(c_level_count * c_size * c_size);
				for (auto i = 0; i < c_size * c_size; i++) {
					pixs[0 * c_size * c_size + i] = 0xff0000ff;
					pixs[1 * c_size * c_size + i] = 0xff7f00ff;
					pixs[2 * c_size * c_size + i] = 0xff00ffff;
					pixs[3 * c_size * c_size + i] = 0xff00ff00;
					pixs[4 * c_size * c_size + i] = 0xffffff00;
					pixs[5 * c_size * c_size + i] = 0xffff0000;
					pixs[6 * c_size * c_size + i] = 0xff0000ff;
				}
				return pixs;
			}();

			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D    },
                                {"iformat",     GL_RGBA8         },
			        {"width",       c_size           },
                                {"height",      c_size           },
			        {"min_level",   0                },
                                {"max_level",   c_level_count - 1},
			        {"auto_mipmap", 0                },
			};
			u_diffuse = spu_texture_new(attrs);

			for (auto i = 0; i < c_level_count; i++) {
				int32_t locs[4] = {0, 0, 0, i};
				spu_texture_send(
				        u_diffuse, c_pixs.data() + i * c_size * c_size, GL_RGBA8, locs);
			}
		}
		// sampler
		{
			const std::vector<float> c_max_aniso = {
			        1.0,
			        2.0,
			        4.0,
			        16.0,
			};

			for (auto i = 0; i < 4; i++) {
				Attrs attrs = {
				        {"target",     GL_TEXTURE_SAMPLER       },
				        {"min_filter", GL_NEAREST_MIPMAP_NEAREST},
				        {"mag_filter", GL_NEAREST               },
				        {"max_aniso",  c_max_aniso[i]           },
				};
				m_samplerId[i] = spu_texture_new(attrs);
			}
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();

		for (auto i = 0; i < 4; i++) {
			spu_frame_set(-1, "scissor0", m_scissors[i]);
			u_sampler = m_samplerId[i];
			m_shader.use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
		spu_frame_set(-1, "scissor0", ezero<Vec4f>());
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_330_sampler_anisotropy_ext");
}  // namespace
}  // namespace spu
