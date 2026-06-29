//
// App :
//
#include "base_app.h"
namespace spu {
namespace {
/* clang-format off */
const char *c_vert = {
    "#version 420 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag_normalized = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord);                                            \n"
    "}                                                                                      \n"
};

const char *c_frag_uint = {
    "#version 420 core                                                                      \n"
    "uniform usampler2D u_diffuse;                                                          \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
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
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_arrayId;
	std::vector<uint32_t> m_textureIds;
	std::vector<uint32_t> m_progIds;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture
		{
			struct {
				const char *path;
				uint32_t prog_id;
				uint32_t iformat;
				uint32_t pformat;
				uint32_t ptype;

			} patterns[] = {
			        {"kueken7_bgra8_srgb.dds",      0, GL_RGBA8,                      0,               0},
			        {"kueken7_bgra8_srgb.dds",      0, GL_COMPRESSED_RG_RGTC2,        0,               0},
			        {"kueken7_bgra8_srgb.dds",      0, GL_COMPRESSED_RGBA_BPTC_UNORM, 0,               0},
			        {"kueken7_bgra8_srgb.dds",      0, GL_COMPRESSED_RED_RGTC1,       0,               0},
			        {"kueken7_rgb_dxt1_unorm.dds",  0, 0,                             0,               0},
			        {"kueken7_rgba_dxt5_unorm.dds", 0, 0,                             0,               0},
			        {"kueken7_r_ati1n_unorm.dds",   0, 0,                             0,               0},
			        {"kueken7_rg_ati2n_unorm.dds",  0, 0,                             0,               0},
			        {"kueken7_bgra8_srgb.dds",      1, GL_RGBA8UI,                    GL_BGRA_INTEGER, 0},
			};

			for (auto &pat: patterns) {
				Attrs attrs;
				if (pat.iformat != 0u) {
					attrs.emplace_back("iformat", pat.iformat);
				}
				if (pat.pformat != 0u) {
					attrs.emplace_back("pformat", pat.pformat);
				}
				if (pat.ptype != 0u) {
					attrs.emplace_back("ptype", pat.ptype);
				}

				auto texture_id = loadDDS(pat.path, attrs);
				m_textureIds.push_back(texture_id);
				m_progIds.push_back(pat.prog_id);
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

			for (auto i = 0; i < 2; i++) {
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_worldscreen", &u_worldscreen},
				        {"u_diffuse",     &u_diffuse    },
				};
				loadShader(m_shaders[i], shader_attrs, unif_attrs);
			}
		}
		// array
		{
			Attrs attrs = {
			        {"shader_id",    m_shaders[0].id()},
			        {"a.a_position", 2                },
			        {"a.a_texcoord", 2                },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs);
		}
	}

	void render() override
	{
		std::vector<Rectf> m_viewports = makeViewports(4, 3);

		for (auto &texture_id: m_textureIds) {
			auto i = &texture_id - &m_textureIds[0];
			auto prog_id = m_progIds[i];

			spu_array_set(m_arrayId, "shader_id", m_shaders[prog_id].id());
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

			u_worldscreen = getCamera().worldscreen();
			u_diffuse = texture_id;
			m_shaders[prog_id].use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_texture_conversion");
}  // namespace
}  // namespace spu
