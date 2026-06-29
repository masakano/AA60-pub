//
// App :
//
#include "base_app.h"
namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 400 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 400 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0) out vec4 color0;                                                  \n"
    "layout(location = 1) out vec4 color1;                                                  \n"
    "layout(location = 2) out vec4 color2;                                                  \n"
    "layout(location = 3) out vec4 color3;                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec4 c = texture(u_diffuse, f_texcoord);                                           \n"
    "    color0 = color1 = color2 = color3 = c;                                             \n"
    "}                                                                                      \n"
};

const char *c_vert_blit = {
    "#version 400 core                                                                      \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                           \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};

const char *c_frag_blit = {
    "#version 400 core                                                                      \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = texture(u_diffuse, f_texcoord);                                            \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_bgTextureId;
	uint32_t m_fgTextureId;
	uint32_t m_colorIds[4];
	std::vector<Rectf> m_viewports;
	SpuShader m_shaders[2];
	Mat4f u_worldscreen;
	uint32_t u_diffuse;
	uint32_t m_frameId;
	uint32_t m_arrayIds[2];

	void initTexture2D()
	{
		int32_t width;
		int32_t height;

		// fg
		{
			Attrs attrs = {
			        {"iformat_r", GL_BGR}
                        };
			m_fgTextureId = loadDDS("kueken7_bgra8_srgb.dds", attrs);
			spu_texture_get(m_fgTextureId, "width", &width);
			spu_texture_get(m_fgTextureId, "height", &height);
		}

		// bg
		{
			Attrs attrs = {
			        {"swizzle_r", GL_ALPHA},
			        {"swizzle_g", GL_ALPHA},
			        {"swizzle_b", GL_ALPHA},
			        {"swizzle_a", GL_ALPHA},
			};
			m_bgTextureId = spu_inventory_new("texture", "ball.png", attrs);
		};

		// frames
		{
			for (auto &color_id: m_colorIds) {
				Attrs color_attrs = {
				        {"iformat", GL_RGB8},
				        {"width",   width  },
				        {"height",  height },
				};
				color_id = spu_texture_new(color_attrs);
			}

			auto viewport = Rectf(0, 0, width, height);
			Attrs frame_attrs = {
			        {"color0",    m_colorIds[0]},
                                {"color1",    m_colorIds[1]},
                                {"color2",    m_colorIds[2]},
			        {"color3",    m_colorIds[3]},
                                {"viewport0", viewport     },
			};
			m_frameId = spu_frame_new(frame_attrs);
		}
	}

	App(const char *name) : BaseApp(name) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(2, 2);
		// program #0
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shaders[0], shader_attrs, unif_attrs);
		}

		// program #1
		{
			Attrs shader_attrs = {
			        {"vert", c_vert_blit},
			        {"frag", c_frag_blit},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen},
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shaders[1], shader_attrs, unif_attrs);
		}

		// array
		{
			for (auto i = 0; i < 2; i++) {
				Attrs attrs = {
				        {"shader_id",    m_shaders[i].id()},
				        {"a.a_position", 2                },
				        {"a.a_texcoord", 2                },
				};
				m_arrayIds[i] = squareQuadsArray<v2fv2f_t>(attrs);
			}
		}
		initTexture2D();
	}

	void render() override
	{
		// Pass 1: Compute the u_worldscreen (Model View getCamera().viewscreen() matrix)
		getCamera().setViewscreen(-1.0, 1.0, -1.0, 1.0, -1.0, +1.0, false);
		u_worldscreen = getCamera().viewscreen();

		static auto rtt_mode = 0;

		spu_printf(0, "hit \"l\" to toggle mode (%d)\n", rtt_mode);
		if (getGesture()->pressed('l')) {
			rtt_mode = (rtt_mode + 1) % 3;
		}

		spu_frame_begin(m_frameId);

		// bg
		{
			u_diffuse = m_bgTextureId;
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);
		}

		// fg
		{
			SpuScopedRenderstate renderstate(true);
			switch (rtt_mode) {
			case 0:  // write_mask
			{
				renderstate.flags.blend = true;
				renderstate.blend_color = 0.25;
				renderstate.blend_func = {
				        GL_ONE,
				        GL_CONSTANT_ALPHA,
				        GL_ONE,
				        GL_CONSTANT_ALPHA,
				};
				renderstate.use();

				auto set_write_mask = [&](int32_t chan, uint32_t r, uint32_t g, uint32_t b) {
					renderstate.blend_func_channel = chan;
					renderstate.write_mask = {r, g, b, 1, 1};  // a=1,z=1
					renderstate.use();
				};

				set_write_mask(0, 0, 1, 1);
				set_write_mask(1, 1, 0, 1);
				set_write_mask(2, 1, 1, 0);
				set_write_mask(3, 1, 1, 1);
				break;
			}
			case 1:  // blend_equation
			{
				renderstate.flags.blend = true;
				renderstate.blend_color = 0.5;
				renderstate.blend_func = {
				        GL_ONE,
				        GL_CONSTANT_ALPHA,
				        GL_ONE,
				        GL_CONSTANT_ALPHA,
				};
				renderstate.use();  // necessary

				auto set_blend_eq = [&](int32_t chan, const uint32_t func) {
					renderstate.blend_func_channel = chan;
					renderstate.blend_eq = {func, func};
					renderstate.use();
				};

				set_blend_eq(0, GL_FUNC_ADD);
				set_blend_eq(1, GL_FUNC_SUBTRACT);
				set_blend_eq(2, GL_FUNC_REVERSE_SUBTRACT);
				set_blend_eq(3, GL_MIN);
				break;
			}
			case 2:  // blend func
			{
				renderstate.flags.blend = true;
				renderstate.blend_color = 0.75;
				renderstate.use();  // necessary

				auto set_blend_func = [&](int32_t chan, const uint32_t sfunc, uint32_t dfunc) {
					renderstate.blend_func_channel = chan;
					renderstate.blend_func = {
					        sfunc,
					        dfunc,
					        sfunc,
					        dfunc,
					};
					renderstate.use();
				};
				set_blend_func(0, GL_DST_COLOR, GL_ZERO);
				set_blend_func(1, GL_CONSTANT_ALPHA, GL_ONE_MINUS_SRC_COLOR);
				set_blend_func(2, GL_ONE, GL_ONE_MINUS_DST_COLOR);
				set_blend_func(3, GL_CONSTANT_ALPHA, GL_SRC_COLOR);
				break;
			}
			}
			u_diffuse = m_fgTextureId;
			m_shaders[0].use();
			spu_array_draw(m_arrayIds[0], GL_TRIANGLES);
		}
		spu_frame_end();

		// Pass 2
		{
			getCamera().setViewscreen(-1.0, 1.0, 1.0, -1.0, -1.0, +1.0, false);
			u_worldscreen = getCamera().viewscreen();

			for (auto i = 0; i < 4; i++) {
				spu_frame_set(-1, "viewport0", m_viewports[i]);

				u_diffuse = m_colorIds[i];
				m_shaders[1].use();
				spu_array_draw(m_arrayIds[1], GL_TRIANGLES);  // single
			}
		}
	}
};
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_400_blend_rtt");
}  // namespace
}  // namespace spu
