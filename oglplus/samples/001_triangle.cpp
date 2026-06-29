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
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(a_position, 1.0);                                            \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(1.0, 0.0, 0.0, 1.0);                                         \n"
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

		float vertices[9] = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0};

		Attrs array_attrs = {
		        {"shader_id",    m_shader.id()         },
		        {"data",         vertices},
		        {"nelem",        3                     },
		        {"a.a_position", 3                     },
		};
		m_array.init(array_attrs);
	}

	void render() override
	{
		m_shader.use();
		m_array.draw(GL_TRIANGLES, 0, 3);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("001_triangle");
}  // namespace
}  // namespace spu::oglplus
