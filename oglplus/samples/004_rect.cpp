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
    "out vec2 f_position;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_position = a_position;                                                        \n"
    "       gl_Position = vec4(a_position, 0.0, 1.0);                                       \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "const float radius = 0.4;                                                              \n"
    "in vec2 f_position;                                                                    \n"
    "uniform vec2 u_red_center;                                                             \n"
    "uniform vec2 u_geen_center;                                                            \n"
    "uniform vec2 u_blue_center;                                                            \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 dist = vec3(                                                               \n"
    "               distance(f_position, u_red_center),                                     \n"
    "               distance(f_position, u_geen_center),                                    \n"
    "               distance(f_position, u_blue_center)                                     \n"
    "       );                                                                              \n"
    "       final_color = vec4(                                                             \n"
    "               dist.r < radius ? 1.0 : (2*radius - dist.r) / radius,                   \n"
    "               dist.g < radius ? 1.0 : (2*radius - dist.g) / radius,                   \n"
    "               dist.b < radius ? 1.0 : (2*radius - dist.b) / radius,                   \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;
	Vec2f u_red_center;
	Vec2f u_geen_center;
	Vec2f u_blue_center;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_red_center",  &u_red_center },
		        {"u_geen_center", &u_geen_center},
		        {"u_blue_center", &u_blue_center},
		};
		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		u_red_center = {-0.141, 0.141};
		u_geen_center = {0.141, 0.141};
		u_blue_center = {0.0, -0.2};

		std::vector<vec2f_t> vertices = {
		        {-1.0, -1.0},
                        {-1.0, +1.0},
                        {+1.0, -1.0},
                        {+1.0, +1.0}
                };

		Attrs array_attrs = {
		        {"shader_id",    m_shader.id()  },
		        {"a.a_position", 2              },
		        {"data",         vertices.data()},
		        {"nelem",        vertices.size()},
		};
		m_array.init(array_attrs);

		Attrs bg_attrs = {
		        {"bgdepth", -1.0}
                };
		SpuPage::set(bg_attrs);
		SpuPage::getRenderstate().flags.depth_test = false;
	}

	void render() override
	{
		m_shader.use();
		m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("004_rect");
}  // namespace
}  // namespace spu::oglplus
