//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */

const char *c_sort_comp =  {
    "#version 440							                                                \n"
    "layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;	 \n"
    "readonly buffer a_input { vec4 v[]; } b_input;			                  \n"
    "writeonly buffer a_output { vec4 v[]; } b_output;			               \n"
    "uniform mat4 u_viewsceen;                                          \n"
    "uniform mat4 u_worldview;                                          \n"
    "mat4 u_matrix = u_viewsceen*u_worldview;                           \n"
    "int index[8] = int[8](0,1,2,3,4,5,6,7);                            \n"
    "const vec3 u_offset[8] = vec3[8](                                  \n"
    "       vec3(-0.5,-0.5,-0.5),                                       \n"
    "       vec3(+0.5,-0.5,-0.5),                                       \n"
    "       vec3(-0.5,+0.5,-0.5),                                       \n"
    "       vec3(+0.5,+0.5,-0.5),                                       \n"
    "       vec3(-0.5,-0.5,+0.5),                                       \n"
    "       vec3(+0.5,-0.5,+0.5),                                       \n"
    "       vec3(-0.5,+0.5,+0.5),                                       \n"
    "       vec3(+0.5,+0.5,+0.5)                                        \n"
    ");                                                                 \n"
    "vec4 child[8], child_vs[8];                                        \n"
    "void make_child(int i)                                             \n"
    "{                                                                  \n"
    "       uint index = gl_GlobalInvocationID.x;			                    \n"
    "       vec4 node = b_input.v[index];                               \n"
    "       child[i] = vec4(node.xyz+node.w*u_offset[i],node.w*0.5);    \n"
    "       child_vs[i] = u_matrix*vec4(child[i].xyz, 1.0);             \n"
    "}                                                                  \n"
    "ivec2 sorting_nw8[19] = ivec2[19](                                 \n"
    "       ivec2(0,7), ivec2(1,6), ivec2(2,5), ivec2(3,4),             \n"
    "       ivec2(0,3), ivec2(4,7), ivec2(1,2), ivec2(5,6),             \n"
    "       ivec2(0,1), ivec2(2,3), ivec2(4,5), ivec2(6,7),             \n"
    "       ivec2(2,4), ivec2(3,5),                                     \n"
    "       ivec2(1,2), ivec2(3,4), ivec2(5,6),                         \n"
    "       ivec2(2,3), ivec2(4,5)                                      \n"
    ");                                                                 \n"
    "void sort_swap_idx(ivec2 pair)                                     \n"
    "{                                                                  \n"
    "       float zx = child_vs[index[pair.x]].z;                       \n"
    "       float zy = child_vs[index[pair.y]].z;                       \n"
    "       if (zx < zy)                                                \n"
    "       {                                                           \n"
    "               int tmp = index[pair.y];                            \n"
    "               index[pair.y] = index[pair.x];                      \n"
    "               index[pair.x] = tmp;                                \n"
    "       }                                                           \n"
    "}                                                                  \n"
    "void main()                                                        \n"
    "{                                                                  \n"
    "       for (int c=0; c!=8; ++c)                                    \n"
    "               make_child(c);                                      \n"
    "       for (int s=0; s!=19; ++s)                                   \n"
    "               sort_swap_idx(sorting_nw8[s]);                      \n"
    "       uint base = gl_GlobalInvocationID.x * 8;			                 \n"
    "       for (int c=0; c!=8; ++c)                                    \n"
    "       {                                                           \n"
    "               b_output.v[base + c] = child[index[c]];             \n"
    "       }                                                           \n"
    "}                                                                  \n"
};
	
const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "mat4 u_matrix = u_viewsceen*u_worldview;                                               \n"
    "in vec4 a_pos_and_size;                                                                \n"
    "out vec3 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_matrix * vec4(a_pos_and_size.xyz, 1.0);                         \n"
    "       gl_PointSize = 9 * gl_Position.w / gl_Position.z;                               \n"
    "       f_color = normalize(vec3(1, 1, 1) - normalize(a_pos_and_size.xyz));             \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_color;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(f_color, 0.5);                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen", &u_viewsceen},
		        {"u_worldview", &u_worldview},
		};
	}
	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class Octree {
public:
	Octree(const Uniforms &unifs, uint32_t depth, float size)
	{
		m_levels = depth;
		m_nodeCounts.resize(m_levels);

		assert(m_levels != 0);

		// compute
		{
			for (auto i = 0u; i < m_levels; i++) {
				m_nodeCounts[i] = 1 << (3 * i);
			}

			m_compArrays.resize(m_levels - 1);
			for (auto i = 0u; i < m_levels - 1; i++) {
				auto &array = m_compArrays[i];
				auto &shader = array.getShader();

				Attrs shader_attrs = {
				        {"comp", c_sort_comp},
				};
				shapes::loadShader(shader, shader_attrs, Attrs(unifs));

				Attrs attrs0 = {
				        {"shader_id", shader.id()    },
				        {"a.a_input", 4              },
				        {"nelem",     m_nodeCounts[i]},
				};
				array.aux(attrs0, 0);

				Attrs attrs1 = {
				        {"shader_id",  shader.id()        },
				        {"a.a_output", 4                  },
				        {"nelem",      m_nodeCounts[i + 1]},
				};

				array.aux(attrs1, 1);

				if (i == 0) {
					float root_data[4] = {0.0, 0.0, 0.0, size};
					array.send(root_data, 1, 0);
				}
				else {
					array.link(0, m_compArrays[i - 1], 1);
				}

				array.getDim().x = (m_nodeCounts[i] + 63) / 64;  // local_size_x = 64
			}
		}
	}

	int32_t lastNodeCount() { return m_nodeCounts.back(); }
	SpuComputeArray &lastComputeArray() { return m_compArrays.back(); }

	void sort()
	{
		for (auto i = 0u; i < m_levels - 1; i++) {
			m_compArrays[i].compute();
		}
	}

private:
	uint32_t m_levels;
	std::vector<SpuComputeArray> m_compArrays;
	std::vector<uint32_t> m_nodeCounts;
};

class App : public SpuPage {
public:
	Uniforms m_unifs;
	Octree m_octree;
	SpuShader m_shader;
	SpuArray m_array;

	App(const char *name) : SpuPage(name, true, {0.05, 0.2, 0.1, 0.0}), m_octree(m_unifs, 6, 3.0) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_shape_frag},
		        {"vert", c_shape_vert},
		};

		shapes::loadShader(m_shader, shader_attrs, Attrs(m_unifs));

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.program_point_size = true;
		renderstate.flags.blend = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};

		Attrs array_attrs = {
		        {"shader_id",        m_shader.id()           },
		        {"a.a_pos_and_size", 4                       },
		        {"nelem",            m_octree.lastNodeCount()},
		};
		m_array.init(array_attrs);
		m_array.link(0, m_octree.lastComputeArray(), 1);
	}

	void render() override
	{
		auto esec = getSeconds().current();
		m_unifs.u_viewsceen = math::perspective(viewport(0), 70, 1, 200);
		m_unifs.u_worldview = Mat4f::orbiting(ezero(), esec, 8, 1, 31, 0, 17, 0, 90, 21);
		m_octree.sort();
		m_shader.use();
		m_array.draw(GL_POINTS);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("020_octree");
}  // namespace
}  // namespace spu::oglplus
