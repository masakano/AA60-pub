//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/spiral_sphere.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_normal_view;                                                                \n"
    "out vec3 f_light;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_normal = mat3(u_nodeworld)* a_normal;                                         \n"
    "       f_normal_view = mat3(u_worldview)*f_normal;                                     \n"
    "       f_light = u_light_pos - gl_Position.xyz;                                        \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_normal_view;                                                                 \n"
    "in vec3 f_light;                                                                       \n"
    "uniform mat4 u_worldview;                                                              \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float ltlen = sqrt(length(f_light));                                            \n"
    "       float ltexp = dot(                                                              \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light)                                                      \n"
    "       );                                                                              \n"
    "       float lview = dot(                                                              \n"
    "               normalize(f_light),                                                     \n"
    "               normalize(vec3(                                                         \n"
    "                       u_worldview[0][2],                                              \n"
    "                       u_worldview[1][2],                                              \n"
    "                       u_worldview[2][2]                                               \n"
    "               ))                                                                      \n"
    "       );                                                                              \n"
    "       float depth = normalize(f_normal_view).z;                                       \n"
    "       vec3 ftrefl = vec3(0.9, 0.8, 0.7);                                              \n"
    "       vec3 scatter = vec3(0.9, 0.6, 0.1);                                             \n"
    "       vec3 bklt = vec3(0.8, 0.6, 0.4);                                                \n"
    "       vec3 ambient = vec3(0.5, 0.4, 0.3);                                             \n"
    "       final_color = vec4(                                                             \n"
    "               pow(max(ltexp, 0.0), 8.0)*ftrefl+                                       \n"
    "               (ltexp+1.0)/ltlen*pow(depth,2.0)*scatter+                               \n"
    "               (-ltexp+1.0)/ltlen*(1.0-depth)*scatter+                                 \n"
    "               (-lview+1.0)*0.6*(1.0-abs(depth))*bklt+                                 \n"
    "               0.2*ambient,                                                            \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_plane_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               a_position;                                                             \n"
    "       f_normal = a_normal;                                                            \n"
    "       f_light = u_light_pos-a_position.xyz;                                           \n"
    "}                                                                                      \n"
};

const char *c_plane_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = sqrt(length(f_light));                                                \n"
    "       float e = dot(                                                                  \n"
    "               f_normal,                                                               \n"
    "               normalize(f_light)                                                      \n"
    "       );                                                                              \n"
    "       float d = l > 0.0 ? e / l : 0.0;                                                \n"
    "       float i = 0.2 + 2.5 * d;                                                        \n"
    "       final_color = vec4(0.8*i, 0.7*i, 0.4*i, 1.0);                                   \n"
    "}                                                                                      \n"
};

const char *c_halo_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "out vec3 g_normal;                                                                     \n"
    "out float g_vd;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_worldview *                                                           \n"
    "               u_nodeworld *                                                           \n"
    "               a_position;                                                             \n"
    "       g_normal = (                                                                    \n"
    "               u_worldview *                                                           \n"
    "               u_nodeworld *                                                           \n"
    "               vec4(a_normal, 0.0)                                                     \n"
    "       ).xyz;                                                                          \n"
    "       g_vd = g_normal.z;                                                              \n"
    "}                                                                                      \n"
};

const char *c_halo_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 12) out;                                         \n"
    "in vec3 g_normal[];                                                                    \n"
    "in float g_vd[];                                                                       \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "out float f_alpha;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "       {                                                                               \n"
    "               int a = v, b = (v+1)%3, c = (v+2)%3;                                    \n"
    "               vec4 pa = gl_in[a].gl_Position;                                         \n"
    "               vec4 pb = gl_in[b].gl_Position;                                         \n"
    "               vec4 pc = gl_in[c].gl_Position;                                         \n"
    "               vec4 px, py;                                                            \n"
    "               vec3 na = g_normal[a];                                                  \n"
    "               vec3 nb = g_normal[b];                                                  \n"
    "               vec3 nc = g_normal[c];                                                  \n"
    "               vec3 nx, ny;                                                            \n"
    "               if (g_vd[a] == 0.0 && g_vd[b] == 0.0)                                   \n"
    "               {                                                                       \n"
    "                       px = pa;                                                        \n"
    "                       nx = na;                                                        \n"
    "                       py = pb;                                                        \n"
    "                       ny = nb;                                                        \n"
    "               }                                                                       \n"
    "               else if (g_vd[a] > 0.0 && g_vd[b] < 0.0)                                \n"
    "               {                                                                       \n"
    "                       float x = g_vd[a]/(g_vd[a]-g_vd[b]);                            \n"
    "                       float y;                                                        \n"
    "                       px = mix(pa, pb, x);                                            \n"
    "                       nx = mix(na, nb, x);                                            \n"
    "                       if (g_vd[c] < 0.0)                                              \n"
    "                       {                                                               \n"
    "                               y = g_vd[a]/(g_vd[a]-g_vd[c]);                          \n"
    "                               py = mix(pa, pc, y);                                    \n"
    "                               ny = mix(na, nc, y);                                    \n"
    "                       }                                                               \n"
    "                       else                                                            \n"
    "                       {                                                               \n"
    "                               y = g_vd[c]/(g_vd[c]-g_vd[b]);                          \n"
    "                               py = mix(pc, pb, y);                                    \n"
    "                               ny = mix(nc, nb, y);                                    \n"
    "                       }                                                               \n"
    "               }                                                                       \n"
    "               else continue;                                                          \n"
    "               vec4 gx1 = vec4(px.xyz, 1.0);                                           \n"
    "               vec4 gy1 = vec4(py.xyz, 1.0);                                           \n"
    "               vec4 gx2 = vec4(px.xyz + nx*0.3, 1.0);                                  \n"
    "               vec4 gy2 = vec4(py.xyz + ny*0.3, 1.0);                                  \n"
    "               gl_Position = u_viewsceen * gy1;                                        \n"
    "               f_alpha = 1.0;                                                          \n"
    "               EmitVertex();                                                           \n"
    "               gl_Position = u_viewsceen * gx1;                                        \n"
    "               f_alpha = 1.0;                                                          \n"
    "               EmitVertex();                                                           \n"
    "               gl_Position = u_viewsceen * gy2;                                        \n"
    "               f_alpha = 0.0;                                                          \n"
    "               EmitVertex();                                                           \n"
    "               gl_Position = u_viewsceen * gx2;                                        \n"
    "               f_alpha = 0.0;                                                          \n"
    "               EmitVertex();                                                           \n"
    "               EndPrimitive();                                                         \n"
    "               break;                                                                  \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_halo_frag =  {
    "#version 330                                                                           \n"
    "in float f_alpha;                                                                      \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(                                                             \n"
    "               0.5, 0.4, 0.3,                                                          \n"
    "               pow(f_alpha, 2.0)                                                       \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	enum {
		e_shape = 0,
		e_halo,
	};
	shapes::Array m_shapeArray;

	SpuShader m_planeShader;
	SpuArray m_planeArray;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;

	App(const char *name) : SpuPage(name, true, {0.2, 0.2, 0.2, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			m_shapeArray.setMaxShaderType(2);  // 0:shape 1:halo

			Attrs shape_shader_attrs = {
			        {"frag", c_shape_frag},
			        {"vert", c_shape_vert},
			};

			Attrs halo_shader_attrs = {
			        {"frag", c_halo_frag},
			        {"geom", c_halo_geom},
			        {"vert", c_halo_vert},
			};

			Attrs unif_attrs = {
			        {"u_light_pos", &u_light_pos},
			        {"u_worldview", &u_worldview},
			        {"u_viewsceen", &u_viewsceen},
			        {"u_nodeworld", &u_nodeworld},
			};
			m_shapeArray.initShader(shape_shader_attrs, unif_attrs, 0);
			m_shapeArray.initShader(halo_shader_attrs, unif_attrs, 1);
			m_shapeArray.initArray(shapes::SpiralSphere(), {"position", "normal"});
		}

		{
			Attrs shader_attrs = {
			        {"frag", c_plane_frag},
			        {"vert", c_plane_vert},
			};

			Attrs unif_attrs = {
			        {"u_light_pos", &u_light_pos},
			        {"u_worldview", &u_worldview},
			        {"u_viewsceen", &u_viewsceen},
			        {"u_nodeworld", &u_nodeworld},
			};
			shapes::loadShader(m_planeShader, shader_attrs, unif_attrs);
		}

		{
			float data[4 * 3] = {-9.0, 0.0, 9.0, -9.0, 0.0, -9.0, +9.0, 0.0, 9.0, +9.0, 0.0, -9.0};

			Attrs vert_attrs = {
			        {"shader_id",    m_planeShader.id()},
			        {"a.a_position", 3                 },
			        {"nelem",        4                 },
			        {"data",         data              },
			};
			m_planeArray.init(vert_attrs);
		}
		{
			float data[4 * 3] = {-0.1, 1.0, 0.1, -0.1, 1.0, -0.1, 0.1, 1.0, 0.1, 0.1, 1.0, -0.1};
			Attrs norm_attrs = {
			        {"a.a_normal", 3   },
			        {"nelem",      4   },
			        {"data",       data},
			};
			m_planeArray.aux(norm_attrs, 1);
		}

		Vec3f light_pos(2.0, 2.5, 9.0);

		u_light_pos = light_pos;

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.blend = false;  // enable later

		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE,
		        GL_SRC_ALPHA,
		        GL_ONE,
		};
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 70, 1, 40);
		u_worldview = Mat4f::orbiting(ezero(), esec, 8.5, 0, 0, 0, 5, 52.5, -37.5, 10);
		u_nodeworld = math::unit().trans({0.0, 2.5, 0.0})
		            * Mat4f(Quatf(esec / 7.0 * math::two_pi(), eone()));

		m_planeShader.use();
		m_planeArray.draw(GL_TRIANGLE_STRIP);

		m_shapeArray.setShaderType(e_shape);
		m_shapeArray.draw(nullptr);

		SpuScopedRenderstate renderstate(true);
		renderstate.write_mask.z = 0;
		renderstate.flags.blend = true;
		renderstate.use();

		m_shapeArray.setShaderType(e_halo);
		m_shapeArray.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("026_shape_halo");
}  // namespace
}  // namespace spu::oglplus
