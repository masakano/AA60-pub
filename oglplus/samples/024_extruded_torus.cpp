//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/torus.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out gl_PerVertex {                                                                     \n"
    "       vec4 gl_Position;                                                               \n"
    "};                                                                                     \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec2 g_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       g_normal = (u_nodeworld*vec4(a_normal,0.0)).xyz;                                \n"
    "       g_texcoord = a_texcoord;                                                        \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                            \n"
    "layout(triangles) in;                                                                   \n"
    "layout(triangle_strip, max_vertices = 15) out;                                          \n"
    "uniform mat4 u_worldview;                                                               \n"
    "uniform mat4 u_viewsceen;                                                               \n"
    "uniform vec3 u_light_pos;                                                               \n"
    "uniform float u_time;                                                                   \n"
    "in gl_PerVertex {                                                                       \n"
    "       vec4 gl_Position;                                                                \n"
    "} gl_in[];                                                                              \n"
    "in vec3 g_normal[];                                                                     \n"
    "in vec2 g_texcoord[];                                                                   \n"
    "out gl_PerVertex {                                                                      \n"
    "       vec4 gl_Position;                                                                \n"
    "};                                                                                      \n"
    "out vec3 f_normal;                                                                      \n"
    "out vec3 f_light;                                                                       \n"
    "out float f_glow;                                                                       \n"
    "flat out int f_top;                                                                     \n"
    "void main()                                                                             \n"
    "{                                                                                       \n"
    "       vec3 face_normal = normalize(                                                    \n"
    "               g_normal[0]+                                                             \n"
    "               g_normal[1]+                                                             \n"
    "               g_normal[2]                                                              \n"
    "       );                                                                               \n"
    "       vec2 face_coord = 0.33333 * (                                                    \n"
    "               g_texcoord[0]+                                                           \n"
    "               g_texcoord[1]+                                                           \n"
    "               g_texcoord[2]                                                            \n"
    "       );                                                                               \n"
    "       float offs = (sin((face_coord.s + u_time/10.0) * 3.14 * 2.0 * 10)*0.5 + 0.5)*0.4;\n"
    "       offs *= cos(face_coord.t * 3.1415 * 2.0)*0.5 + 0.51;                             \n"
    "       vec3 pos[3], norm[3];                                                            \n"
    "       for (int i=0; i!=3; ++i)                                                         \n"
    "               pos[i] = gl_in[i].gl_Position.xyz;                                       \n"
    "       for (int i=0; i!=3; ++i)                                                         \n"
    "               norm[i] = cross(                                                         \n"
    "                       face_normal,                                                     \n"
    "                       normalize(pos[(i+1)%3] - pos[i])                                 \n"
    "               );                                                                       \n"
    "       vec3 pofs = face_normal * offs;                                                  \n"
    "       f_top = 0;                                                                       \n"
    "       for (int i=0; i!=3; ++i)                                                         \n"
    "       {                                                                                \n"
    "               f_normal = norm[i];                                                      \n"
    "               for (int j=0; j!=2; ++j)                                                 \n"
    "               {                                                                        \n"
    "                       vec3 tpos = pos[(i+j)%3];                                        \n"
    "                       f_light = u_light_pos-tpos;                                      \n"
    "                       f_glow = 1.0;                                                    \n"
    "                       gl_Position =                                                    \n"
    "                               u_viewsceen *                                            \n"
    "                               u_worldview *                                            \n"
    "                               vec4(tpos, 1.0);                                         \n"
    "                       EmitVertex();                                                    \n"
    "                       f_glow = 0.7;                                                    \n"
    "                       f_light = u_light_pos-tpos+pofs;                                 \n"
    "                       gl_Position =                                                    \n"
    "                               u_viewsceen *                                            \n"
    "                               u_worldview *                                            \n"
    "                               vec4(tpos + pofs, 1.0);                                  \n"
    "                       EmitVertex();                                                    \n"
    "               }                                                                        \n"
    "               EndPrimitive();                                                          \n"
    "       }                                                                                \n"
    "       f_glow = 0.0;                                                                    \n"
    "       f_top = 1;                                                                       \n"
    "       for (int i=0; i!=3; ++i)                                                         \n"
    "       {                                                                                \n"
    "               f_light = u_light_pos - (pos[i]+pofs);                                   \n"
    "               f_normal = g_normal[i];                                                  \n"
    "               gl_Position =                                                            \n"
    "                       u_viewsceen *                                                    \n"
    "                       u_worldview *                                                    \n"
    "                       vec4(pos[i] + pofs, 1.0);                                        \n"
    "               EmitVertex();                                                            \n"
    "       }                                                                                \n"
    "       EndPrimitive();                                                                  \n"
    "}                                                                                       \n"
};

const char *c_face_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "in float f_glow;                                                                       \n"
    "flat in int f_top;                                                                     \n"
    "uniform vec3 u_top_color;                                                              \n"
    "uniform vec3 u_side_color;                                                             \n"
    "const vec3 light_color = vec3(1.0, 1.0, 1.0);                                          \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float d = max(dot(                                                              \n"
    "               normalize(f_light),                                                     \n"
    "               normalize(f_normal)                                                     \n"
    "       ), 0.0);                                                                        \n"
    "       vec3 color;                                                                     \n"
    "       if (f_top != 0)                                                                 \n"
    "       {                                                                               \n"
    "               color = u_top_color * d +                                               \n"
    "                       light_color * pow(d, 8.0);                                      \n"
    "       }                                                                               \n"
    "       else                                                                            \n"
    "       {                                                                               \n"
    "               color = u_side_color * f_glow +                                         \n"
    "                       light_color *                                                   \n"
    "                       pow(d, 2.0) * 0.2;                                              \n"
    "       }                                                                               \n"
    "       final_color = vec4(color, 1.0);                                                 \n"
    "}                                                                                      \n"
};

const char *c_frame_frag =  {
    "#version 330                                                                           \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(0.2, 0.1, 0.0, 1.0);                                         \n"
        //"     final_color = vec4(1.0, 1.0, 1.0, 1.0);"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	enum {
		e_face = 0,
		e_frame,
	};

	shapes::Array m_shapeArray;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;
	Vec3f u_top_color;
	Vec3f u_side_color;
	float u_time;

	App(const char *name) : SpuPage(name, true, {0.7, 0.6, 0.5, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_shapeArray.setMaxShaderType(2);  // 0:face 1:frame

		{
			Attrs shader_attrs = {
			        {"geom", c_geom     },
			        {"vert", c_vert     },
			        {"frag", c_face_frag},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen",  &u_viewsceen },
                                {"u_worldview",  &u_worldview },
			        {"u_nodeworld",  &u_nodeworld },
                                {"u_light_pos",  &u_light_pos },
			        {"u_time",       &u_time      },

			        {"u_top_color",  &u_top_color },
                                {"u_side_color", &u_side_color},
			};
			m_shapeArray.initShader(shader_attrs, unif_attrs, 0);
		}
		{
			Attrs shader_attrs = {
			        {"geom", c_geom      },
			        {"vert", c_vert      },
			        {"frag", c_frame_frag},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen},
                                {"u_worldview", &u_worldview},
			        {"u_nodeworld", &u_nodeworld},
                                {"u_light_pos", &u_light_pos},
			        {"u_time",      &u_time     },
			};
			m_shapeArray.initShader(shader_attrs, unif_attrs, 1);
		}
		u_light_pos = {4, 4, -8};
		u_top_color = {0.2, 0.2, 0.2};
		u_side_color = {0.9, 0.9, 0.2};

		auto torus_shape = shapes::Torus(1.0, 0.5, 18, 36);
		m_shapeArray.initArray(torus_shape, {"position", "normal", "texcoord"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 30);
		u_worldview = Mat4f::orbiting(ezero(), esec, 4.5, 0, 0, 0, 35, 0, 60, 30);
		u_nodeworld
		        = math::unit().rot("xy", -esec * 0.33 * math::two_pi(), -esec * 0.25 * math::two_pi());
		u_time = esec;

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.fill = true;
		renderstate.use();
		m_shapeArray.setShaderType(e_face);
		m_shapeArray.draw(nullptr);

		renderstate.flags.fill = false;
		renderstate.use();
		m_shapeArray.setShaderType(e_frame);
		m_shapeArray.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("024_extruded_torus");
}  // namespace
}  // namespace spu::oglplus
