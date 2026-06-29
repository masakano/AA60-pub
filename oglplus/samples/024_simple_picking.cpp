//
// App :
//
#include <spu++/spu_page.h>
#include <shapes/cube.hpp>
#include <shapes/array.hpp>
#include <math/matrix.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
	
const char *c_vert =  {
    "#version 330						\n"
    "uniform mat4 u_viewscreen;                \n"
    "uniform mat4 u_worldview;                 \n"
    "in vec4 a_position;					\n"
    "out vec3 g_color;						\n"
    "flat out int g_instance_id;				\n"
    "void main()						\n"
    "{								\n"
    "       float x = gl_InstanceID % 6 - 2.5;			\n"
    "       float y = gl_InstanceID / 6 - 2.5;			\n"
    "       mat4 nodeworld = mat4(				\n"
    "                1.0, 0.0, 0.0, 0.0,			\n"
    "                0.0, 1.0, 0.0, 0.0,			\n"
    "                0.0, 0.0, 1.0, 0.0,			\n"
    "                2*x, 2*y, 0.0, 1.0				\n"
    "       );							\n"
    "       gl_Position =					\n"
    "               u_viewscreen *				\n"
    "               u_worldview *				\n"
    "               nodeworld *					\n"
    "               a_position;					\n"
    "       g_color = vec3(					\n"
    "               abs(normalize((nodeworld*a_position).xy)),	\n"
    "               0.1						\n"
    "       );							\n"
    "       g_instance_id = gl_InstanceID;			\n"
    "}								\n"
};
	
const char *c_frag =  {
    "#version 330                                                                       \n"
    "flat in int g_instance_id;                                                         \n"
    "in vec3 g_color;                                                                   \n"
    "uniform int u_picked;                                                              \n"
    "out vec4 final_color;                                                              \n"
    "void main()                                                                        \n"
    "{                                                                                  \n"
    "       if (g_instance_id == u_picked)                                              \n"
    "               final_color = vec4(1.0, 1.0, 1.0, 1.0);                             \n"
    "       else final_color = vec4(g_color, 1.0);                                      \n"
    "}                                                                                  \n"
};

const char *c_comp =  {
    "#version 440                                                                       \n"
    "layout(local_size_x = 12, local_size_y = 36, local_size_z = 1) in;                 \n"
    "uniform mat4 u_viewscreen;                                                         \n"
    "uniform mat4 u_worldview;                                                          \n"
    "uniform vec2 u_mouse_pos;                                                          \n"
    "											\n"
    "readonly buffer a_position { vec4 v[]; } b_position;                               \n"
    "writeonly buffer a_output { struct { float depth; int id; }  v[]; } b_output;      \n"
    "											\n"
    "vec2 barycentric_coords(vec4 a, vec4 b, vec4 c)                                    \n"
    "{                                                                                  \n"
        // we'll need normalized device coordinates
        // of the triangle vertices
    "       vec2 ad = vec2(a.x/a.w, a.y/a.w);                                           \n"
    "       vec2 bd = vec2(b.x/b.w, b.y/b.w);                                           \n"
    "       vec2 cd = vec2(c.x/c.w, c.y/c.w);                                           \n"
    "       vec2 u = cd - ad;                                                           \n"
    "       vec2 v = bd - ad;                                                           \n"
    "       vec2 r = u_mouse_pos - ad;                                                  \n"
    "       float d00 = dot(u, u);                                                      \n"
    "       float d01 = dot(u, v);                                                      \n"
    "       float d02 = dot(u, r);                                                      \n"
    "       float d11 = dot(v, v);                                                      \n"
    "       float d12 = dot(v, r);                                                      \n"
    "       float id = 1.0 / (d00 * d11 - d01 * d01);                                   \n"
    "       float ut = (d11 * d02 - d01 * d12) * id;                                    \n"
    "       float vt = (d00 * d12 - d01 * d02) * id;                                    \n"
    "       return vec2(ut, vt);                                                        \n"
    "}                                                                                  \n"
    "											\n"
    "vec3 intersection(vec3 a, vec3 b, vec3 c, vec2 bc)                                 \n"
    "{                                                                                  \n"
    "       return (c - a)*bc.x + (b - a)*bc.y;                                         \n"
    "}                                                                                  \n"
    "											\n"
    "bool inside_triangle(vec2 b)                                                       \n"
    "{                                                                                  \n"
    "       return (                                                                    \n"
    "               (b.x >= 0.0) &&                                                     \n"
    "               (b.y >= 0.0) &&                                                     \n"
    "               (b.x + b.y <= 1.0)                                                  \n"
    "       );                                                                          \n"
    "}                                                                                  \n"
    "											\n"
    "void main()                                                                        \n"
    "{                                                                                  \n"
    "       uint triangle_id = gl_GlobalInvocationID.x;                                 \n"
    "       uint instance_id = gl_GlobalInvocationID.y;                                 \n"
    "       float x = instance_id % 6 - 2.5;                                            \n"
    "       float y = instance_id / 6 - 2.5;                                            \n"
    "       mat4 nodeworld = mat4(							\n"
    "                1.0, 0.0, 0.0, 0.0,                                                \n"
    "                0.0, 1.0, 0.0, 0.0,                                                \n"
    "                0.0, 0.0, 1.0, 0.0,                                                \n"
    "                2*x, 2*y, 0.0, 1.0                                                 \n"
    "       );                                                                          \n"
    "       mat4 worldscreen = u_viewscreen * u_worldview * nodeworld;			\n"
    "       vec4 position0 = worldscreen * b_position.v[triangle_id * 3 + 0];		\n"
    "       vec4 position1 = worldscreen * b_position.v[triangle_id * 3 + 1];		\n"
    "       vec4 position2 = worldscreen * b_position.v[triangle_id * 3 + 2];		\n"
    "       vec2 bc = barycentric_coords(                                               \n"
    "               position0,								\n"
    "               position1,								\n"
    "               position2								\n"
    "       );                                                                          \n"
    "       if (inside_triangle(bc))                                                    \n"
    "       {                                                                           \n"
    "               vec3 position = intersection(                                       \n"
    "                       position0.xyz,						\n"
    "                       position1.xyz,						\n"
    "                       position2.xyz,						\n"
    "                       bc);                                                        \n"
    "               b_output.v[0].depth = position.z;                                   \n"
    "               b_output.v[0].id = int(instance_id);                                \n"
    "       }                                                                           \n"
    "}                                                                                  \n"
    
};

/* clang-format on */
class App : public SpuPage {
public:
	struct PickPoint {
		float depth;
		int32_t instance_id;
	};

	SpuComputeArray m_compArray;
	SpuShader m_shader;
	SpuArray m_array;

	Mat4f u_viewscreen;
	Mat4f u_worldview;
	float u_mouse_pos[2];
	int32_t u_picked;

	App(const char *name) : SpuPage(name, true, {0.9, 0.9, 0.9, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		// compute
		{
			auto &array = m_compArray;
			auto &shader = array.getShader();

			Attrs shader_attrs = {
			        {"comp", c_comp},
			};

			Attrs unif_attrs = {
			        {"u_viewscreen", &u_viewscreen  },
			        {"u_worldview",  &u_worldview   },
			        {"u_mouse_pos",  &u_mouse_pos[0]},
			};

			shapes::loadShader(shader, shader_attrs, unif_attrs);

			auto points = cubePoints();

			Attrs attrs0 = {
			        {"shader_id",    shader.id()  },
			        {"a.a_position", 4            },
			        {"nelem",        points.size()},
			        {"data",         points.data()},
			};

			PickPoint data = {0.0, 0};
			Attrs attrs1 = {
			        {"a.a_output", sizeof(PickPoint) / 4},
			        {"nelem",      1                    },
			        {"data",       &data                },
			};

			array.aux(attrs0, 0);
			array.aux(attrs1, 1);
		}

		// cube
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};

			Attrs unif_attrs = {
			        {"u_viewscreen", &u_viewscreen},
			        {"u_worldview",  &u_worldview },
			        {"u_picked",     &u_picked    },
			};
			shapes::loadShader(m_shader, shader_attrs, unif_attrs);

			auto points = cubePoints();
			Attrs array_attrs = {
			        {"shader_id",    m_shader.id()},
			        {"a.a_position", 4            },
			        {"nelem",        points.size()},
			        {"data",         points.data()},
			};
			m_array.init(array_attrs);
		}
	}

	void render() override
	{
		auto &curr = SpuPage::getGesture()->curr();
		auto esec = getSeconds().current();

		u_viewscreen = math::perspective(viewport(0), 60, 1, 80);
		u_mouse_pos[0] = curr.cursor[0] / (0.5 * curr.winsize[0]) - 1.0;
		u_mouse_pos[1] = curr.cursor[1] / (0.5 * curr.winsize[1]) - 1.0;
		u_worldview = Mat4f::orbiting(ezero(), esec, 16, 0, 0, 0, 10, 0, 30, 20);

		m_compArray.compute();

		auto *ptr = (PickPoint *)(m_compArray.map("rw", 1));
		u_picked = ptr->instance_id;
		ptr->instance_id = -1;
		m_compArray.unmap(1);

		m_shader.use();
		m_array.draw(GL_TRIANGLES, 0, 0, 36);
	}

	std::vector<Vec4f> cubePoints() const
	{
		shapes::Cube cube_shape;
		std::vector<float> datas;
		auto dim = cube_shape.positions(datas);
		assert(dim == 3);

		std::vector<Vec4f> points;
		for (auto i = 0u; i < datas.size(); i += 3u) {
			points.emplace_back(datas[i + 0], datas[i + 1], datas[i + 2], 1.0f);
		}
		return points;
	}
};
static ObjectRegistry<SpuPage>::Creator<App> page_creator("024_simple_picking");
}  // namespace
}  // namespace spu::oglplus
