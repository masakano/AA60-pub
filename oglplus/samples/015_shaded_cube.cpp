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
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_color;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_normal = (                                                                    \n"
    "               u_worldview * vec4(                                                     \n"
    "                       normalize(                                                      \n"
    "                               a_normal +                                              \n"
    "                               a_position.xyz*0.5                                      \n"
    "                       ), 0.0                                                          \n"
    "               )                                                                       \n"
    "       ).xyz;                                                                          \n"
    "       f_color = abs(a_normal);                                                        \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               a_position;                                                             \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_color;                                                                       \n"
    "in vec3 f_normal;                                                                      \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float intensity = pow(                                                          \n"
    "               dot(                                                                    \n"
    "                       f_normal,                                                       \n"
    "                       vec3(0.0, 0.0, 1.0)                                             \n"
    "               ), 3.0                                                                  \n"
    "       );                                                                              \n"
    "       final_color = vec4(f_color,1.0)*intensity;                                      \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;
	Mat4f u_viewsceen;
	Mat4f u_worldview;

	App(const char *name) : SpuPage(name, true, {0.08, 0.08, 0.03, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen", &u_viewsceen},
		        {"u_worldview", &u_worldview},
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

		const uint32_t vertex_count = 6 * 2 * 3;
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

		const float n[6][3] = {
		        {-1.0, 0.0,  0.0 },
                        {0.0,  -1.0, 0.0 },
                        {0.0,  0.0,  -1.0},
		        {1.0,  0.0,  0.0 },
                        {0.0,  1.0,  0.0 },
                        {0.0,  0.0,  1.0 }
                };

		float cube_normals[vertex_count * 3];

		for (auto f = 0; f != 6; ++f) {
			for (auto v = 0; v != 6; ++v) {
				for (auto ci = 0; ci != 3; ++ci) {
					cube_normals[(f * 6 + v) * 3 + ci] = n[f][ci];
				}
			}
		}

		Attrs vert_attrs = {
		        {"shader_id",    m_shader.id()},
		        {"data",         cube_vertices},
		        {"nelem",        vertex_count },
		        {"a.a_position", 3            },
		};

		Attrs norm_attrs = {
		        {"data",       cube_normals},
		        {"nelem",      vertex_count},
		        {"a.a_normal", 3           },
		};

		m_array.init(vert_attrs);
		m_array.aux(norm_attrs, 1);
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 200);
		u_worldview = Mat4f::orbiting(ezero(), esec, 3, 0, 0, 0, 2.66, 0, 90, 20);

		m_shader.use();
		m_array.draw(GL_TRIANGLES);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("015_shaded_cube");
}  // namespace
}  // namespace spu::oglplus
