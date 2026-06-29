//
// App :
//
#include <spu++/spu_page.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/torus.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "uniform mat4 u_viewsceen,                                                              \n"
    "u_worldview, u_nodeworld;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_normal = mat3(u_nodeworld)* a_normal;                                         \n"
    "       f_light = u_light_pos - gl_Position.xyz;                                        \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "uniform vec3 u_color;                                                                  \n"
    "uniform float u_light_mult;                                                            \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = sqrt(length(f_light));                                                \n"
    "       float d = l > 0.0 ?                                                             \n"
    "               dot(                                                                    \n"
    "                       f_normal,                                                       \n"
    "                       normalize(f_light)                                              \n"
    "               ) / l : 0.0;                                                            \n"
    "       float i = 0.3 + max(d, 0.0) * u_light_mult;                                     \n"
    "       final_color = vec4(u_color*i, 1.0);                                             \n"
    "}                                                                                      \n"
};

const char *c_shadow_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "out float g_ld;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       vec3 f_normal = mat3(u_nodeworld)* a_normal;                                    \n"
    "       vec3 light_dir = u_light_pos - gl_Position.xyz;                                 \n"
    "       g_ld = dot(f_normal, normalize(light_dir));                                     \n"
    "}                                                                                      \n"
};

const char *c_shadow_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 12) out;                                         \n"
    "in float g_ld[];                                                                       \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "       {                                                                               \n"
    "               int a = v, b = (v+1)%3, c = (v+2)%3;                                    \n"
    "               vec4 pa = gl_in[a].gl_Position;                                         \n"
    "               vec4 pb = gl_in[b].gl_Position;                                         \n"
    "               vec4 pc = gl_in[c].gl_Position;                                         \n"
    "               vec4 px, py;                                                            \n"
    "               if (g_ld[a] == 0.0 && g_ld[b] == 0.0)                                   \n"
    "               {                                                                       \n"
    "                       px = pa;                                                        \n"
    "                       py = pb;                                                        \n"
    "               }                                                                       \n"
    "               else if (g_ld[a] > 0.0 && g_ld[b] < 0.0)                                \n"
    "               {                                                                       \n"
    "                       float x = g_ld[a]/(g_ld[a]-g_ld[b]);                            \n"
    "                       float y;                                                        \n"
    "                       px = mix(pa, pb, x);                                            \n"
    "                       if (g_ld[c] < 0.0)                                              \n"
    "                       {                                                               \n"
    "                               y = g_ld[a]/(g_ld[a]-g_ld[c]);                          \n"
    "                               py = mix(pa, pc, y);                                    \n"
    "                       }                                                               \n"
    "                       else                                                            \n"
    "                       {                                                               \n"
    "                               y = g_ld[c]/(g_ld[c]-g_ld[b]);                          \n"
    "                               py = mix(pc, pb, y);                                    \n"
    "                       }                                                               \n"
    "               }                                                                       \n"
    "               else continue;                                                          \n"
    "               vec3 vx = px.xyz - u_light_pos;                                         \n"
    "               vec3 vy = py.xyz - u_light_pos;                                         \n"
    "               vec4 sx = vec4(px.xyz + vx*10.0, 1.0);                                  \n"
    "               vec4 sy = vec4(py.xyz + vy*10.0, 1.0);                                  \n"
    "               vec4 cpx = u_worldview * px;                                            \n"
    "               vec4 cpy = u_worldview * py;                                            \n"
    "               vec4 csx = u_worldview * sx;                                            \n"
    "               vec4 csy = u_worldview * sy;                                            \n"
    "               gl_Position = u_viewsceen * cpy;                                        \n"
    "               EmitVertex();                                                           \n"
    "               gl_Position = u_viewsceen * cpx;                                        \n"
    "               EmitVertex();                                                           \n"
    "               gl_Position = u_viewsceen * csy;                                        \n"
    "               EmitVertex();                                                           \n"
    "               gl_Position = u_viewsceen * csx;                                        \n"
    "               EmitVertex();                                                           \n"
    "               EndPrimitive();                                                         \n"
    "               break;                                                                  \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_shadow_frag =  {
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
	enum {
		e_shape = 0,
		e_shadow,
	};
	shapes::Array m_shapeArray;
	SpuArray m_planeArray;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;

	Vec3f u_light_pos;
	Vec3f u_color;
	float u_light_mult;

	App(const char *name) : SpuPage(name, true, {0.2, 0.2, 0.2, 0.0}, 1.0, 0) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			m_shapeArray.setMaxShaderType(2);  // 0:shape, 1:shadow

			Attrs shape_shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};

			Attrs shadow_shader_attrs = {
			        {"vert", c_shadow_vert},
			        {"geom", c_shadow_geom},
			        {"frag", c_shadow_frag},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen",  &u_viewsceen },
                                {"u_worldview",  &u_worldview },
			        {"u_nodeworld",  &u_nodeworld },
                                {"u_light_pos",  &u_light_pos },
			        {"u_color",      &u_color     },
                                {"u_light_mult", &u_light_mult},
			};
			m_shapeArray.initShader(shape_shader_attrs, unif_attrs, 0);
			m_shapeArray.initShader(shadow_shader_attrs, unif_attrs, 1);

			auto torus_shape = shapes::Torus(1.0, 0.7, 72, 48);
			m_shapeArray.initArray(torus_shape, {"position", "normal"});
		}
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
		}
		{
			u_light_pos = {2.0, 9.0, 3.0};
			u_color = {0.8, 0.7, 0.4};
			u_light_mult = 2.5;
		}
		{
			// clang-format off
			float data[4 * 3] = {
				-9.0, 0.0, -9.0, -9.0, 0.0, 9.0,
				+9.0, 0.0, -9.0, +9.0, 0.0, 9.0
			};
			// clang-format on

			Attrs vert_attrs = {
			        {"shader_id",    m_shapeArray.getShaders().at(e_shape).id()},
			        {"data",         data                                      },
			        {"nelem",        4                                         },
			        {"a.a_position", 3                                         },
			};
			m_planeArray.init(vert_attrs);
		}
		{
			// clang-format off
			float data[4 * 3] = {
				-0.1, 1.0, 0.1, -0.1, 1.0, -0.1,
				+0.1, 1.0, 0.1, +0.1, 1.0, -0.1
			};
			// clang-format on

			Attrs norm_attrs = {
			        {"data",       data},
			        {"nelem",      4   },
			        {"a.u_normal", 3   },
			};
			m_planeArray.aux(norm_attrs, 1);
		}
	}

	void drawPlane()
	{
		u_nodeworld = math::unit();
		u_color = {0.8, 0.7, 0.4};

		m_shapeArray.getShaders().at(e_shape).use();
		m_planeArray.draw(GL_TRIANGLE_STRIP);
	}

	void drawShape(const Mat4f &model)
	{
		u_nodeworld = model;
		u_color = {0.9, 0.8, 0.1};

		m_shapeArray.setShaderType(e_shape);
		m_shapeArray.draw(nullptr);
	}

	void drawShadow(const Mat4f &model)
	{
		u_nodeworld = model;
		m_shapeArray.setShaderType(e_shadow);
		m_shapeArray.draw(nullptr);
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 70, 1, 30);

		u_worldview = Mat4f::orbiting(ezero(), esec, 9, 0, 0, 0, 10, 52.5, -37.5, 12.5);

		auto model
		        = math::unit().trans({0.0, 2.5, 0.0}) * Mat4f(Quatf(esec / 5 * math::two_pi(), eone()));

		auto &renderstate = SpuPage::getRenderstate();

		// shadow plane and torus
		{
			renderstate.cull_face = GL_BACK;
			renderstate.write_mask = {1, 1, 1, 1, 1};
			renderstate.flags.stencil_test = false;
			renderstate.depth_func = GL_LEQUAL;
			renderstate.use();

			u_light_mult = 0.2;

			drawPlane();
			drawShape(model);
		}

		// shadow volume
		{
			renderstate.write_mask = {0, 0, 0, 0, 0};
			renderstate.flags.stencil_test = true;
			renderstate.stencil_func = {
			        GL_ALWAYS, 0, ~0u, GL_KEEP, GL_KEEP, GL_INCR,
			        GL_ALWAYS, 0, ~0u, GL_KEEP, GL_KEEP, GL_DECR,
			};
			renderstate.use();

			renderstate.cull_face = GL_BACK;
			renderstate.use();
			drawShadow(model);

			renderstate.cull_face = GL_FRONT;
			renderstate.use();
			drawShadow(model);
		}

		// plane and torus
		{
			renderstate.cull_face = GL_BACK;
			renderstate.write_mask = {1, 1, 1, 1, 1};
			renderstate.stencil_func = {
			        GL_EQUAL, 0, ~0u, GL_KEEP, GL_KEEP, GL_KEEP,
			        GL_EQUAL, 0, ~0u, GL_KEEP, GL_KEEP, GL_KEEP,
			};
			renderstate.depth_func = GL_EQUAL;
			renderstate.use();

			u_light_mult = 2.5;
			drawPlane();
			drawShape(model);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> page_creator("026_stencil_shadow");
}  // namespace
}  // namespace spu::oglplus
