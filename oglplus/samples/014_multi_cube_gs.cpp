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
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 108) out;                                        \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "out vec3 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       for (int c=0; c!=36; ++c)                                                       \n"
    "       {                                                                               \n"
    "               float angle = c * 10 * 2 * 3.14159 / 360.0;                             \n"
    "               float cx = cos(angle);                                                  \n"
    "               float sx = sin(angle);                                                  \n"
    "               mat4 u_nodeworld = mat4(                                                \n"
    "                        cx, 0.0,  sx, 0.0,                                             \n"
    "                       0.0, 1.0, 0.0, 0.0,                                             \n"
    "                       -sx, 0.0,  cx, 0.0,                                             \n"
    "                       0.0, 0.0, 0.0, 1.0                                              \n"
    "               ) * mat4(                                                               \n"
    "                        1.0, 0.0, 0.0, 0.0,                                            \n"
    "                        0.0, 1.0, 0.0, 0.0,                                            \n"
    "                        0.0, 0.0, 1.0, 0.0,                                            \n"
    "                       12.0, 0.0, 0.0, 1.0                                             \n"
    "               );                                                                      \n"
    "               for (int v=0; v!=gl_in.length(); ++v)                                   \n"
    "               {                                                                       \n"
    "                       vec4 vert = gl_in[v].gl_Position;                               \n"
    "                       gl_Position =                                                   \n"
    "                               u_viewsceen * u_worldview * u_nodeworld * vert;         \n"
    "                       f_color = abs(normalize(u_nodeworld*vert)).xzy;                 \n"
    "                       EmitVertex();                                                   \n"
    "               }                                                                       \n"
    "               EndPrimitive();                                                         \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_color;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(f_color, 1.0);                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;
	Mat4f u_viewsceen;
	Mat4f u_worldview;

	App(const char *name) : SpuPage(name, true, {0.9, 0.9, 0.9, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"geom", c_geom},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen", &u_viewsceen},
		        {"u_worldview", &u_worldview},
		};

		m_array.initShader(shader_attrs, unif_attrs);
		m_array.initArray(shapes::Cube(), {"position"});
		m_array.setMode(GL_TRIANGLE_STRIP);
		m_array.setInstanceCount(36);
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 70, 1, 50);
		u_worldview = Mat4f::orbiting(ezero(), esec, 18.5, 0, 0, 0, 2.66, 0, 30, 20);
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("014_multi_cube_gs");
}  // namespace
}  // namespace spu::oglplus
