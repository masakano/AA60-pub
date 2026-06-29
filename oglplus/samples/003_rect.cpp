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
    "       gl_Position = vec4(a_position, 0.0, 1.0);                                       \n"
    "       f_position = vec2(                                                              \n"
    "               (a_position.x + 1.0)/2.0,                                               \n"
    "               (a_position.y + 1.0)/2.0                                                \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec2 f_position;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(                                                             \n"
    "               f_position.x - f_position.x * f_position.y,                             \n"
    "               f_position.y - f_position.x * f_position.y,                             \n"
    "               f_position.x * f_position.y,                                            \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};
		m_shader.init(shader_attrs);

		std::vector<vec2f_t> vertices = {
		        {-1.0, -1.0},
		        {-1.0, +1.0},
		        {+1.0, -1.0},
		        {+1.0, +1.0},
		};

		Attrs array_attrs = {
		        {"shader_id",    m_shader.id()                },
		        {"a.a_position", 2                            },
		        {"data",         vertices.data()},
		        {"nelem",        vertices.size()              },
		};
		m_array.init(array_attrs);
	}

	void render() override
	{
		m_shader.use();
		m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("003_rect");
}  // namespace
}  // namespace spu::oglplus
