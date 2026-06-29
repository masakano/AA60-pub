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
const char *c_frag = {
    "#version 420 core                                                                      \n"
    "layout(binding = 0, r32ui) coherent uniform uimage2D u_image_data;                     \n"
    "uniform ivec2 u_image_size;                                                            \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    uint fetch = imageLoad(u_image_data, ivec2(f_texcoord * u_image_size)).x;          \n"
    "    color = unpackUnorm4x8(fetch);                                                     \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	Vec4i u_image_size;

	uint32_t u_image_data;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture
		{
			auto width = 256;
			auto height = 256;

			Attrs attrs = {
			        {"iformat",     GL_R32UI  },
                                {"width",       width     },
                                {"height",      height    },
			        {"max_level",   0         },
                                {"auto_mipmap", 0         },
                                {"mag_filter",  GL_NEAREST},
			        {"min_filter",  GL_NEAREST},
			};
			u_image_data = spu_texture_new(attrs);
			u_image_size = Vec4i(width, height, 0, 0);

			struct RGBA8 {
				uint8_t r, g, b, a;
			};
			std::vector<RGBA8> pixels(width * height);
			for (auto &p: pixels) {
				auto i = &p - &pixels[0];
				auto x = i % width;
				auto y = i / width;
				p = {uint8_t(x), uint8_t(y), 0, 255};
			}
			spu_texture_send(u_image_data, pixels.data(), GL_R32UI);
		}

		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_image_size",  &u_image_size },
			        {"u_image_data",  &u_image_data },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);

			auto format = GL_R32UI;
			auto access = GL_READ_ONLY;

			Attrs set_attrs = {
			        {"u_image_data.format", &format},
			        {"u_image_data.access", &access},
			};
			spu_shader_set(m_shader.id(), set_attrs);
		}
		// array #0
		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs);
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 1000.0);
		u_worldscreen = getCamera().worldscreen();

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_image_unpack");
}  // namespace
}  // namespace spu
