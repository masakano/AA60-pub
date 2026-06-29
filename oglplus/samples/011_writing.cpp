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

const char *c_geom =  {
    "#version 330                                                                           \n"
    "layout(lines) in;                                                                      \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec4 offs = vec4(0.02, 0.01, 0.0, 0.0);                                         \n"
    "       gl_Position = gl_in[0].gl_Position - offs;                                      \n"
    "       EmitVertex();                                                                   \n"
    "       gl_Position = gl_in[0].gl_Position + offs;                                      \n"
    "       EmitVertex();                                                                   \n"
    "       gl_Position = gl_in[1].gl_Position - offs;                                      \n"
    "       EmitVertex();                                                                   \n"
    "       gl_Position = gl_in[1].gl_Position + offs;                                      \n"
    "       EmitVertex();                                                                   \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(0.0, 0.0, 0.0, 1.0);                                         \n"
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
		        {"geom", c_geom},
		};
		m_shader.init(shader_attrs);

		const Vec2f points[] = {
		        {-0.33, +0.50},
                        {-0.45, +0.70},
                        {-0.66, +0.70},
                        {-0.66, +0.30},
                        {-0.66, -0.20},
		        {-0.35, -0.15},
                        {-0.30, +0.05},
                        {-0.20, +0.50},
                        {-0.30, +0.50},
                        {-0.33, +0.50},
		        {-0.50, +0.45},
                        {-0.10, +0.40},
                        {+0.10, +0.55},
                        {-0.20, +0.40},
                        {-0.30, -0.10},
		        {+0.00, -0.10},
                        {+0.10, -0.10},
                        {+0.20, -0.10},
                        {+0.10, +0.55},
                        {+0.20, +0.00},
		        {+0.30, -0.70},
                        {+0.00, -0.75},
                        {-0.40, -0.75},
                        {+0.00, +0.00},
                        {+0.40, +0.10},
		        {+0.60, +0.10},
                        {+0.70, +0.90},
                        {+0.55, +0.90},
                        {+0.35, +0.90},
                        {+0.10, -0.10},
		        {+0.55, +0.00},
                        {+0.90, +0.10},
                        {+0.70, +0.10},
                        {+0.90, +0.20},
		};
		BezierCurves<Vec2f, double, 3> bezier;

		bezier.init(std::vector<Vec2f>(points, points + sizeof(points) / sizeof(points[0])));

		auto vecs = bezier.approximate(25);
		std::vector<vec2f_t> pvec2(vecs.begin(), vecs.end());

		Attrs vert_attrs = {
		        {"shader_id",    m_shader.id()             },
		        {"data",         pvec2.data()},
		        {"nelem",        pvec2.size()              },
		        {"a.a_position", 2                         },
		};
		m_array.init(vert_attrs);

		auto bgcolor0 = Vec4f(0.9, 0.9, 0.9, 0.0);

		Attrs frame_attrs = {
		        {"bgcolor0", bgcolor0},
		        {"bgdepth",  -1.0    },
		};
		SpuPage::set(frame_attrs);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = false;
		// renderstate.use();
	}

	void render() override
	{
		m_shader.use();
		m_array.draw(GL_LINE_STRIP);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("011_writing");
}  // namespace
}  // namespace spu::oglplus
