//
// GridArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/tetrahedrons.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_grid_offset;                                                            \n"
    "uniform float u_time;                                                                  \n"
    "in vec4 a_position;                                                                    \n"
    "out vec3 g_normal;                                                                     \n"
    "out float g_value;                                                                     \n"
    "const vec4 source[4] = vec4[4](                                                        \n"
    "       vec4(-0.2, 0.3, 0.1, 0.3),                                                      \n"
    "       vec4(-0.5, 3.1,-0.1, 0.1),                                                      \n"
    "       vec4( 0.2, 0.4,-0.4, 0.3),                                                      \n"
    "       vec4(-0.1, 0.2,-0.5, 0.7)                                                       \n"
    ");                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position + vec4(u_grid_offset, 0.0);                            \n"
    "       g_value = -gl_Position.y;                                                       \n"
    "       g_normal = vec3(0.0, 1.0, 0.0);                                                 \n"
    "       for (int s=0; s!=4; ++s)                                                        \n"
    "       {                                                                               \n"
    "               float x = gl_Position.x - source[s].x;                                  \n"
    "               float z = gl_Position.z - source[s].z;                                  \n"
    "               float a = source[s].y;                                                  \n"
    "               float w = source[s].w*20.0;                                             \n"
    "               float r = 0.7*x*x + 0.4*z*z;                                            \n"
    "               float t = r + s - u_time*(0.5 - 0.1*s);                                 \n"
    "               float g = w * t;                                                        \n"
    "               float d = 2.0*a*exp(-r-1.0)*(w*cos(g)-sin(g));                          \n"
    "               g_value += sin(g)*exp(-r-1.0)*a;                                        \n"
    "               g_normal += vec3(x*d, 0.0, z*d);                                        \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                             \n"
    "layout(triangles_adjacency) in;                                                          \n"
    "layout(triangle_strip, max_vertices = 6) out;                                            \n"
    "uniform mat4 u_worldscreen;                                                              \n"
    "uniform vec3 u_eye_position;                                                             \n"
    "uniform vec3 u_light_position;                                                           \n"
    "uniform vec2 u_viewport_dimensions;                                                      \n"
    "in vec3 g_normal[];                                                                      \n"
    "in float g_value[];                                                                      \n"
    "noperspective out vec3 f_dist;                                                           \n"
    "out float f_specular, f_diffuse, f_value;                                                \n"
    "void do_nothing(){ }                                                                     \n"
    "float find_t(int i1, int i2)                                                             \n"
    "{                                                                                        \n"
    "       return g_value[i1]/(g_value[i1] - g_value[i2]);                                   \n"
    "}                                                                                        \n"
    "vec4 make_position(int i1, int i2, float t)                                              \n"
    "{                                                                                        \n"
    "       return mix(gl_in[i1].gl_Position,gl_in[i2].gl_Position,t);                        \n"
    "}                                                                                        \n"
    "void make_vertex(                                                                        \n"
    "       int i1,                                                                           \n"
    "       int i2,                                                                           \n"
    "       float t,                                                                          \n"
    "       vec4 p0,                                                                          \n"
    "       vec4 p1,                                                                          \n"
    "       vec4 p2,                                                                          \n"
    "       vec4 c0,                                                                          \n"
    "       vec2 s0,                                                                          \n"
    "       vec2 s1,                                                                          \n"
    "       vec2 s2,                                                                          \n"
    "       vec3 m                                                                            \n"
    ")                                                                                        \n"
    "{                                                                                        \n"
    "       vec3 normal = normalize(mix(g_normal[i1], g_normal[i2], t));                      \n"
    "       vec3 light_dir = normalize(u_light_position - p0.xyz);                            \n"
    "       vec3 view_dir = normalize(u_eye_position - p0.xyz);                               \n"
    "       float light_refl = dot(reflect(-light_dir, normal), view_dir);                    \n"
    "       float light_hit = dot(normal, light_dir);                                         \n"
    "       gl_Position = c0;                                                                 \n"
    "       f_specular = pow(clamp(light_refl+0.1, 0.0, 1.0), 32);                            \n"
    "       f_diffuse = pow(max((light_hit*0.7+0.3), 0.0), 2.0);                              \n"
    "       f_value = pow(dot(normalize(p1-p0), normalize(p2-p0)),2.0);                       \n"
    "       vec2 v0 = s2 - s1;                                                                \n"
    "       vec2 v1 = s2 - s0;                                                                \n"
    "       vec2 v2 = s1 - s0;                                                                \n"
    "       float a = abs(v1.x*v2.y - v1.y*v2.x);                                             \n"
    "       float d = a / length(v0);                                                         \n"
    "       f_dist = m * vec3(d, d, d);                                                       \n"
    "       EmitVertex();                                                                     \n"
    "}                                                                                        \n"

    "void make_triangle(int a1, int a2, int b1, int b2, int c1, int c2)                     \n"
    "{                                                                                      \n"
    "       float ta = find_t(a1, a2);                                                      \n"
    "       float tb = find_t(b1, b2);                                                      \n"
    "       float tc = find_t(c1, c2);                                                      \n"
    "       vec4 pa = make_position(a1, a2, ta);                                            \n"
    "       vec4 pb = make_position(b1, b2, tb);                                            \n"
    "       vec4 pc = make_position(c1, c2, tc);                                            \n"
    "       vec4 ca = u_worldscreen*pa;                                                       \n"
    "       vec4 cb = u_worldscreen*pb;                                                       \n"
    "       vec4 cc = u_worldscreen*pc;                                                       \n"
    "       vec2 sa = u_viewport_dimensions * ca.xy / ca.w;                                 \n"
    "       vec2 sb = u_viewport_dimensions * cb.xy / cb.w;                                 \n"
    "       vec2 sc = u_viewport_dimensions * cc.xy / cc.w;                                 \n"
    "       make_vertex(a1, a2, ta, pa, pb, pc, ca, sa, sb, sc, vec3(1,0,0));               \n"
    "       make_vertex(b1, b2, tb, pb, pa, pc, cb, sb, sa, sc, vec3(0,1,0));               \n"
    "       make_vertex(c1, c2, tc, pc, pa, pb, cc, sc, sa, sb, vec3(0,0,1));               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"

    "void make_quad(int a1,int a2,int b1,int b2,int c1,int c2,int                           \n"
    "d1,int d2)                                                                             \n"
    "{                                                                                      \n"
    "       float ta = find_t(a1, a2);                                                      \n"
    "       float tb = find_t(b1, b2);                                                      \n"
    "       float tc = find_t(c1, c2);                                                      \n"
    "       float td = find_t(d1, d2);                                                      \n"

    "       vec4 pa = make_position(a1, a2, ta);                                            \n"
    "       vec4 pb = make_position(b1, b2, tb);                                            \n"
    "       vec4 pc = make_position(c1, c2, tc);                                            \n"
    "       vec4 pd = make_position(d1, d2, td);                                            \n"

    "       vec4 ca = u_worldscreen*pa;                                                       \n"
    "       vec4 cb = u_worldscreen*pb;                                                       \n"
    "       vec4 cc = u_worldscreen*pc;                                                       \n"
    "       vec4 cd = u_worldscreen*pd;                                                       \n"

    "       vec2 sa = u_viewport_dimensions * ca.xy / ca.w;                                 \n"
    "       vec2 sb = u_viewport_dimensions * cb.xy / cb.w;                                 \n"
    "       vec2 sc = u_viewport_dimensions * cc.xy / cc.w;                                 \n"
    "       vec2 sd = u_viewport_dimensions * cd.xy / cd.w;                                 \n"

    "       make_vertex(a1, a2, ta, pa, pb, pc, ca, sa, sb, sc, vec3(1,0,0));               \n"
    "       make_vertex(b1, b2, tb, pb, pa, pc, cb, sb, sa, sc, vec3(0,1,0));               \n"
    "       make_vertex(c1, c2, tc, pc, pa, pb, cc, sc, sa, sb, vec3(0,0,1));               \n"
    "       EndPrimitive();                                                                 \n"

    "       make_vertex(c1, c2, tc, pc, pb, pd, cc, sc, sb, sd, vec3(1,0,0));               \n"
    "       make_vertex(b1, b2, tb, pb, pc, pd, cb, sb, sc, sd, vec3(0,1,0));               \n"
    "       make_vertex(d1, d2, td, pd, pc, pb, cd, sd, sc, sb, vec3(0,0,1));               \n"
    "       EndPrimitive();                                                                 \n"

    "}                                                                                      \n"

    "void process_tetrahedron(int a, int b, int c, int d)                                   \n"
    "{                                                                                      \n"
    "       if (g_value[a] >= 0.0)                                                          \n"
    "       {                                                                               \n"
    "               if (g_value[b] >= 0.0)                                                  \n"
    "               {                                                                       \n"
    "                       if (g_value[c] >= 0.0)                                          \n"
    "                       {                                                               \n"
    "                               if (g_value[d] >= 0.0)                                  \n"
    "                                       do_nothing();                                   \n"
    "                               else make_triangle(d,a, d,b, d,c);                      \n"
    "                       }                                                               \n"
    "                       else                                                            \n"
    "                       {                                                               \n"
    "                               if (g_value[d] >= 0.0)                                  \n"
    "                                       make_triangle(c,a, c,d, c,b);                   \n"
    "                               else make_quad(c,a, d,a, c,b, d,b);                     \n"
    "                       }                                                               \n"
    "               }                                                                       \n"
    "               else                                                                    \n"
    "               {                                                                       \n"
    "                       if (g_value[c] >= 0.0)                                          \n"
    "                       {                                                               \n"
    "                               if (g_value[d] >= 0.0)                                  \n"
    "                                       make_triangle(b,c, b,d, b,a);                   \n"
    "                               else make_quad(b,c, d,c, b,a, d,a);                     \n"
    "                       }                                                               \n"
    "                       else                                                            \n"
    "                       {                                                               \n"
    "                               if (g_value[d] >= 0.0)                                  \n"
    "                                       make_quad(c,a, c,d, b,a, b,d);                  \n"
    "                               else make_triangle(c,a, d,a, b,a);                      \n"
    "                       }                                                               \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       else                                                                            \n"
    "       {                                                                               \n"
    "               if (g_value[b] >= 0.0)                                                  \n"
    "               {                                                                       \n"
    "                       if (g_value[c] >= 0.0)                                          \n"
    "                       {                                                               \n"
    "                               if (g_value[d] >= 0.0)                                  \n"
    "                                       make_triangle(a,b, a,d, a,c);                   \n"
    "                               else make_quad(a,c, a,b, d,c, d,b);                     \n"
    "                       }                                                               \n"
    "                       else                                                            \n"
    "                       {                                                               \n"
    "                               if (g_value[d] >= 0.0)                                  \n"
    "                                       make_quad(a,b, a,d, c,b, c,d);                  \n"
    "                               else make_triangle(a,b, d,b, c,b);                      \n"
    "                       }                                                               \n"
    "               }                                                                       \n"
    "               else                                                                    \n"
    "               {                                                                       \n"
    "                       if (g_value[c] >= 0.0)                                          \n"
    "                       {                                                               \n"
    "                               if (g_value[d] >= 0.0)                                  \n"
    "                                       make_quad(b,c, b,d, a,c, a,d);                  \n"
    "                               else make_triangle(b,c, d,c, a,c);                      \n"
    "                       }                                                               \n"
    "                       else                                                            \n"
    "                       {                                                               \n"
    "                               if (g_value[d] >= 0.0)                                  \n"
    "                                       make_triangle(a,d, c,d, b,d);                   \n"
    "                               else do_nothing();                                      \n"
    "                       }                                                               \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "}                                                                                      \n"

    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       process_tetrahedron(0, 2, 4, 1);                                                \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "noperspective in vec3 f_dist;                                                          \n"
    "in float f_specular, f_diffuse, f_value;                                               \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float min_dist = min(min(f_dist.x, f_dist.y), f_dist.z);                        \n"
    "       float edge_alpha = exp2(-pow(min_dist, 2.0));                                   \n"
    "       float ambient = 0.8;                                                            \n"
    "       vec3 u_color = vec3(                                                            \n"
    "               1.0-f_value*(gl_FrontFacing?1:0),                                       \n"
    "               1.0-f_value*(gl_FrontFacing?0:1),                                       \n"
    "               1.0-f_value                                                             \n"
    "       );                                                                              \n"
    "       vec3 fill = u_color * (ambient + f_diffuse + f_specular);                       \n"
    "       vec3 line = vec3(0, 0, 0);                                                      \n"
    "       final_color = mix(fill, line, edge_alpha);                                      \n"
    "}                                                                                      \n"
};

/* clang-format on */
class GridArray : public shapes::Array {
public:
	Vec3f u_grid_offset;
	float u_time;
	Mat4f u_worldscreen;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	Vec2f u_viewport_dimensions;

	explicit GridArray(float quality)
	{
		Attrs shader_attrs = {
		        {"frag", c_frag},
                        {"vert", c_vert},
                        {"geom", c_geom}
                };

		Attrs unif_attrs = {
		        {"u_grid_offset",         &u_grid_offset        },
		        {"u_time",                &u_time               },
		        {"u_worldscreen",         &u_worldscreen        },
		        {"u_eye_position",        &u_eye_position       },
		        {"u_light_position",      &u_light_position     },
		        {"u_viewport_dimensions", &u_viewport_dimensions},
		};
		initShader(shader_attrs, unif_attrs);

		shapes::Tetrahedrons shape(1.0, 8 + quality * quality * 8);
		initArray<shapes::Tetrahedrons, shapes::Shape::WithAdjacencyTag>(shape, {"position"});
	}
};

class App : public SpuPage {
public:
	static constexpr float c_quality = 0.5;

	GridArray m_array;
	Mat4f m_viewscreen;
	int32_t m_arrayRepeat;

	App(const char *name) : SpuPage(name, true, {0.1, 0.1, 0.1, 0.0}), m_array(c_quality) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		const auto light_position = Vec3f(12.0, 1.0, 8.0);

		m_arrayRepeat = 1 + c_quality * 2;
		m_array.u_light_position = light_position;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		m_viewscreen = math::perspective(viewport(0), 60, 1, 60);
		m_array.u_viewport_dimensions = Vec2f(viewport(0).sx, viewport(0).sy);
		m_array.u_time = esec;

		auto worldview = Mat4f::orbiting(ezero(), esec, 4, -1, 14, 0, 26, 55, 30, 14);
		m_array.u_eye_position = worldview.unitary_inverse().c[3];
		m_array.u_worldscreen = m_viewscreen * worldview;

		for (auto y = -1; y != 1; ++y) {
			for (auto z = -m_arrayRepeat; z != m_arrayRepeat; ++z) {
				for (auto x = -m_arrayRepeat; x != m_arrayRepeat; ++x) {
					m_array.u_grid_offset = {float(x), float(y), float(z)};
					m_array.draw(nullptr);
				}
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("029_surface");
}  // namespace
}  // namespace spu::oglplus
