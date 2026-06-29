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
    "#version 410                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "out vec4 f_position;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_position = u_worldview * a_position;                                          \n"
    "}                                                                                      \n"
};

const char *c_tesc =  {
    "#version 410                                                                           \n"
    "layout(vertices = 16) out;                                                             \n"
    "in vec4 f_position[];                                                                  \n"
    "patch out vec3 teco_position[16];                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       if (gl_InvocationID == 0)                                                       \n"
    "       {                                                                               \n"
    "               int tl = 1-int(100.0 / f_position[gl_InvocationID].z);                  \n"
    "               gl_TessLevelInner[0] = tl;                                              \n"
    "               gl_TessLevelInner[1] = tl;                                              \n"
    "               gl_TessLevelOuter[0] = tl;                                              \n"
    "               gl_TessLevelOuter[1] = tl;                                              \n"
    "               gl_TessLevelOuter[2] = tl;                                              \n"
    "               gl_TessLevelOuter[3] = tl;                                              \n"
    "       }                                                                               \n"
    "       teco_position[gl_InvocationID] =                                                \n"
    "               f_position[gl_InvocationID].xyz;                                        \n"
    "}                                                                                      \n"
};

const char *c_tese =  {
    "#version 410                                                                           \n"
    "layout(quads, equal_spacing, ccw) in;                                                  \n"
    "uniform mat4 u_viewscreen;                                                              \n"
    "patch in vec3 teco_position[16];                                                       \n"
    "const mat4 b = mat4(                                                                   \n"
    "       -1, 3,-3, 1,                                                                    \n"
    "        3,-6, 3, 0,                                                                    \n"
    "       -3, 3, 0, 0,                                                                    \n"
    "        1, 0, 0, 0                                                                     \n"
    ");                                                                                     \n"
    "mat4 px, py, pz;                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float u = gl_TessCoord.x, v = gl_TessCoord.y;                                   \n"
    "       for (int j=0; j!=4; ++j)                                                        \n"
    "       for (int i=0; i!=4; ++i)                                                        \n"
    "       {                                                                               \n"
    "               int k = j*4+i;                                                          \n"
    "               px[j][i] = teco_position[k].x;                                          \n"
    "               py[j][i] = teco_position[k].y;                                          \n"
    "               pz[j][i] = teco_position[k].z;                                          \n"
    "       }                                                                               \n"
    "       mat4 cx = b * px * b;                                                           \n"
    "       mat4 cy = b * py * b;                                                           \n"
    "       mat4 cz = b * pz * b;                                                           \n"
    "       vec4 up = vec4(u*u*u, u*u, u, 1);                                               \n"
    "       vec4 vp = vec4(v*v*v, v*v, v, 1);                                               \n"
    "       vec4 temp_position = vec4(dot(cx * vp, up),                                     \n"
    "dot(cy * vp, up), dot(cz * vp, up), 1.0);                                              \n"
    "       gl_Position = u_viewscreen * temp_position;                                      \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 410                                                                           \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec3(0.1, 0.1, 0.1);                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;
	Mat4f u_viewscreen;
	Mat4f u_worldview;

	App(const char *name) : SpuPage(name, true, {0.9, 0.9, 0.9, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		        {"tesc", c_tesc},
		        {"tese", c_tese},
			{"use_unif_block", true},
		};

		Attrs unif_attrs = {
		        {"u_viewscreen", &u_viewscreen},
		        {"u_worldview", &u_worldview},
		};
		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		float patch_cp_pos[16 * 3] = {-2.0, 0.0, -2.0, -1.0, 0.0, -3.0, 1.0, 0.0, -5.0, 2.0, 0.0, -2.0,
		                              -1.0, 0.0, -1.0, +0.0, 4.0, -1.0, 1.0, 4.0, -1.0, 3.0, 0.0, -1.0,
		                              -1.0, 0.0, +1.0, -1.0, 4.0, +1.0, 0.0, 4.0, +1.0, 1.0, 0.0, +1.0,
		                              -2.0, 0.0, +2.0, -1.0, 0.0, +5.0, 1.0, 0.0, +3.0, 2.0, 0.0, +2.0};

		Attrs vert_attrs = {
		        {"shader_id",    m_shader.id()},
		        {"data",         patch_cp_pos },
		        {"nelem",        16           },
		        {"a.a_position", 3            },
		};
		m_array.init(vert_attrs);
		m_array.set("patch_vertices", 16);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.fill = false;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewscreen = math::perspective(viewport(0), 70, 1, 100);
		u_worldview = Mat4f::orbiting(ezero(), esec, 30, -25, 17, 0, 7.65, 0, 90, 31);
		m_shader.use();
		m_array.draw(GL_PATCHES, 0, 16);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("019_bpatch_tess");
}  // namespace
}  // namespace spu::oglplus
