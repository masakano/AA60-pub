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
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "layout (triangles) in;                                                                 \n"
    "layout (triangle_strip, max_vertices = 48) out;                                        \n"
    "const vec3 u_light_position = vec3(12.0, 10.0, 7.0);                                   \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec2 u_viewport_dimensions;                                                    \n"
    "uniform int u_tess_level;                                                              \n"
    "noperspective out vec3 f_dist;                                                         \n"
    "flat out vec3 f_normal;                                                                \n"
    "out vec3 f_color;                                                                      \n"
    "out vec3 f_light_dir;                                                                  \n"
    "void make_triangle(vec4 p0, vec4 p1, vec4 p2)                                          \n"
    "{                                                                                      \n"
    "       vec3 n0 = (u_nodeworld*vec4(p0.xyz, 0)).xyz;                                    \n"
    "       vec3 n1 = (u_nodeworld*vec4(p1.xyz, 0)).xyz;                                    \n"
    "       vec3 n2 = (u_nodeworld*vec4(p2.xyz, 0)).xyz;                                    \n"
    "       vec4 m0 = u_nodeworld*p0;                                                       \n"
    "       vec4 m1 = u_nodeworld*p1;                                                       \n"
    "       vec4 m2 = u_nodeworld*p2;                                                       \n"
    "       vec4 c0 = u_viewsceen*u_worldview*m0;                                           \n"
    "       vec4 c1 = u_viewsceen*u_worldview*m1;                                           \n"
    "       vec4 c2 = u_viewsceen*u_worldview*m2;                                           \n"
    "       vec2 s0 = u_viewport_dimensions * c0.xy/c0.w;                                   \n"
    "       vec2 s1 = u_viewport_dimensions * c1.xy/c1.w;                                   \n"
    "       vec2 s2 = u_viewport_dimensions * c2.xy/c2.w;                                   \n"
    "       vec2 v0 = s2 - s1;                                                              \n"
    "       vec2 v1 = s0 - s2;                                                              \n"
    "       vec2 v2 = s1 - s0;                                                              \n"
    "       float d0 = abs(v1.x*v2.y-v1.y*v2.x)/length(v0);                                 \n"
    "       float d1 = abs(v2.x*v0.y-v2.y*v0.x)/length(v1);                                 \n"
    "       float d2 = abs(v0.x*v1.y-v0.y*v1.x)/length(v2);                                 \n"
    "       f_normal = normalize(n0+n1+n2);                                                 \n"
    "       gl_Position = c0;                                                               \n"
    "       f_color = normalize(abs(vec3(1, 1, 1) - n0));                                   \n"
    "       f_light_dir = u_light_position - m0.xyz;                                        \n"
    "       f_dist = vec3(d0, 0.0, 0.0);                                                    \n"
    "       EmitVertex();                                                                   \n"
    "       gl_Position = c1;                                                               \n"
    "       f_color = normalize(abs(vec3(1, 1, 1) - n1));                                   \n"
    "       f_light_dir = u_light_position - m1.xyz;                                        \n"
    "       f_dist = vec3(0.0, d1, 0.0);                                                    \n"
    "       EmitVertex();                                                                   \n"
    "       gl_Position = c2;                                                               \n"
    "       f_color = normalize(abs(vec3(1, 1, 1) - n2));                                   \n"
    "       f_light_dir = u_light_position - m2.xyz;                                        \n"
    "       f_dist = vec3(0.0, 0.0, d2);                                                    \n"
    "       EmitVertex();                                                                   \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
    "void do_tess_1(vec4 p_0, vec4 p_1, vec4 p_2, int l)                                    \n"
    "{                                                                                      \n"
    "       if (l == 1) make_triangle(p_0, p_1, p_2);                                       \n"
    "       else                                                                            \n"
    "       {                                                                               \n"
    "               vec4 p01 = vec4(normalize(p_0.xyz+p_1.xyz), 1.0);                       \n"
    "               vec4 p12 = vec4(normalize(p_1.xyz+p_2.xyz), 1.0);                       \n"
    "               vec4 p20 = vec4(normalize(p_2.xyz+p_0.xyz), 1.0);                       \n"
    "               make_triangle(p_0, p01, p20);                                           \n"
    "               make_triangle(p01, p_1, p12);                                           \n"
    "               make_triangle(p20, p12, p_2);                                           \n"
    "               make_triangle(p01, p12, p20);                                           \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
    "void do_tess_0(vec4 p_0, vec4 p_1, vec4 p_2, int l)                                    \n"
    "{                                                                                      \n"
    "       if (l == 0) make_triangle(p_0, p_1, p_2);                                       \n"
    "       else                                                                            \n"
    "       {                                                                               \n"
    "               vec4 p01 = vec4(normalize(p_0.xyz+p_1.xyz), 1.0);                       \n"
    "               vec4 p12 = vec4(normalize(p_1.xyz+p_2.xyz), 1.0);                       \n"
    "               vec4 p20 = vec4(normalize(p_2.xyz+p_0.xyz), 1.0);                       \n"
    "               do_tess_1(p_0, p01, p20, l);                                            \n"
    "               do_tess_1(p01, p_1, p12, l);                                            \n"
    "               do_tess_1(p20, p12, p_2, l);                                            \n"
    "               do_tess_1(p01, p12, p20, l);                                            \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       do_tess_0(                                                                      \n"
    "               gl_in[0].gl_Position,                                                   \n"
    "               gl_in[1].gl_Position,                                                   \n"
    "               gl_in[2].gl_Position,                                                   \n"
    "               u_tess_level                                                            \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"

    "noperspective in vec3 f_dist;                                                          \n"
    "flat in vec3 f_normal;                                                                 \n"
    "in vec3 f_color;                                                                       \n"
    "in vec3 f_light_dir;                                                                   \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float min_dist = min(min(f_dist.x,f_dist.y),f_dist.z);                          \n"
    "       float edge_alpha = exp2(-pow(min_dist, 2.0));                                   \n"
    "       const float ambient = 0.7;                                                      \n"
    "       float diffuse = max(dot(                                                        \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light_dir)                                                  \n"
    "       )+0.1, 0.0)*1.4;                                                                \n"
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
	Vec2f u_viewport_dimensions;
	uint32_t u_tess_level;

	App(const char *name) : SpuPage(name, true, {0.7, 0.7, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		        {"geom", c_geom},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen",           &u_viewsceen          },
                        {"u_worldview",           &u_worldview          },
		        {"u_nodeworld",           &u_nodeworld          },
                        {"u_viewport_dimensions", &u_viewport_dimensions},
		        {"u_tess_level",          &u_tess_level         },
		};
		m_array.initShader(shader_attrs, unif_attrs);

		shapes::Icosahedron ico_shape;
		m_array.initArray(ico_shape, {"position"});
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewport_dimensions = Vec2f(viewport(0).sx, viewport(0).sy);
		u_viewsceen = math::perspective(viewport(0), 60, 1, 50);
		u_worldview = Mat4f::orbiting(ezero(), esec, 15, -8, 13, 0, 11, 0, 31, 21);

		const Vec3f offsets[4] = {
		        {+2, 0, +0},
		        {-2, 0, +0},
		        {+0, 0, +2},
		        {+0, 0, -2},
		};

		auto camera_position = Vec3f(u_worldview.unitary_inverse().c[3]);
		for (auto i = 0; i != 4; ++i) {
			u_nodeworld = math::unit().rot("X", esec * 11) * math::unit().trans(offsets[i])
			            * math::unit().rot("Z", esec * (37 + 3 * i));

			u_tess_level
			        = 17.0f
			        / length((u_nodeworld.unitary_inverse() * Vec4f(camera_position, 1.0)) + 0.1);

			m_array.draw(nullptr);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("019_gs_tessell");
}  // namespace
}  // namespace spu::oglplus
