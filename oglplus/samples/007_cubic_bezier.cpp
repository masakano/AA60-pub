//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "out vec4 final_color;                                                                  \n"
    "uniform vec3 u_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(u_color, 1.0);                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_controlArray;
	SpuArray m_curveArray;
	Vec3f u_color;

	App(const char *name) : SpuPage(name, true, {0.5, 0.5, 0.5, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_color", &u_color},
		};

		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		Vec2f bezier_cps[] = {
		        {-0.9, -0.9},
                        {-0.9, +0.9},
                        {+0.9, +0.9},
                        {+0.9, +0.0},
                        {+0.9, -0.9},
		        {+0.0, -0.9},
                        {+0.0, +0.5},
                        {+0.5, +0.5},
                        {+0.7, +0.5},
                        {+0.7, +0.0},
		        {-0.7, -0.2},
                        {-0.8, +0.8},
                        {+0.9, +0.9},
		};

		BezierCurves<Vec2f, double, 3> bezier;

		bezier.init(std::vector<Vec2f>(
		        bezier_cps, bezier_cps + sizeof(bezier_cps) / sizeof(bezier_cps[0])));

		{
			auto vecs = bezier.approximate(25);
			std::vector<vec2f_t> pvecs(vecs.begin(), vecs.end());

			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"data",         pvecs.data() },
			        {"nelem",        pvecs.size() },
			        {"a.a_position", 2            },
			};
			m_curveArray.init(attrs);
		}

		{
			auto vecs = bezier.controlPoints();
			std::vector<vec2f_t> pvecs(vecs.begin(), vecs.end());

			Attrs attrs = {
			        {"shader_id",    m_shader.id()},
			        {"data",         pvecs.data() },
			        {"nelem",        pvecs.size() },
			        {"a.a_position", 2            },
			};
			m_controlArray.init(attrs);
		}
	}

	void render() override
	{
		// lines
		{
			u_color = {0.9, 0.9, 0.2};
			m_shader.use();
			m_controlArray.draw(GL_LINE_STRIP);

			u_color = {0.9, 0.9, 0.9};
			m_shader.use();
			m_curveArray.draw(GL_LINE_STRIP);
		}

		// points
		{
			SpuScopedRenderstate renderstate(true);
			renderstate.point_size = 16.0;
			renderstate.use();

			u_color = {0.9, 0.0, 0.0};
			m_shader.use();
			m_controlArray.draw(GL_POINTS);
		}
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("007_cubic_bezier");
}  // namespace
}  // namespace spu::oglplus
