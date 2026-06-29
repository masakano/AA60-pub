//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/cube.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "out vec2 f_texcoord;                                                                   \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_normal = mat3(u_nodeworld)* a_normal;                                         \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_light = u_light_pos - gl_Position.xyz;                                        \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_texture;                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = sqrt(length(f_light));                                                \n"
    "       float d = l > 0? dot(f_normal, normalize(f_light))/ l : 0.0;                    \n"
    "       float i = 0.6 + max(d, 0.0);                                                    \n"
    "       final_color = texture(u_texture, f_texcoord)*i;                                 \n"
        /*"     final_color.r *= 1.5;"*/
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	static constexpr auto c_tex_side = 512;

	uint32_t m_currentTex = 0;

	shapes::Array m_array;
	SpuFrame m_frames[2];

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;
	uint32_t u_texture;

	App(const char *name) : SpuPage(name, true, {1.0, 1.0, 1.0, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen},
                                {"u_worldview", &u_worldview},
			        {"u_nodeworld", &u_nodeworld},
                                {"u_light_pos", &u_light_pos},
			        {"u_texture",   &u_texture  },
			};
			m_array.initShader(shader_attrs, unif_attrs);
			u_light_pos = {4, 4, -8};
		}

		{
			m_array.initArray(shapes::Cube(), {"position", "normal", "texcoord"});

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.cull_face = GL_BACK;
		}

		for (auto &m_frame: m_frames) {
			auto viewport = Rectf(0, 0, c_tex_side, c_tex_side);
			auto bgcolor = Vec4f(1.0, 1.0, 1.0, 0.0);
			Attrs attrs = {
			        {"viewport0",         viewport             },
			        {"color0.target",     GL_TEXTURE_2D        },
			        {"color0.iformat",    GL_RGBA8             },
			        {"color0.min_filter", GL_LINEAR            },
			        {"color0.min_filter", GL_LINEAR            },
			        {"color0.wrap_s",     GL_REPEAT            },
			        {"color0.wrap_t",     GL_REPEAT            },
			        {"depth.target",      GL_RENDERBUFFER      },
			        {"depth.iformat",     GL_DEPTH_COMPONENT32F},
			        {"depth.wrap_s",      GL_REPEAT            },
			        {"depth.wrap_t",      GL_REPEAT            },
			        {"bgcolor0",          bgcolor              },
			};
			m_frame = SpuFrame(attrs);
		}
	}

	void render() override
	{
		auto front = m_currentTex;
		auto back = (m_currentTex + 1) % 2;
		m_currentTex = back;

		auto esec = getSeconds().current();
		u_texture = m_frames[front].getBuffer("color0").id();
		u_worldview = Mat4f::orbiting(ezero(), esec, 3, 0, 0, 0, 10.29, 0, 60, 20);
		u_nodeworld = math::unit().rot("x", -esec * 0.25 * math::two_pi());

		auto texc_viewport = Rectf(0, 0, 1, 1);
		u_viewsceen = math::perspective(texc_viewport, 40, 1, 40);

		m_frames[back].begin();
		m_frames[back].clear();
		m_array.draw(nullptr);
		m_frames[back].end();

		u_viewsceen = math::perspective(viewport(0), 75, 1, 40);
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("025_recursive_texture");
}  // namespace
}  // namespace spu::oglplus
