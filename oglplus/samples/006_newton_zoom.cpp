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
    "uniform mat4 u_zoom_matrix;                                                            \n"
    "in vec2 a_position;                                                                    \n"
    "out vec2 f_position;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_position = mat2(u_zoom_matrix) * a_position;                                  \n"
    "       gl_Position = vec4(a_position, 0.0, 1.0);                                       \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec2 f_position;                                                                    \n"
    "uniform vec3 u_color1;                                                                 \n"
    "uniform vec3 u_color2;                                                                 \n"
    "out vec4 final_color;                                                                  \n"
    "                                                                                       \n"
    "vec2 f(vec2 n)                                                                         \n"
    "{                                                                                      \n"
    "       return vec2(                                                                    \n"
    "               n.x*n.x*n.x - 3.0*n.x*n.y*n.y - 1.0,                                    \n"
    "               -n.y*n.y*n.y + 3.0*n.x*n.x*n.y                                          \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
    "                                                                                       \n"
    "vec2 df(vec2 n)                                                                        \n"
    "{                                                                                      \n"
    "       return 3.0 * vec2(                                                              \n"
    "               n.x*n.x - n.y*n.y,                                                      \n"
    "               2.0 * n.x * n.y                                                         \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
    "                                                                                       \n"
    "vec2 cdiv(vec2 a, vec2 b)                                                              \n"
    "{                                                                                      \n"
    "       float d = dot(b, b);                                                            \n"
    "       if (d == 0.0) return a;                                                         \n"
    "       else return vec2(                                                               \n"
    "               (a.x*b.x + a.y*b.y) / d,                                                \n"
    "               (a.y*b.x - a.x*b.y) / d                                                 \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec2 z = f_position;                                                            \n"
    "       int i, max = 128;                                                               \n"
    "       for (i = 0; i != max; ++i)                                                      \n"
    "       {                                                                               \n"
    "               vec2 zn = z - cdiv(f(z), df(z));                                        \n"
    "               if (distance(zn, z) < 0.00001) break;                                   \n"
    "               z = zn;                                                                 \n"
    "       }                                                                               \n"
    "       final_color = vec4(                                                             \n"
    "               mix(                                                                    \n"
    "                       u_color1.rgb,                                                   \n"
    "                       u_color2.rgb,                                                   \n"
    "                       float(i) / float(max)                                           \n"
    "               ),                                                                      \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;

	Vec3f u_color1 = Vec3f(0.2, 0.02, 0.05);
	Vec3f u_color2 = Vec3f(1.0, 0.95, 0.98);
	Mat4f u_zoom_matrix;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_color1",      &u_color1     },
		        {"u_color2",      &u_color2     },
		        {"u_zoom_matrix", &u_zoom_matrix},
		};
		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		std::vector<vec2f_t> vertices = {
		        {-1.0, -1.0},
		        {-1.0, +1.0},
		        {+1.0, -1.0},
		        {+1.0, +1.0},
		};

		Attrs vert_attrs = {
		        {"shader_id",    m_shader.id()  },
		        {"a.a_position", 2              },
		        {"data",         vertices.data()},
		        {"nelem",        vertices.size()},
		};
		m_array.init(vert_attrs);
	}

	void render() override
	{
		auto perpendicular = [](const Vec2f &a) { return Vec2f(-a.f[1], a.f[0]); };
		auto esec = getSeconds().current();
		auto scale = 0.01 + 2.0 - 2.0 * sin(esec / 30 * math::two_pi());
		auto angle = (esec / 10 * math::two_pi());
		auto x = Vec2f(cos(angle), sin(angle));
		auto y = perpendicular(x);

		u_zoom_matrix = Mat4f(Vec4f(x * scale, 0, 0), Vec4f(y * scale, 0, 0), Vec4f(0, 0, 1, 0),
		                      Vec4f(0, 0, 0, 1))
		                        .transpose4();

		m_shader.use();
		m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("006_newton_zoom");
}  // namespace
}  // namespace spu::oglplus
