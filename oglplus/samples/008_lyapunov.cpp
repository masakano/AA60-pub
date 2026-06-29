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
    "uniform float u_scroll_factor;                                                         \n"
    "in vec2 a_position;                                                                    \n"
    "in vec2 a_coord;                                                                       \n"
    "out vec2 f_position;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_position = u_scroll_factor * a_coord;                                         \n"
    "       gl_Position = vec4(a_position, 0.0, 1.0);                                       \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec2 f_position;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "const int npasses = 100;                                                               \n"
    "const int nclr = 5;                                                                    \n"
    "uniform vec4 u_clrs[5];                                                                \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec2 ab = f_position;                                                           \n"
    "       float x_n = 0.5;                                                                \n"
    "       float sum = 0;                                                                  \n"
    "       for (int i = 0; i < npasses; ++i)                                               \n"
    "       {                                                                               \n"
    "               x_n = ab.x * x_n * (1 - x_n);                                           \n"
    "               sum += log(abs(ab.x*(1 - 2 * x_n)));                                    \n"
    "               x_n = ab.x * x_n * (1 - x_n);                                           \n"
    "               sum += log(abs(ab.x*(1 - 2 * x_n)));                                    \n"
    "               x_n = ab.x * x_n * (1 - x_n);                                           \n"
    "               sum += log(abs(ab.x*(1 - 2 * x_n)));                                    \n"
    "               x_n = ab.x * x_n * (1 - x_n);                                           \n"
    "               sum += log(abs(ab.x*(1 - 2 * x_n)));                                    \n"
    "               x_n = ab.x * x_n * (1 - x_n);                                           \n"
    "               sum += log(abs(ab.x*(1 - 2 * x_n)));                                    \n"
    "               x_n = ab.x * x_n * (1 - x_n);                                           \n"
    "               sum += log(abs(ab.x*(1 - 2 * x_n)));                                    \n"
    "               x_n = ab.y * x_n * (1.0 - x_n);                                         \n"
    "               sum += log(abs(ab.y*(1 - 2 * x_n)));                                    \n"
    "               x_n = ab.y * x_n * (1.0 - x_n);                                         \n"
    "               sum += log(abs(ab.y*(1 - 2 * x_n)));                                    \n"
    "               x_n = ab.y * x_n * (1.0 - x_n);                                         \n"
    "               sum += log(abs(ab.y*(1 - 2 * x_n)));                                    \n"
    "               x_n = ab.y * x_n * (1.0 - x_n);                                         \n"
    "               sum += log(abs(ab.y*(1 - 2 * x_n)));                                    \n"
    "               x_n = ab.y * x_n * (1.0 - x_n);                                         \n"
    "               sum += log(abs(ab.y*(1 - 2 * x_n)));                                    \n"
    "               x_n = ab.y * x_n * (1.0 - x_n);                                         \n"
    "               sum += log(abs(ab.y*(1 - 2 * x_n)));                                    \n"
    "       }                                                                               \n"
    "       float lambda = sum / (12 * npasses);                                            \n"
    "       float a = (clamp(lambda, -2.0, 2.0) + 2.0) / 4;                                 \n"
    "       for (int i = 0; i != (nclr - 1); ++i)                                           \n"
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
	static const auto c_nclr = 5;
	SpuShader m_shader;
	SpuArray m_array;

	float u_clrs[c_nclr * 4] = {0.4, 0.2,  1.0, 0.00, 1.0, 0.2,  0.2, 0.30, 1.0, 1.0,
	                            1.0, 0.95, 1.0, 1.0,  1.0, 0.98, 0.1, 0.1,  0.1, 1.00};
	float u_scroll_factor;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_clrs",          &u_clrs         },
		        {"u_scroll_factor", &u_scroll_factor},
		};

		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		std::vector<vec2f_t> vertices = {
		        {-1.0, -1.0},
		        {-1.0, +1.0},
		        {+1.0, -1.0},
		        {+1.0, +1.0},
		};

		float minf = 3.0;
		float maxf = 4.0;
		float rectangle_coords[8] = {minf, minf, minf, maxf, maxf, minf, maxf, maxf};

		Attrs vert_attrs = {
		        {"shader_id",    m_shader.id()  },
		        {"a.a_position", 2              },
		        {"data",         vertices.data()},
		        {"nelem",        vertices.size()},
		};

		Attrs coord_attrs = {
		        {"data",      rectangle_coords},
		        {"nelem",     4               },
		        {"a.a_coord", 2               },
		};

		m_array.aux(vert_attrs, 0);
		m_array.aux(coord_attrs, 1);
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_scroll_factor = 1.0 / (0.005 * esec + 1.0);
		m_shader.use();
		m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("008_lyapunov");
}  // namespace
}  // namespace spu::oglplus
