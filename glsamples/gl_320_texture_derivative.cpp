//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

#define e_use_x

#ifdef e_use_x
/* clang-format off */
const char *c_vert_x = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position,0.0,1.0);                             \n"
    "}                                                                                      \n"
};

const char *c_frag_x = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "float textureLevel(in sampler2D u_sampler, in vec2 texcoord)                           \n"
    "{                                                                                      \n"
    "    vec2 texture_size = vec2(textureSize(u_sampler, 0));                               \n"
    "    float levelCount = max(log2(texture_size.x), log2(texture_size.y));                \n"
    "    vec2 dx = dFdx(texcoord * texture_size);                                           \n"
    "    vec2 dy = dFdy(texcoord * texture_size);                                           \n"
    "    float d = max(dot(dx, dx), dot(dy, dy));                                           \n"
    "    d = clamp(d, 1.0, pow(2, (levelCount - 1) * 2));                                   \n"
    "    return 0.5 * log2(d);                                                              \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    float level = textureLevel(u_diffuse, f_texcoord);                                 \n"
    "    color = textureLod(u_diffuse, f_texcoord, level);                                  \n"
    "}                                                                                      \n"
};

#else	
const char *c_vert_y = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position,0.0,1.0);                            \n"
    "}                                                                                      \n"
};

const char *c_frag_y = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord);                                            \n"
    "}                                                                                      \n"
};
#endif

/* clang-format on */
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

		// program
		{
			Attrs shader_attrs = {
#ifdef e_use_x
			        {"vert", c_vert_x},
			        {"frag", c_frag_x},
#else
			        {"vert", c_vert_y},
			        {"frag", c_frag_y},
#endif
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// array
		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs);
		}

		// texture
		{
			const auto c_size = 64;
			const auto c_level_count = 7;

			// use other color..
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
			        {"target",      GL_TEXTURE_2D           },
			        {"iformat",     GL_RGBA8                },
			        {"width",       c_size                  },
			        {"height",      c_size                  },
			        {"base_level",  0                       },
			        {"max_level",   c_level_count - 1       },
			        {"min_filter",  GL_NEAREST_MIPMAP_LINEAR},
			        {"mag_filter",  GL_NEAREST              },
			        {"wrap_s",      GL_CLAMP_TO_EDGE        },
			        {"wrap_t",      GL_CLAMP_TO_EDGE        },
			        {"auto_mipmap", 0                       },
			};
			u_diffuse = spu_texture_new(attrs);

			for (auto i = 0; i < c_level_count; i++) {
				int32_t locs[4] = {0, 0, 0, i};
				spu_texture_send(
				        u_diffuse, c_pixs.data() + i * c_size * c_size, GL_RGBA8, locs);
			}
		}
		{
			auto &renderstate = getRenderstate();
			renderstate.flags.sample_shading = true;
			renderstate.min_sample_shading = 1.0;
			// renderstate.use();
		}
	}

	void render() override
	{
		getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);

		u_worldscreen = getCamera().worldscreen() * Mat4f().scale(2.0);

		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_texture_derivative");
}  // namespace
}  // namespace spu
