//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/cube.hpp>
#include <shapes/torus.hpp>

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
    "       f_light = u_light_pos-gl_Position.xyz;                                          \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_frag_cube =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_texture;                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = sqrt(length(f_light));                                                \n"
    "       float d = l > 0? dot(f_normal, normalize(f_light)) / l : 0.0;                   \n"
    "       float i = 0.6 + max(d, 0.0);                                                    \n"
    "       final_color = texture(u_texture, f_texcoord)*i;                                 \n"
    "}                                                                                      \n"
};

const char *c_frag_torus =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float d = dot(                                                                  \n"
    "               f_normal,                                                               \n"
    "               normalize(f_light)                                                      \n"
    "       );                                                                              \n"
    "       float i = (                                                                     \n"
    "               int(f_texcoord.x*18) % 2+                                               \n"
    "               int(f_texcoord.y*14) % 2                                                \n"
    "       ) % 2;                                                                          \n"
    "       float c = (0.4 + max(d, 0.0))*(1-i/2);                                          \n"
    "       final_color = vec4(c, c, c, 1.0);                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	static constexpr auto c_tex_side = 512;

	shapes::Array m_cubeArray;
	shapes::Array m_torusArray;
	SpuFrame m_frame;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;

	Vec3f u_light_pos;
	uint32_t u_texture;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		// cube array
		{
			Attrs shader_attrs = {
			        {"vert", c_vert     },
			        {"frag", c_frag_cube},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen},
                                {"u_worldview", &u_worldview},
			        {"u_nodeworld", &u_nodeworld},
                                {"u_light_pos", &u_light_pos},
			        {"u_texture",   &u_texture  },
			};
			m_cubeArray.initShader(shader_attrs, unif_attrs);
			m_cubeArray.initArray(shapes::Cube(), {"position", "normal", "texcoord"});
		}

		// torus array
		{
			Attrs shader_attrs = {
			        {"vert", c_vert      },
			        {"frag", c_frag_torus},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen},
			        {"u_worldview", &u_worldview},
			        {"u_nodeworld", &u_nodeworld},
			        {"u_light_pos", &u_light_pos},
			};
			auto torus_shape = shapes::Torus(1.0, 0.5, 72, 48);

			m_torusArray.initShader(shader_attrs, unif_attrs);
			m_torusArray.initArray(torus_shape, {"position", "normal", "texcoord"});
		}

		// frame
		{
			auto viewport = Rectf(0, 0, c_tex_side, c_tex_side);
			auto bgcolor0 = Vec4f(0.8, 0.8, 0.8, 0.0);

			Attrs init_attrs = {
			        {"viewport0",         viewport             },
			        {"color0.target",     GL_TEXTURE_2D        },
			        {"color0.iformat",    GL_RGBA8             },
			        {"color0.min_filter", GL_LINEAR            },
			        {"color0.mag_filter", GL_LINEAR            },
			        {"color0.wrap_s",     GL_REPEAT            },
			        {"color0.wrap_t",     GL_REPEAT            },
			        {"depth.target",      GL_RENDERBUFFER      },
			        {"depth.iformat",     GL_DEPTH_COMPONENT32F},
			};
			m_frame.init(init_attrs);
			m_frame.set("bgcolor0", bgcolor0);
		}

		// renderstate
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.cull_face = GL_BACK;

			auto bgcolor0 = Vec4f(0.4, 0.9, 0.4, 1.0);
			this->set(Attrs({Attr("bgcolor0", bgcolor0)}));
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();

		// render to texture
		{
			m_frame.begin();
			m_frame.clear();

			u_light_pos = {2.0, 3.0, 4.0};

			auto texc_viewport = Rectf(0, 0, 1, 1);
			u_viewsceen = math::perspective(texc_viewport, 60, 1, 30);
			u_worldview = Mat4f::orbiting(ezero(), esec, 3.5, 0, 0, 0, 14.4, 0, 90, 30);
			u_nodeworld = Mat4f(Quatf(esec / 2 * math::two_pi(), Vec3f(1, 1, 1)));

			m_torusArray.draw(nullptr);
			m_frame.end();
		}

		// render to frame buffer
		{
			u_light_pos = {4, 4, -8};
			u_viewsceen = math::perspective(viewport(0), 70, 1, 30);
			u_worldview = Mat4f::orbiting(ezero(), esec, 3, 0, 0, 0, 10.25, 0, 60, 20);
			u_nodeworld = math::unit().rot("x", -esec / 4 * math::two_pi());
			u_texture = m_frame.getBuffer("color0").id();

			m_cubeArray.draw(nullptr);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("025_rendered_texture");
}  // namespace
}  // namespace spu::oglplus
