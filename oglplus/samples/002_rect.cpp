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
    "in vec3 a_color;                                                                       \n"
    "out vec3 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_color = a_color;                                                              \n"
    "       gl_Position = vec4(a_position, 0.0, 1.0);                                       \n"
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

		/*
		struct _vec2f_t {
		        float x, y;
		};
		printf("size = %ld:%ld\n", sizeof(_vec2f_t), sizeof(vec2f_t));
		*/
		std::vector<vec2f_t> vertices = {
		        {-1.0, -1.0},
		        {-1.0, +1.0},
		        {+1.0, -1.0},
		        {+1.0, +1.0},
		};

		Attrs array_attrs0 = {
		        {"shader_id",    m_shader.id()                },
		        {"a.a_position", 2                            },
		        {"data",         vertices.data()},
		        {"nelem",        vertices.size()              },
		};

		std::vector<vec3f_t> colors = {
		        eone(),
		        {1.0, 0.0, 0.0},
		        ey(),
		        ez(),
		};
		Attrs array_attrs1 = {
		        {"a.a_color", 3                          },
		        {"data",      colors.data()},
		        {"nelem",     colors.size()              },
		};

		m_array.aux(array_attrs0, 0);
		m_array.aux(array_attrs1, 1);
	}

	void render() override
	{
		m_shader.use();
		m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("002_rect");
}  // namespace
}  // namespace spu::oglplus
