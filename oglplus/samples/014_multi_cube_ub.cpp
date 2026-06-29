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
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "layout (std140) uniform u_model_block {                                                \n"
    "       mat4 model_matrices[36];                                                        \n"
    "};                                                                                     \n"
    "in vec4 a_position;                                                                    \n"
    "out vec3 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       mat4 u_nodeworld = model_matrices[gl_InstanceID];                               \n"
    "       gl_Position = u_viewsceen * u_worldview * u_nodeworld * a_position;             \n"
    "       f_color = abs(normalize((u_nodeworld * a_position).yxz));                       \n"
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
	Mat4f u_model_block[36];

	App(const char *name) : SpuPage(name, true, {0.9, 0.9, 0.9, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			Attrs shader_attrs = {
			        {"frag", c_frag},
			        {"vert", c_vert},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen",   &u_viewsceen  },
			        {"u_worldview",   &u_worldview  },
			        {"u_model_block", &u_model_block},
			};
			m_array.initShader(shader_attrs, unif_attrs);
			m_array.initArray(shapes::Cube(), {"position"});
			m_array.setMode(GL_TRIANGLE_STRIP);
			m_array.setInstanceCount(36);
		}

		// make the matrices
		{
			// 36 x 4x4 matrices

			auto angle = 0.0f;
			auto astep = radians(10.0f);

			std::vector<Mat4f> matrices;

			for (auto i = 0; i < 36; i++) {
				auto cx = cosf(angle);
				auto sx = sinf(angle);

				auto matrix = Mat4f(cx, 0, -sx, 0, 0, 1, 0, 0, sx, 0, cx, 0, 0, 0, 0, 1)
				            * Mat4f(1, 0, 0, 12, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

				angle += astep;

				matrices.emplace_back(matrix);
			}

			memcpy(u_model_block[0].data(), matrices.data(), matrices.size() * sizeof(matrices[0]));
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 70, 1, 50);
		u_worldview = Mat4f::orbiting(ezero(), esec, 18.5, 0, 0, 0, 2.66, 0, 30, 20);
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("014_multi_cube_ub");
}  // namespace
}  // namespace spu::oglplus
