//
// App :
//
#include "base_app.h"
namespace spu {
namespace {
/* clang-format off */
const uint32_t c_sets1[][64] = {
	{
		4, 0, 1, 2, 1, 7, 0, 7, 7, 1, 0, 5, 6, 7, 7, 3,
		1, 5, 2, 5, 7, 2, 6, 1, 1, 3, 3, 3, 0, 2, 4, 0,
		7, 5, 0, 6, 1, 2, 0, 3, 7, 2, 4, 1, 7, 3, 4, 3,
		4, 3, 5, 1, 5, 2, 3, 6, 7, 2, 2, 2, 5, 5, 6, 6,
	},
	{
		0, 2, 3, 4, 1, 6, 5, 7, 5, 4, 0, 5, 3, 7, 2, 1,
		6, 7, 1, 2, 5, 4, 3, 0, 1, 5, 4, 3, 7, 0, 6, 2,
		3, 6, 7, 0, 2, 1, 4, 5, 2, 0, 5, 1, 4, 3, 7, 6,
		4, 1, 2, 7, 6, 5, 0, 3, 7, 3, 6, 5, 0, 2, 1, 4,
	},
	{
		2, 3, 4, 5, 7, 2, 7, 5, 6, 7, 1, 3, 6, 6, 2, 0,
		6, 7, 5, 7, 6, 6, 2, 4, 6, 3, 1, 7, 6, 6, 7, 1,
		7, 0, 4, 3, 2, 2, 5, 1, 0, 1, 4, 5, 3, 5, 4, 1,
		0, 0, 5, 5, 0, 4, 1, 1, 2, 2, 7, 5, 4, 1, 1, 4,
	},
	{
		0, 7, 6, 7, 3, 6, 7, 3, 2, 6, 3, 6, 1, 3, 6, 1,
		3, 5, 4, 5, 0, 4, 5, 0, 1, 4, 0, 4, 2, 0, 4, 2,
		2, 6, 3, 6, 1, 3, 6, 1, 3, 5, 4, 5, 0, 4, 5, 0,
		2, 6, 3, 6, 1, 3, 6, 1, 6, 1, 5, 1, 4, 5, 1, 4,
	},
};

const uint32_t c_sets2[][64] = {
	{
		4, 0, 1, 0, 7, 1, 4, 1, 3, 7, 4, 7, 3, 6, 3, 0,
		1, 6, 5, 5, 6, 1, 7, 2, 0, 5, 5, 4, 0, 2, 2, 1,
		7, 5, 7, 7, 0, 1, 4, 7, 0, 7, 6, 7, 2, 3, 4, 6,
		3, 1, 3, 1, 0, 5, 3, 1, 3, 1, 1, 0, 5, 5, 5, 3,
	},
	{
		7, 3, 6, 0, 5, 4, 1, 2, 4, 5, 2, 1, 7, 3, 6, 0,
		3, 6, 1, 4, 2, 0, 7, 5, 2, 0, 7, 5, 6, 1, 4, 3,
		0, 7, 3, 6, 4, 5, 2, 1, 5, 1, 4, 2, 3, 6, 0, 7,
		6, 2, 0, 3, 1, 7, 5, 4, 1, 4, 5, 7, 0, 2, 3, 6,
	},
	{
		2, 3, 4, 5, 7, 2, 7, 5, 6, 7, 1, 3, 6, 6, 2, 0,
		6, 7, 5, 7, 6, 6, 2, 4, 6, 3, 1, 7, 6, 6, 7, 1,
		7, 0, 4, 3, 2, 2, 5, 1, 0, 1, 4, 5, 3, 5, 4, 1,
		0, 0, 5, 5, 0, 4, 1, 1, 2, 2, 7, 5, 4, 1, 1, 4,
	},
	{
		0, 7, 6, 7, 3, 6, 7, 3, 2, 6, 3, 6, 1, 3, 6, 1,
		3, 5, 4, 5, 0, 4, 5, 0, 1, 4, 0, 4, 2, 0, 4, 2,
		2, 6, 3, 6, 1, 3, 6, 1, 3, 5, 4, 5, 0, 4, 5, 0,
		2, 6, 3, 6, 1, 3, 6, 1, 6, 1, 5, 1, 4, 5, 1, 4,
	},
};

const uint32_t c_sets3[][64] = {
	{
		0, 4, 3, 4, 7, 2, 6, 5, 3, 3, 5, 1, 2, 0, 3, 1,
		0, 2, 3, 2, 5, 3, 7, 4, 3, 2, 5, 4, 3, 7, 3, 6,
		1, 7, 6, 1, 0, 6, 2, 3, 2, 7, 1, 4, 5, 0, 4, 5,
		6, 6, 3, 3, 4, 5, 5, 4, 1, 6, 4, 4, 3, 4, 3, 3,
	},
	{
		6, 0, 7, 3, 2, 4, 5, 1, 4, 1, 2, 5, 0, 7, 3, 6,
		7, 3, 5, 6, 4, 1, 0, 2, 2, 7, 4, 0, 1, 3, 6, 5,
		0, 5, 1, 7, 6, 2, 4, 3, 1, 2, 3, 4, 5, 6, 7, 0,
		5, 4, 6, 2, 3, 0, 1, 7, 3, 6, 0, 1, 7, 5, 2, 4,
	},
	{
		2, 3, 4, 5, 7, 2, 7, 5, 6, 7, 1, 3, 6, 6, 2, 0,
		6, 7, 5, 7, 6, 6, 2, 4, 6, 3, 1, 7, 6, 6, 7, 1,
		7, 0, 4, 3, 2, 2, 5, 1, 0, 1, 4, 5, 3, 5, 4, 1,
		0, 0, 5, 5, 0, 4, 1, 1, 2, 2, 7, 5, 4, 1, 1, 4,
	},
	{
		0, 7, 6, 7, 3, 6, 7, 3, 2, 6, 3, 6, 1, 3, 6, 1,
		3, 5, 4, 5, 0, 4, 5, 0, 1, 4, 0, 4, 2, 0, 4, 2,
		2, 6, 3, 6, 1, 3, 6, 1, 3, 5, 4, 5, 0, 4, 5, 0,
		2, 6, 3, 6, 1, 3, 6, 1, 6, 1, 5, 1, 4, 5, 1, 4,
	},
};

const uint32_t c_sets4[][64] = {
	{
		2, 4, 7, 5, 0, 1, 6, 0, 0, 5, 7, 0, 6, 1, 0, 3,
		3, 7, 3, 0, 6, 6, 4, 1, 7, 5, 0, 4, 6, 0, 7, 2,
		4, 3, 6, 6, 5, 3, 6, 0, 3, 2, 2, 1, 6, 0, 1, 2,
		2, 4, 6, 4, 1, 7, 6, 3, 2, 5, 7, 6, 2, 5, 6, 1,
	},
	{
		6, 7, 2, 3, 1, 5, 0, 4, 4, 3, 0, 5, 7, 6, 1, 2,
		1, 5, 7, 6, 2, 4, 0, 3, 0, 2, 1, 4, 3, 7, 6, 5,
		3, 4, 6, 2, 5, 0, 7, 1, 7, 0, 5, 1, 4, 3, 2, 6,
		2, 6, 4, 7, 0, 1, 5, 3, 5, 1, 3, 0, 6, 2, 4, 7,
	},
	{
		2, 3, 4, 5, 7, 2, 7, 5, 6, 7, 1, 3, 6, 6, 2, 0,
		6, 7, 5, 7, 6, 6, 2, 4, 6, 3, 1, 7, 6, 6, 7, 1,
		7, 0, 4, 3, 2, 2, 5, 1, 0, 1, 4, 5, 3, 5, 4, 1,
		0, 0, 5, 5, 0, 4, 1, 1, 2, 2, 7, 5, 4, 1, 1, 4,
	},
	{
		0, 7, 6, 7, 3, 6, 7, 3, 2, 6, 3, 6, 1, 3, 6, 1,
		3, 5, 4, 5, 0, 4, 5, 0, 1, 4, 0, 4, 2, 0, 4, 2,
		2, 6, 3, 6, 1, 3, 6, 1, 3, 5, 4, 5, 0, 4, 5, 0,
		2, 6, 3, 6, 1, 3, 6, 1, 6, 1, 5, 1, 4, 5, 1, 4,
	},
};
/* clang-format on */

/* clang-format off */
const char *c_vert = {
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

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2DArray u_diffuse;                                                      \n"
    "uniform int u_layer;                                                                   \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, vec3(f_texcoord, u_layer));                             \n"
    "}                                                                                      \n"
};
/* clang-format on */

class App : public BaseApp {
public:
	SpuShader m_shader;
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t u_layer;
	uint32_t m_arrayId;

	std::array<vec3sb_t, 8 * 8> buildColorChart(vec3ub_t const c_colors[], const uint32_t components[])
	{
		std::array<vec3sb_t, 8 * 8> data;

		for (auto j = 0u; j < 8; ++j) {
			for (auto i = 0u; i < 8; ++i) {
				const auto texel_index = i + j * 8u;
				const auto color_index = components[texel_index];
				assert(color_index <= 7);

				data[texel_index] = {
				        char(c_colors[color_index].x),
				        char(c_colors[color_index].y),
				        char(c_colors[color_index].z),
				};
			}
		}

		return data;
	}

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture
		{
			const uint32_t c_size(8);

			const vec3ub_t c_colors[] = {
			        {214, 37,  152}, // 0, Pink C
			        {239, 51,  64 }, // 1, Red 032 C
			        {254, 80,  0  }, // 2, Orange 021 C
			        {255, 215, 0  }, // 3, Yellow 012 C
			        {0,   132, 61 }, // 4, 348 C (Green)
			        {0,   133, 202}, // 5, Process Blue C
			        {16,  6,   159}, // 6, Blue 072 C
			        {78,  0,   142}  // 7, Medium Purple C
			};

			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D_ARRAY},
			        {"iformat",     GL_RGB8            },
			        {"width",       c_size             },
			        {"height",      c_size             },
			        {"depth",       16                 },
			        {"base_level",  0                  },
			        {"max_level",   0                  },
			        {"min_filter",  GL_NEAREST         },
			        {"mag_filter",  GL_NEAREST         },
			        {"wrap_s",      GL_CLAMP_TO_EDGE   },
			        {"wrap_t",      GL_CLAMP_TO_EDGE   },
			        {"auto_mipmap", 0                  },
			};
			u_diffuse = spu_texture_new(attrs);

			const uint32_t *c_sets[] = {
			        c_sets1[0], c_sets1[1], c_sets1[2], c_sets1[3], c_sets2[0], c_sets2[1],
			        c_sets2[2], c_sets2[3], c_sets3[0], c_sets3[1], c_sets3[2], c_sets3[3],
			        c_sets4[0], c_sets4[1], c_sets4[2], c_sets4[3],
			};

			for (auto i = 0; i < 16; i++) {
				const auto data = buildColorChart(c_colors, c_sets[i]);
				int32_t dst_loc[4] = {0, 0, i, 0};
				uint32_t size[4] = {c_size, c_size, 1, 0};
				spu_texture_send(u_diffuse, &data[0], GL_RGB8, dst_loc, size);
			}
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
			        {"u_layer",       &u_layer      },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}

		// array
		{
			const auto s = Vec2f(0.8);
			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 2            },
			        {"a.a_texcoord", 2            },
			};
			m_arrayId = squareQuadsArray<v2fv2f_t>(attrs, s);
		}
		getCamera().setViewscreen(-1.0, 1.0, -1.0, 1.0, -1.0, +1.0, false);
		u_worldscreen = getCamera().viewscreen();
	}

	void render() override
	{
		// auto viewport_save = viewport(0);
		auto w = float(viewport(0).sx / 4);
		auto h = float(viewport(0).sy / 4);

		for (auto y = 0.0f; y < 4.0f; ++y) {
			for (auto x = 0.0f; x < 4.0f; ++x) {
				auto viewport = Rectf(x * w, y * h, w, h);
				spu_frame_set(-1, "viewport0", viewport);
				// getViewports().at(0) = Rectf(x * w, y * h, w, h);
				// sync(0);  // don't forget

				u_layer = x + y * 4;
				m_shader.use();
				spu_array_draw(m_arrayId, GL_TRIANGLES);
			}
		}
		// spu_frame_set(-1, "viewport0", viewport_save);
		// getViewports().at(0) = viewport_save;
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_texture_float");
}  // namespace
}  // namespace spu
