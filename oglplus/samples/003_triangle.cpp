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
    "in vec3 a_position;                                                                    \n"
    "in vec3 color;                                                                         \n"
    "uniform mat4 u_nodescreen;                                                             \n"
    "out vec4 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodescreen * vec4(a_position, 1.0);                             \n"
    "       f_color = vec4(color, 1.0);                                                     \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec4 f_color;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = f_color;                                                          \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;
	Mat4f u_nodescreen;

	App(const char *name) : SpuPage(name, true, {1.0, 1.0, 1.0, 1.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_nodescreen", &u_nodescreen},
		};
		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		std::vector<vec3f_t> vertices = {
		        {0.0, 0.0, 0.0},
                        {1.0, 0.0, 0.0},
                        ey()
                };

		std::vector<vec3f_t> colors = {
		        {1.0, 0.0, 0.0},
                        ey(), ez()
                };

		Attrs array_attrs0 = {
		        {"shader_id",    m_shader.id()  },
		        {"a.a_position", 3              },
		        {"data",         vertices.data()},
		        {"nelem",        vertices.size()}
                };

		Attrs array_attrs1 = {
		        {"a.Color", 3            },
		        {"data",    colors.data()},
		        {"nelem",   colors.size()},
		};

		m_array.aux(array_attrs0, 0);
		m_array.aux(array_attrs1, 1);

		u_nodescreen = {
		        2.0, 0.0, 0.0, -1.0, 0.0, 2.0, 0.0, -1.0, 0.0, 0.0, 2.0, +0.0, 0.0, 0.0, 0.0, +1.0,
		};
	}

	void render() override
	{
		m_shader.use();
		m_array.draw(GL_TRIANGLES, 0, 3);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("003_triangle");
}  // namespace
}  // namespace spu::oglplus
