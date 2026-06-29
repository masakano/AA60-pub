//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                          \n"
    "}                                                                                      \n"
};

const char *c_frag_normalized = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord);                                            \n"
    "}                                                                                      \n"
};

const char *c_frag_uint = {
    "#version 330                                                                           \n"
    "uniform usampler2D u_diffuse;                                                          \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec2 size = textureSize(u_diffuse, 0) - 1;                                         \n"
    "    ivec2 coord = ivec2(f_texcoord * size);                                            \n"
    "    uvec4 texel = texelFetch(u_diffuse, coord + ivec2(0, 0), 0);                       \n"
    "    color = vec4(texel) / 255.f;                                                       \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shaders[2];
	uint32_t m_arrayIds[2];
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	std::vector<Rectf> m_viewports;
	std::vector<uint32_t> m_textureIds;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 2);
		// texture
		{
			uint32_t internal_format[]
			        = {GL_RGBA8, GL_RGBA8UI, GL_RGBA16F, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
			           GL_RGBA8_SNORM};

			uint32_t pixel_format[] = {GL_BGRA, GL_BGRA_INTEGER, GL_BGRA, GL_BGRA, GL_BGRA};

			m_textureIds.resize(5);
			for (auto i = 0; i < 5; i++) {
				Attrs attrs = {
				        {"iformat",    internal_format[i]},
				        {"pformat",    pixel_format[i]   },
				        {"ptype",      GL_UNSIGNED_BYTE  },
				        {"base_level", 0                 },
				        {"max_level",  0                 },
				        {"min_filter", GL_NEAREST        },
				        {"mag_filter", GL_NEAREST        },
				};
				m_textureIds[i] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
			}
		}
		// program
		{
			const char *verts[] = {
			        c_vert,
			        c_vert,
			};

			const char *frags[] = {
			        c_frag_normalized,
			        c_frag_uint,
			};

			for (auto &shader: m_shaders) {
				int32_t i = &shader - &m_shaders[0];
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen},
				        {"u_diffuse",     &u_diffuse    },
				};
				loadShader(shader, shader_attrs, unif_attrs);
			}
		}
		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>();
			for (auto i = 0; i < 2; i++) {
				Attrs attrs = {
				        {"shader_id",    m_shaders[i].id()},
				        {"a.a_position", 2                },
				        {"a.a_texcoord", 2                },
				        {"data",         c_vertices.data()},
				        {"nelem",        c_vertices.size()},
				};
				m_arrayIds[i] = spu_array_new(attrs);
			}
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen();

		int32_t tex_index[] = {0, 1, 2, 4};   // work
		int32_t unif_index[] = {0, 1, 0, 0};  // 0:normalized, 1:uint

		for (auto i = 0; i < 4; i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			u_diffuse = m_textureIds[tex_index[i]];
			m_shaders[unif_index[i]].use();
			spu_array_draw(m_arrayIds[unif_index[i]], GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_texture_format");
}  // namespace
}  // namespace spu
