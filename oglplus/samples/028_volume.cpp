//
// GridArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/cloud.hpp>
#include <shapes/tetrahedrons.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform sampler3D u_texture;                                                           \n"
    "uniform float u_threshold;                                                             \n"
    "uniform float u_grid_step;                                                             \n"

    "in vec4 a_position;                                                                    \n"
    "out vec3 g_gradient;                                                                   \n"
    "out float g_value;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       float density = texture(u_texture, a_position.xyz).r;                           \n"
    "       g_value = density - u_threshold;                                                \n"
    "       g_gradient = vec3(0.0, 0.0, 0.0);                                               \n"
    "       for (int z=-1; z!=2; ++z)                                                       \n"
    "       for (int y=-1; y!=2; ++y)                                                       \n"
    "       for (int x=-1; x!=2; ++x)                                                       \n"
    "       {                                                                               \n"
    "               vec3 offs = vec3(u_grid_step*x, u_grid_step*y, u_grid_step*z);          \n"
    "               vec3 coord = a_position.xyz + offs;                                     \n"
    "               float diff = density - texture(u_texture, coord).r;                     \n"
    "               g_gradient += diff * offs;                                              \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles_adjacency) in;                                                        \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "uniform mat4 u_transform_matrix;                                                       \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "in vec3 g_gradient[];                                                                  \n"
    "in float g_value[];                                                                    \n"
    "out vec3 f_normal, f_light_dir, f_view_dir;                                            \n"
    "void do_nothing(){ };                                                                  \n"
    "float find_t(int i1, int i2)                                                           \n"
    "{                                                                                      \n"
    "       float d = g_value[i2] - g_value[i1];                                            \n"
    "       return -g_value[i1]/d;                                                          \n"
    "}                                                                                      \n"
    "void make_vertex(int i1, int i2)                                                       \n"
    "{                                                                                      \n"
    "       float t = find_t(i1, i2);                                                       \n"
    "       gl_Position = mix(                                                              \n"
    "               gl_in[i1].gl_Position,                                                  \n"
    "               gl_in[i2].gl_Position,                                                  \n"
    "               t                                                                       \n"
    "       );                                                                              \n"
    "       f_normal = mix(                                                                 \n"
    "               g_gradient[i1],                                                         \n"
    "               g_gradient[i2],                                                         \n"
    "               t                                                                       \n"
    "       );                                                                              \n"
    "       f_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       f_view_dir = u_eye_position - gl_Position.xyz;                                  \n"
    "       gl_Position = u_transform_matrix * gl_Position;                                 \n"
    "       EmitVertex();                                                                   \n"
    "}                                                                                      \n"
    "void make_triangle(int a1, int a2, int b1, int b2, int c1, int c2)                     \n"
    "{                                                                                      \n"
    "       make_vertex(a1, a2);                                                            \n"
    "       make_vertex(b1, b2);                                                            \n"
    "       make_vertex(c1, c2);                                                            \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
    "void make_quad(int a1,int a2,int b1,int b2,int c1,int c2,int d1,int d2)                \n"
    "{                                                                                      \n"
    "       make_vertex(a1, a2);                                                            \n"
    "       make_vertex(b1, b2);                                                            \n"
    "       make_vertex(c1, c2);                                                            \n"
    "       make_vertex(d1, d2);                                                            \n"
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
    "in vec3 f_normal, f_light_dir, f_view_dir;                                             \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 normal = normalize(f_normal);                                              \n"
    "       vec3 light_dir = normalize(f_light_dir);                                        \n"
    "       vec3 view_dir = normalize(f_view_dir);                                          \n"
    "       vec3 view_refl = reflect(-view_dir, normal);                                    \n"
    "       vec3 u_color = abs(vec3(1.0, 1.0, 1.0) - normal);                               \n"
    "       float ambient = 0.3;                                                            \n"
    "       float diffuse = max(dot(normal, light_dir), 0.0);                               \n"
    "       float specular = pow(max(dot(view_refl, light_dir), 0.0), 16.0);                \n"
    "       final_color = vec4(                                                             \n"
    "               ambient * vec3(0.2, 0.1, 0.1)+                                          \n"
    "               diffuse * u_color +                                                     \n"
    "               specular* vec3(1.0, 1.0, 1.0),                                          \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class GridArray : public shapes::Array {
public:
	float u_threshold;
	float u_grid_step;
	Mat4f u_transform_matrix;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	uint32_t u_texture;

	explicit GridArray(float quality)
	{
		Attrs shader_attrs = {
		        {"frag", c_frag},
                        {"vert", c_vert},
                        {"geom", c_geom}
                };

		Attrs unif_attrs = {
		        {"u_threshold",        &u_threshold       },
		        {"u_grid_step",        &u_grid_step       },
		        {"u_transform_matrix", &u_transform_matrix},
		        {"u_eye_position",     &u_eye_position    },
		        {"u_light_position",   &u_light_position  },
		        {"u_texture",          &u_texture         },
		};
		Array::initShader(shader_attrs, unif_attrs);

		m_gridDiv = 64 + (quality > 0.75 ? 64 : 0);
		shapes::Tetrahedrons shape(1.0, m_gridDiv);
		Array::initArray<shapes::Tetrahedrons, shapes::Shape::WithAdjacencyTag>(shape, {"position"});
	}
	double uStep() const { return 1.0 / m_gridDiv; }

private:
	uint32_t m_gridDiv;
};

class App : public SpuPage {
public:
	static constexpr float c_quality = 0.5;

	GridArray m_array;
	SpuTexture m_texture;

	App(const char *name) : SpuPage(name, true, {0.8, 0.7, 0.6, 0.0}), m_array(c_quality) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_array.u_grid_step = m_array.uStep();

		{
			auto cloud = images::Cloud(128, 128, 128, Vec3f(0, 0, 0), 0.5, 0.5, 0.5, 0.1);

			auto border = Vec4f(0, 0, 0, 0);

			Attrs attrs = {
			        {"target",     GL_TEXTURE_3D     },
			        {"min_filter", GL_LINEAR         },
			        {"mag_filter", GL_LINEAR         },
			        {"wrap_s",     GL_CLAMP_TO_BORDER},
			        {"wrap_t",     GL_CLAMP_TO_BORDER},
			        {"wrap_r",     GL_CLAMP_TO_BORDER},
			        {"border",     border            },

			        {"width",      128               }, // use image interface...
			        {"height",     128               },
			        {"depth",      128               },
			        {"iformat",    GL_R8             },
			        //{"data",        cloud.data<unsigned char>()},
			        {"data",       cloud.data()      },
			};
			m_texture.init(attrs);
		}
		m_array.u_texture = m_texture.id();

		const auto light_position = Vec3f(12.0, 1.0, 8.0);
		m_array.u_light_position = light_position;

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.flags.ccw = false;
		renderstate.cull_face = GL_BACK;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto viewscreen = math::perspective(viewport(0), 65, 1, 30);
		auto worldview = Mat4f::orbiting(ezero(), esec, 2.0, 0.2, 14.0, 0, 19, 45, 40, 17);
		auto nodeworld = math::unit().trans({-0.5, -0.5, -0.5});

		m_array.u_eye_position = worldview.unitary_inverse().c[3];
		m_array.u_transform_matrix = viewscreen * worldview * nodeworld;
		m_array.u_threshold = 0.5 + sin(esec / 7.0 * math::two_pi()) * 0.4;
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("028_volume");
}  // namespace
}  // namespace spu::oglplus
