//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/plane.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 440                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "layout (binding = 0) buffer a_atomic { uint ui[]; } ac;                                \n"
    "const float mult = 1.0/128.0;                                                          \n"
    "uniform float u_vc_int;                                                                  \n"
    "in vec4 a_position;                                                                    \n"
    "out vec3 g_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_viewsceen * u_worldview *  a_position;                          \n"
    "       g_color = vec3(                                                                 \n"
    "               fract(atomicAdd(ac.ui[0], 1)*mult),                                     \n"
    "               0.0,                                                                    \n"
    "               0.0                                                                     \n"
    "       )*max(u_vc_int, 0.0);                                                             \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 440                                                                           \n"
    "layout (triangles) in;                                                                 \n"
    "layout (triangle_strip, max_vertices = 3) out;                                         \n"
    "layout (binding = 0) buffer a_atomic { uint ui[]; } ac;                                \n"
    "const float mult = 1.0/128.0;                                                          \n"
    "uniform float u_gc_int;                                                                  \n"
    "in vec3 g_color[3];                                                                    \n"
    "out vec3 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 u_color = vec3(                                                            \n"
    "               0.0,                                                                    \n"
    "               fract(atomicAdd(ac.ui[1], 1)*mult),                                     \n"
    "               0.0                                                                     \n"
    "       )*max(u_gc_int, 0.0);                                                             \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "       {                                                                               \n"
    "               gl_Position = gl_in[v].gl_Position;                                     \n"
    "               f_color = g_color[v] + u_color;                                         \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 440                                                                           \n"
    "layout (binding = 0) buffer a_atomic { uint ui[]; } ac;                                \n"
    "const float mult = 1.0/4096.0;                                                         \n"
    "uniform float u_fc_int;                                                                  \n"
    "in vec3 f_color;                                                                       \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 u_color = vec3(                                                            \n"
    "               0.0,                                                                    \n"
    "               0.0,                                                                    \n"
    "               sqrt(fract(atomicAdd(ac.ui[2], 1)*mult))                                \n"
    "       )*max(u_fc_int, 0.0);                                                             \n"
    "       final_color = f_color + u_color;                                                \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	static constexpr auto c_grid_side = 8u;

	shapes::Array m_array;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	float u_vc_int;
	float u_gc_int;
	float u_fc_int;

	App(const char *name) : SpuPage(name, true, {0.2, 0.2, 0.2, 0.0}) {}

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
			        {"u_viewsceen", &u_viewsceen},
                                {"u_worldview", &u_worldview},
			        {"u_vc_int",    &u_vc_int   },
                                {"u_gc_int",    &u_gc_int   },
			        {"u_fc_int",    &u_fc_int   },
			};
			m_array.initShader(shader_attrs, unif_attrs);
		}

		{
			shapes::Plane plane_shape(
			        Vec3f(0.0, 0.0, 0.0), Vec3f(1.0, 0.0, 0.0), Vec3f(0.0, 0.0, -1.0), c_grid_side,
			        c_grid_side);

			m_array.initArray(plane_shape, {"position"});

			// atomic counter
			const uint32_t atomic_data[3] = {0u, 0u, 0u};
			Attrs attrs1 = {
			        {"a.+0",  0          }, // shader storage
			        {"nelem", 3 * 4      }, // in bytes
			        {"data",  atomic_data},
			};
			m_array.aux(attrs1, 1);
		}

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = false;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 20);
		u_worldview = Mat4f::orbiting(Vec3f(0.0, 0.5, 0.0), esec, 2.8, 0, 0, 0, 90.0, 80, 0, 0);

		u_vc_int = sin((esec / 5.0 - 0.25) * math::two_pi());
		u_gc_int = sin((esec / 6.0) * math::two_pi());
		u_fc_int = sin((esec / 7.0 + 0.25) * math::two_pi());

		m_array.draw(nullptr);

		const uint32_t tmp[3] = {0u, 0u, 0u};
		m_array.send(tmp, sizeof(tmp), 1);  // works
#if 0
		// debug (slow!)
		{
			uint32_t *ptr = (uint32_t *)m_array.map(GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, 1);
			spu_printf(0, "atomic counter = %d\n", *ptr);
			m_array.unmap(1);
		}
#endif
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("018_atomic_counters");
}  // namespace
}  // namespace spu::oglplus
