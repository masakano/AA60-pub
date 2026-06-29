//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/icosahedron.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
        //"#version 330\n"
    "#version 420                                                                           \n"
    "uniform vec3 u_view_position;                                                          \n"
    "in vec3 a_position;                                                                    \n"
    "out vec3 tc_position;                                                                  \n"
    "out float tc_distance;                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       tc_position = a_position;                                                       \n"
    "       tc_distance = length(u_view_position - tc_position);                            \n"
    "}                                                                                      \n"
};

const char *c_tesc =  {
    "#version 420                                                                           \n"
    "layout(vertices = 3) out;                                                              \n"
    "in vec3 tc_position[];                                                                 \n"
    "in float tc_distance[];                                                                \n"
    "out vec3 te_position[];                                                                \n"
    "int tess_level(float dist)                                                             \n"
    "{                                                                                      \n"
    "       return int(9.0 / sqrt(dist+0.1));                                               \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       te_position[gl_InvocationID] =                                                  \n"
    "               tc_position[gl_InvocationID];                                           \n"

    "       if (gl_InvocationID == 0)                                                       \n"
    "       {                                                                               \n"
    "               gl_TessLevelInner[0] = tess_level((                                     \n"
    "                       tc_distance[0]+                                                 \n"
    "                       tc_distance[1]+                                                 \n"
    "                       tc_distance[2]                                                  \n"
    "               )*0.333);                                                               \n"
    "               gl_TessLevelOuter[0] = tess_level((                                     \n"
    "                       tc_distance[1]+                                                 \n"
    "                       tc_distance[2]                                                  \n"
    "               )*0.5);                                                                 \n"
    "               gl_TessLevelOuter[1] = tess_level((                                     \n"
    "                       tc_distance[2]+                                                 \n"
    "                       tc_distance[0]                                                  \n"
    "               )*0.5);                                                                 \n"
    "               gl_TessLevelOuter[2] = tess_level((                                     \n"
    "                       tc_distance[0]+                                                 \n"
    "                       tc_distance[1]                                                  \n"
    "               )*0.5);                                                                 \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_tese =  {
    "#version 330                                                                           \n"
    "#extension GL_ARB_tessellation_shader : enable                                         \n"
    "layout(triangles, equal_spacing, ccw) in;                                              \n"
    "const vec3 u_light_position = vec3(12.0, 10.0, 7.0);                                   \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec3 te_position[];                                                                 \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_light_dir;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 p0 = gl_TessCoord.x * te_position[0];                                      \n"
    "       vec3 p1 = gl_TessCoord.y * te_position[1];                                      \n"
    "       vec3 p2 = gl_TessCoord.z * te_position[2];                                      \n"
    "       vec4 temp_position = vec4(normalize(p0+p1+p2), 0.0);                            \n"
    "       g_normal = (u_nodeworld * temp_position).xyz;                                   \n"
    "       temp_position.w = 1.0;                                                          \n"
    "       temp_position = u_nodeworld * temp_position;                                    \n"
    "       g_light_dir = u_light_position - temp_position.xyz;                             \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               temp_position;                                                          \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 420                                                                           \n"
    "layout (triangles) in;                                                                 \n"
    "layout (triangle_strip, max_vertices = 4) out;                                         \n"
    "uniform vec3 u_offset;                                                                 \n"
    "uniform vec2 u_viewport_dimensions;                                                    \n"
    "in vec3 g_normal[];                                                                    \n"
    "in vec3 g_light_dir[];                                                                 \n"
    "noperspective out vec3 f_dist;                                                         \n"
    "flat out vec3 f_normal;                                                                \n"
    "out vec3 f_color;                                                                      \n"
    "out vec3 f_light_dir;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_normal = normalize(                                                           \n"
    "               g_normal[0]+                                                            \n"
    "               g_normal[1]+                                                            \n"
    "               g_normal[2]                                                             \n"
    "       );                                                                              \n"
    "       vec2 screen_pos[3];                                                             \n"
    "       for (int i=0; i!=3; ++i)                                                        \n"
    "       {                                                                               \n"
    "               screen_pos[i] =                                                         \n"
    "                       u_viewport_dimensions*                                          \n"
    "                       gl_in[i].gl_Position.xy/                                        \n"
    "                       gl_in[i].gl_Position.w;                                         \n"
    "       }                                                                               \n"
    "       vec2 tmp_vect[3];                                                               \n"
    "       for (int i=0; i!=3; ++i)                                                        \n"
    "       {                                                                               \n"
    "               tmp_vect[i] =                                                           \n"
    "                       screen_pos[(i+2)%3]-                                            \n"
    "                       screen_pos[(i+1)%3];                                            \n"
    "       }                                                                               \n"
    "       const vec3 edge_mask[3] = vec3[3](                                              \n"
    "               vec3(1.0, 0.0, 0.0),                                                    \n"
    "               vec3(0.0, 1.0, 0.0),                                                    \n"
    "               vec3(0.0, 0.0, 1.0)                                                     \n"
    "       );                                                                              \n"
    "       for (int i=0; i!=3; ++i)                                                        \n"
    "       {                                                                               \n"
    "               float dist = abs(                                                       \n"
    "                       tmp_vect[(i+1)%3].x*tmp_vect[(i+2)%3].y-                        \n"
    "                       tmp_vect[(i+1)%3].y*tmp_vect[(i+2)%3].x                         \n"
    "               ) / length(tmp_vect[i]);                                                \n"
    "               vec3 dist_vect = vec3(dist, dist, dist);                                \n"
    "               gl_Position = gl_in[i].gl_Position;                                     \n"
    "               f_color = normalize(abs(                                                \n"
    "                       vec3(2.0, 2.0, 2.0)-                                            \n"
    "                       g_normal[i]-                                                    \n"
    "                       u_offset                                                        \n"
    "               ));                                                                     \n"
    "               f_light_dir = g_light_dir[i];                                           \n"
    "               f_dist = edge_mask[i] * dist_vect;                                      \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 420                                                                           \n"
    "noperspective in vec3 f_dist;                                                          \n"
    "flat in vec3 f_normal;                                                                 \n"
    "in vec3 f_color;                                                                       \n"
    "in vec3 f_light_dir;                                                                   \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float min_dist = min(min(f_dist.x,f_dist.y),f_dist.z);                          \n"
    "       float edge_alpha = exp2(-pow(min_dist, 2.0));                                   \n"
    "       const float ambient = 0.8;                                                      \n"
    "       float diffuse = max(dot(                                                        \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light_dir)                                                  \n"
    "       ), 0.0);                                                                        \n"
    "       vec3 face_color = f_color * (diffuse + ambient);                                \n"
    "       const vec3 edge_color = vec3(0.0, 0.0, 0.0);                                    \n"
    "       final_color = mix(face_color, edge_color, edge_alpha);                          \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_array;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_offset;
	Vec3f u_view_position;
	Vec2f u_viewport_dimensions;

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.8, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
                                {"tesc", c_tesc},
                                {"tese", c_tese},
			        {"geom", c_geom},
                                {"frag", c_frag},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen",           &u_viewsceen          },
			        {"u_worldview",           &u_worldview          },
			        {"u_nodeworld",           &u_nodeworld          },
			        {"u_offset",              &u_offset             },
			        {"u_view_position",       &u_view_position      },
			        {"u_viewport_dimensions", &u_viewport_dimensions},
			};
			m_array.initShader(shader_attrs, unif_attrs);
		}

		{
			shapes::SimpleIcosahedron ico_shape;
			m_array.initArray(ico_shape, {"position"});
			m_array.setMode(GL_PATCHES);
			m_array.set("patch_vertices", 3);
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewport_dimensions = Vec2f(viewport(0).sx, viewport(0).sy);
		u_viewsceen = math::perspective(viewport(0), 60, 1, 40);
		u_worldview = Mat4f::orbiting(ezero(), esec, 14, -8, 13, 0, 11, 0, 31, 21);
		const Vec3f offsets[6] = {
		        {+2, +0, +0},
                        {-2, +0, +0},
                        {+0, +2, +0},
                        {+0, -2, +0},
                        {+0, +0, +2},
                        {+0, +0, -2},
		};

		for (auto i = 0; i != 6; ++i) {
			auto model = math::unit().rot("X", esec * 11) * math::unit().trans(offsets[i])
			           * math::unit().rot("Z", esec * (37 + 9 * i));

			u_nodeworld = model;
			u_offset = offsets[i];
			u_view_position = model.unitary_inverse() * u_worldview.unitary_inverse().c[3];

			m_array.draw(nullptr);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("019_tessellation");
}  // namespace
}  // namespace spu::oglplus
