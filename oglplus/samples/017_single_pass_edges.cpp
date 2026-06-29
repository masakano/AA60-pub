//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/subdiv_sphere.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 420                                                                           \n"
    "const vec3 u_light_position = vec3(10.0, 10.0, 7.0);                                   \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_light_dir;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       g_normal = (                                                                    \n"
    "               u_nodeworld * vec4(a_position.xyz, 0.0)                                 \n"
    "       ).xyz;                                                                          \n"
    "       g_light_dir = u_light_position -  gl_Position.xyz;                              \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 420                                                                           \n"
    "layout (triangles) in;                                                                 \n"
    "layout (triangle_strip, max_vertices = 3) out;                                         \n"
    "uniform vec2 u_viewports;                                                              \n"
    "in vec3 g_normal[];                                                                    \n"
    "in vec3 g_light_dir[];                                                                 \n"
    "noperspective out vec3 f_dist;                                                         \n"
    "flat out vec3 f_normal;                                                                \n"
    "flat out vec3 f_color;                                                                 \n"
    "out vec3 f_light_dir;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_normal = normalize(                                                           \n"
    "               g_normal[0]+                                                            \n"
    "               g_normal[1]+                                                            \n"
    "               g_normal[2]                                                             \n"
    "       );                                                                              \n"
    "       f_color = normalize(abs(                                                        \n"
    "               vec3(1.0, 1.0, 1.0)-                                                    \n"
    "               f_normal                                                                \n"
    "       ));                                                                             \n"
    "       vec2 screen_pos[3];                                                             \n"
    "       for (int i=0; i!=3; ++i)                                                        \n"
    "       {                                                                               \n"
    "               screen_pos[i] =                                                         \n"
    "                       u_viewports*                                                    \n"
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
    "               f_light_dir = g_light_dir[i];                                           \n"
    "               f_dist = edge_mask[i] * dist_vect;                                      \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 420                                                                           \n"
    "uniform float u_edge_width;                                                            \n"
    "noperspective in vec3 f_dist;                                                          \n"
    "flat in vec3 f_normal;                                                                 \n"
    "flat in vec3 f_color;                                                                  \n"
    "in vec3 f_light_dir;                                                                   \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float min_dist = min(min(f_dist.x,f_dist.y),f_dist.z);                          \n"
    "       float edge_alpha = exp2(-pow(min_dist/u_edge_width, 2.0));                      \n"
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
	Vec2f u_viewports;
	float u_edge_width;

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.8, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
                                {"geom", c_geom},
                                {"frag", c_frag}
                        };

			Attrs unif_attrs = {
			        {"u_viewsceen",  &u_viewsceen },
                                {"u_worldview",  &u_worldview },
			        {"u_nodeworld",  &u_nodeworld },
                                {"u_viewports",  &u_viewports },
			        {"u_edge_width", &u_edge_width},
			};

			m_array.initShader(shader_attrs, unif_attrs);
		}

		{
			shapes::SimpleSubdivSphere make_shape(1, shapes::SubdivSphereInitialShape::Octohedron);

			m_array.initArray(make_shape, {"position"});

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.flags.clip_distance0 = true;
			renderstate.cull_face = GL_BACK;
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewports = Vec2f(viewport(0).sx, viewport(0).sy);
		u_viewsceen = math::perspective(viewport(0), 68, 1, 20);
		u_edge_width = 4.0 + sin(esec / 7.0 * math::two_pi()) * 3.0;
		u_nodeworld = math::unit().rot("z", -esec / 9.79 * math::two_pi());
		u_worldview = Mat4f::orbiting(ezero(), esec, 5.5, -2.0, 27, 0, 11.5, 0, 31, 21);
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("017_single_pass_edges");
}  // namespace
}  // namespace spu::oglplus
