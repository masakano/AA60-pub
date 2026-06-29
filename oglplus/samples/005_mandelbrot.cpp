//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_coord;                                                                       \n"
    "out vec2 f_position;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_position = a_coord;                                                           \n"
    "       gl_Position = vec4(a_position, 0.0, 1.0);                                       \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec2 f_position;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "const int nclr = 5;                                                                    \n"
    "uniform vec4 u_clrs[5];                                                                \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec2 z = vec2(0.0, 0.0);                                                        \n"
    "       vec2 c = f_position;                                                            \n"
    "       int i = 0, max = 128;                                                           \n"
    "       while ((i != max) && (distance(z, c) < 2.0))                                    \n"
    "       {                                                                               \n"
    "               vec2 zn = vec2(                                                         \n"
    "                       z.x * z.x - z.y * z.y + c.x,                                    \n"
    "                       2.0 * z.x * z.y + c.y                                           \n"
    "               );                                                                      \n"
    "               z = zn;                                                                 \n"
    "               ++i;                                                                    \n"
    "       }                                                                               \n"
    "       float a = sqrt(float(i) / float(max));                                          \n"
    "       for (i = 0; i != (nclr - 1); ++i)                                               \n"
    "       {                                                                               \n"
    "               if (a >= u_clrs[i].a && a < u_clrs[i+1].a)                              \n"
    "               {                                                                       \n"
    "                       float m = (a - u_clrs[i].a) / (u_clrs[i+1].a - u_clrs[i].a);    \n"
    "                       final_color = vec4(                                             \n"
    "                               mix(u_clrs[i].rgb, u_clrs[i+1].rgb, m),                 \n"
    "                               1.0                                                     \n"
    "                       );                                                              \n"
    "                       break;                                                          \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;
	std::vector<Vec4f> u_clrs = {
	        {0.4, 0.2, 1.0, 0.00},
                {1.0, 0.2, 0.2, 0.30},
                {1.0, 1.0, 1.0, 0.95},
	        {1.0, 1.0, 1.0, 0.98},
                {0.1, 0.1, 0.1, 1.00},
	};

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_clrs", u_clrs.data()},
		};
		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		std::vector<vec2f_t> vertices = {
		        {-1.0, -1.0},
		        {-1.0, +1.0},
		        {+1.0, -1.0},
		        {+1.0, +1.0},
		};

		std::vector<vec2f_t> coords = {
		        {-1.5, -0.5},
		        {-1.5, +1.0},
		        {+0.5, -0.5},
		        {+0.5, +1.0},
		};

		Attrs array_attrs = {
		        {"shader_id",    m_shader.id()  },
		        {"a.a_position", 2              },
		        {"data",         vertices.data()},
		        {"nelem",        vertices.size()},
		};

		Attrs coord_attrs = {
		        {"a.a_coord", 2            },
		        {"data",      coords.data()},
		        {"nelem",     coords.size()},
		};

		m_array.aux(array_attrs, 0);
		m_array.aux(coord_attrs, 1);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = false;
	}

	void render() override
	{
		m_shader.use();
		m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("005_mandelbrot");
}  // namespace
}  // namespace spu::oglplus
