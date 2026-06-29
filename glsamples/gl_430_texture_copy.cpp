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
    "in vec3 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 1.0);                                \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 420 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord.st);                                         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_textureIds[2];  // 0:diffuse 1:copy
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, {1.0, 0.5, 0.0, +1.0}) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture
		{
			Attrs attrs = {
			        {"mag_filter",  GL_LINEAR              },
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"max_level",   8                      },
			        {"auto_mipmap", 1                      },
			};
			m_textureIds[0] = loadDDS("kueken7_bgra8_srgb.dds", attrs);
			m_textureIds[1] = loadDDS("kueken7_bgra8_srgb.dds", attrs);

			fillByColor();
			fillByImage();
		}
		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen}, // UBO
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
		// change mode
		static auto clear_mode = 1;
		spu_printf(0, "hit \"l\" to clear (%d)\n", clear_mode);
		if (getGesture()->pressed('l')) {
			clear_mode = (clear_mode + 1) & 0x1;
			clear_mode == 0 ? fillByColor() : fillByImage();
		}

		{
			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
			u_worldscreen = getCamera().worldscreen();
		}

		u_diffuse = m_textureIds[1];
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}

	// clear with different color in every level
	void fillByColor()
	{
		int32_t max_level;
		spu_texture_get(m_textureIds[1], "max_level", &max_level);
		for (auto level = 0; level <= max_level; level++) {
			auto color = rand();
			int32_t loc[4] = {0, 0, 0, level};
			spu_texture_send(m_textureIds[1], &color, GL_RGBA8, loc, nullptr, true);
		}
	}

	void fillByImage()
	{
		uint32_t width;
		uint32_t height;
		int32_t max_level;

		spu_texture_get(m_textureIds[1], "width", &width);
		spu_texture_get(m_textureIds[1], "height", &height);
		spu_texture_get(m_textureIds[1], "max_level", &max_level);

		for (auto level = 0; level <= max_level; level++) {
			printf("\tcopy %d x %d...\n", width, height);
			int32_t dst_loc[4] = {0, 0, 0, level};
			int32_t src_loc[4] = {0, 0, 0, level};
			uint32_t size[3] = {width, height, 1};
			spu_texture_copy(m_textureIds[1], m_textureIds[0], dst_loc, src_loc, size);
			width /= 2;
			height /= 2;
		}
	}
};
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_texture_copy");
}  // namespace
}  // namespace spu
