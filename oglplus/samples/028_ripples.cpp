//
// GridArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/newton.hpp>
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
    "       vec4( 0.2, 0.3, 0.1, 0.1),                                                      \n"
    "       vec4(-0.5, 0.1,-0.1, 0.2),                                                      \n"
    "       vec4(-0.1, 0.2,-0.5, 0.1),                                                      \n"
    "       vec4(-0.2, 0.2,-0.4, 0.2)                                                       \n"
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
    "               float w = source[s].w*50.0;                                             \n"
    "               float r = x*x + z*z;                                                    \n"
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
    "layout(triangle_strip, max_vertices = 4) out;                                            \n"
    "uniform mat4 u_worldscreen;                                                              \n"
    "uniform vec3 u_eye_position;                                                             \n"
    "uniform vec3 u_light_position;                                                           \n"
    "in vec3 g_normal[];                                                                      \n"
    "in float g_value[];                                                                      \n"
    "out vec3 f_normal, f_light_dir, f_view_dir;                                              \n"
    "void do_nothing(){ };                                                                    \n"
    "float find_t(int i1, int i2)                                                             \n"
    "{                                                                                        \n"
    "       float d = g_value[i2] - g_value[i1];                                              \n"
    "       return -g_value[i1]/d;                                                            \n"
    "}                                                                                        \n"
    "void make_vertex(int i1, int i2)                                                         \n"
    "{                                                                                        \n"
    "       float t = find_t(i1, i2);                                                         \n"
    "       gl_Position = mix(                                                                \n"
    "               gl_in[i1].gl_Position,                                                    \n"
    "               gl_in[i2].gl_Position,                                                    \n"
    "               t                                                                         \n"
    "       );                                                                                \n"
    "       f_normal = mix(                                                                   \n"
    "               g_normal[i1],                                                             \n"
    "               g_normal[i2],                                                             \n"
    "               t                                                                         \n"
    "       );                                                                                \n"
    "       f_light_dir = u_light_position - gl_Position.xyz;                                 \n"
    "       f_view_dir = u_eye_position - gl_Position.xyz;                                    \n"
    "       gl_Position = u_worldscreen * gl_Position;                                        \n"
    "       EmitVertex();                                                                     \n"
    "}                                                                                        \n"

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

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform samplerCube u_texture;                                                         \n"
    "in vec3 f_normal, f_light_dir, f_view_dir;                                             \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 normal = normalize(f_normal);                                              \n"
    "       vec3 light_dir = normalize(f_light_dir);                                        \n"
    "       vec3 view_dir = normalize(f_view_dir);                                          \n"
    "       float light_refl = dot(reflect(-light_dir, normal), view_dir);                  \n"
    "       float light_hit = dot(normal, light_dir);                                       \n"
    "       float specular = pow(clamp(light_refl+0.1, 0.0, 1.0), 32);                      \n"
    "       float diffuse1 = pow(max(light_hit*0.6+0.4, 0.0), 2.0);                         \n"
    "       float diffuse2 = sqrt(max(light_hit+0.2, 0.0));                                 \n"
    "       float diffuse = diffuse1 * 0.8 + diffuse2 * 0.2;                                \n"
    "       float view_light = max(0.3-dot(view_dir, light_dir), 0.0);                      \n"
    "       float ambient = 0.9;                                                            \n"
    "       vec3 environ = texture(u_texture, reflect(-view_dir, normal)).rgb;              \n"
    "       final_color =                                                                   \n"
    "               environ * ambient +                                                     \n"
    "               vec3(0.4, 0.8, 0.4) * diffuse+                                          \n"
    "               vec3(0.2, 0.2, 0.2) * specular;                                         \n"
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
	uint32_t u_texture;

	explicit GridArray(float quality)
	{
		Attrs shader_attrs = {
		        {"frag", c_frag},
                        {"geom", c_geom},
                        {"vert", c_vert}
                };

		Attrs unif_attrs = {
		        {"u_grid_offset",    &u_grid_offset   },
                        {"u_time",           &u_time          },
		        {"u_worldscreen",    &u_worldscreen   },
                        {"u_eye_position",   &u_eye_position  },
		        {"u_light_position", &u_light_position},
                        {"u_texture",        &u_texture       },
		};
		initShader(shader_attrs, unif_attrs);
		shapes::Tetrahedrons shape(1.0, 16 + quality * quality * 64);
		initArray<shapes::Tetrahedrons, shapes::Shape::WithAdjacencyTag>(shape, {"position"});
	}
};

class App : public SpuPage {
public:
	static constexpr float c_quality = 0.5;

	GridArray m_array;
	SpuTexture m_texture;
	Mat4f m_viewscreen;
	int32_t m_gridRepeat;

	App(const char *name) : SpuPage(name, true, {0.7, 0.65, 0.55, 0.0}), m_array(c_quality) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_gridRepeat = 1 + c_quality * 2;

		{
			auto image = images::NewtonFractal(
			        256, 256, Vec3f(0.1, 0.1, 0.1), Vec3f(1, 1, 1), Vec2f(-1, -1), Vec2f(1, 1),
			        images::NewtonFractal::X4Minus1(), images::NewtonFractal::DefaultMixer());

			auto unit = image.dataSize();
			std::vector<uint8_t> pix(unit * 6);

			for (auto i = 0; i < 6; i++) {
				memcpy(&pix[unit * i], image.data(), image.dataSize());
			}

			int32_t cube_target[6] = {
			        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
			};

			Attrs tex_attrs = {
			        {"target",      GL_TEXTURE_CUBE_MAP    },
			        {"iformat",     GL_RGB32F              },
			        {"width",       image.width()          },
			        {"height",      image.height()         },
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
			        {"wrap_s",      GL_CLAMP_TO_EDGE       },
			        {"wrap_t",      GL_CLAMP_TO_EDGE       },
			        {"wrap_r",      GL_CLAMP_TO_EDGE       },
			        {"data",        pix.data()             },
			        {"cube_target", cube_target            },
			};
			m_texture.init(tex_attrs);
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
		m_viewscreen = math::perspective(viewport(0), 60, 1, 100);
		m_array.u_time = esec;

		auto worldview = Mat4f::orbiting(ezero(), esec, 4.5, 1.0, 14.0, 0, 26, 55, 30, 14);

		m_array.u_eye_position = worldview.unitary_inverse().c[3];
		m_array.u_worldscreen = m_viewscreen * worldview;

		for (auto z = -m_gridRepeat; z != m_gridRepeat; ++z) {
			for (auto x = -m_gridRepeat; x != m_gridRepeat; ++x) {
				m_array.u_grid_offset = {float(x), -0.5f, float(z)};
				m_array.draw(nullptr);
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("028_ripples");
}  // namespace
}  // namespace spu::oglplus
