//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/torus.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec4 u_clip_plane;                                                             \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       gl_ClipDistance[0] = dot(u_clip_plane, gl_Position);                            \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float i = (                                                                     \n"
    "               int(f_texcoord.x*36) % 2+                                               \n"
    "               int(f_texcoord.y*24) % 2                                                \n"
    "       ) % 2;                                                                          \n"
    "       if (gl_FrontFacing)                                                             \n"
    "               final_color = vec4(1-i/2, 1-i/2, 1-i/2, 1.0);                           \n"
    "       else final_color = vec4(0+i/2, 0+i/2, 0+i/2, 1.0);                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec4f u_clip_plane;

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			Attrs shader_attrs = {
			        {"frag", c_frag},
			        {"vert", c_vert},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen",  &u_viewsceen },
			        {"u_worldview",  &u_worldview },
			        {"u_nodeworld",  &u_nodeworld },
			        {"u_clip_plane", &u_clip_plane},
			};
			m_array.initShader(shader_attrs, unif_attrs);
			u_clip_plane = {0, 0, 1, 0};
		}

		{
			auto torus_shape = shapes::Torus(1.0, 0.5, 36, 24);
			m_array.initArray(torus_shape, {"position", "texcoord"});

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.clip_distance0 = true;
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 60);
		u_worldview = Mat4f::orbiting(ezero(), esec, 4.5, 0, 0, 0, 10, 45, 30, 7);
		u_nodeworld = math::unit().rot("x", -esec / 12.0 * math::two_pi());

		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("017_clipped_torus");
}  // namespace
}  // namespace spu::oglplus
