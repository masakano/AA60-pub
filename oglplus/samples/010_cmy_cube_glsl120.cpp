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
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "attribute vec4 a_position;                                                             \n"
    "attribute vec3 a_normal;                                                               \n"
    "varying vec3 f_normal;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_normal = a_normal;                                                            \n"
    "       gl_Position = u_viewsceen *                                                     \n"
    "               u_worldview *                                                           \n"
    "               a_position;                                                             \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 120                                                                           \n"
    "varying vec3 f_normal;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_FragColor = vec4(normalize(abs(vec3(1, 1, 1) - f_normal)), 1.0);             \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;
	Mat4f u_worldview;
	Mat4f u_viewsceen;

	App(const char *name) : SpuPage(name, true, {1.0, 1.0, 1.0, 1.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		        {"use_unif_block", false},
		};

		Attrs unif_attrs = {
		        {"u_worldview", &u_worldview},
		        {"u_viewsceen", &u_viewsceen},
		};
		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		const float c[8][3] = {
		        {-0.5, -0.5, +0.5},
                        {-0.5, -0.5, -0.5},
                        {-0.5, +0.5, -0.5},
                        {-0.5, +0.5, +0.5},
		        {+0.5, -0.5, +0.5},
                        {+0.5, -0.5, -0.5},
                        {+0.5, +0.5, -0.5},
                        {+0.5, +0.5, +0.5}
                };

		const auto vertex_count = 6 * 2 * 3;

		const float cube_vertices[vertex_count * 3]
		        = {c[0][0], c[0][1], c[0][2], c[2][0], c[2][1], c[2][2], c[1][0], c[1][1], c[1][2],
		           c[0][0], c[0][1], c[0][2], c[3][0], c[3][1], c[3][2], c[2][0], c[2][1], c[2][2],

		           c[0][0], c[0][1], c[0][2], c[1][0], c[1][1], c[1][2], c[4][0], c[4][1], c[4][2],
		           c[1][0], c[1][1], c[1][2], c[5][0], c[5][1], c[5][2], c[4][0], c[4][1], c[4][2],

		           c[1][0], c[1][1], c[1][2], c[2][0], c[2][1], c[2][2], c[5][0], c[5][1], c[5][2],
		           c[2][0], c[2][1], c[2][2], c[6][0], c[6][1], c[6][2], c[5][0], c[5][1], c[5][2],

		           c[4][0], c[4][1], c[4][2], c[5][0], c[5][1], c[5][2], c[6][0], c[6][1], c[6][2],
		           c[4][0], c[4][1], c[4][2], c[6][0], c[6][1], c[6][2], c[7][0], c[7][1], c[7][2],

		           c[2][0], c[2][1], c[2][2], c[3][0], c[3][1], c[3][2], c[7][0], c[7][1], c[7][2],
		           c[2][0], c[2][1], c[2][2], c[7][0], c[7][1], c[7][2], c[6][0], c[6][1], c[6][2],

		           c[0][0], c[0][1], c[0][2], c[4][0], c[4][1], c[4][2], c[3][0], c[3][1], c[3][2],
		           c[3][0], c[3][1], c[3][2], c[4][0], c[4][1], c[4][2], c[7][0], c[7][1], c[7][2]};

		Attrs vert_attrs = {
		        {"shader_id",    m_shader.id()},
		        {"data",         cube_vertices},
		        {"nelem",        vertex_count },
		        {"a.a_position", 3            },
		};

		const float n[6][3] = {
		        {-1.0, 0.0,  0.0 },
                        {0.0,  -1.0, 0.0 },
                        {0.0,  0.0,  -1.0},
		        {+1.0, 0.0,  0.0 },
                        {0.0,  +1.0, 0.0 },
                        {0.0,  0.0,  +1.0}
                };

		float cube_normals[vertex_count * 3];
		for (auto f = 0; f != 6; ++f) {
			for (auto v = 0; v != 6; ++v) {
				for (auto ci = 0; ci != 3; ++ci) {
					cube_normals[(f * 6 + v) * 3 + ci] = n[f][ci];
				}
			}
		}

		Attrs normal_attrs = {
		        {"data",       cube_normals},
		        {"nelem",      vertex_count},
		        {"a.a_normal", 3           },
		};

		m_array.aux(vert_attrs, 0);
		m_array.aux(normal_attrs, 1);

		auto eye = Vec3f(2, 2, 2);
		auto dir = Vec3f(-eye);
		auto up = ey();

		u_worldview = math::worldview(eye, dir, up);
	}

	void render() override
	{
		u_viewsceen = math::perspective(viewport(0), 48, 1, 100);
		m_shader.use();
		m_array.draw(GL_TRIANGLES, 0, 6 * 2 * 3);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("010_cmy_cube_glsl120");
}  // namespace
}  // namespace spu::oglplus
