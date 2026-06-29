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
    "#version 120                                                                           \n"
    "attribute vec3 a_position;                                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(a_position, 1.0);                                            \n"
    "};                                                                                     \n"
};

const char *c_frag =  {
    "#version 120                                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);                                        \n"
    "};                                                                                     \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuArray m_array;
	SpuShader m_shader;

	App(const char *name) : SpuPage(name, true, {0.1, 0.1, 0.1, 1.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};
		m_shader.init(shader_attrs);

		std::vector<vec3f_t> vertices = {
		        {0.0, 0.0, 0.0},
		        {1.0, 0.0, 0.0},
		        -ey(),
		};

		Attrs array_attrs = {
		        {"shader_id",    m_shader.id()                },
		        {"a.a_position", 3                            },
		        {"data",         vertices.data()},
		        {"nelem",        vertices.size()              },
		};
		m_array.init(array_attrs);
	}

	void render() override
	{
		m_shader.use();
		m_array.draw(GL_TRIANGLES, 0, 3);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("001_triangle_glsl120");
}  // namespace
}  // namespace spu::oglplus
