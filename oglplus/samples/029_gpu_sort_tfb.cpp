//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/sort_nw.hpp>
#include <shapes/spiral_sphere.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_mesh_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "                   u_viewsceen*u_worldview*u_nodeworld*a_position;                     \n"
    "}                                                                                      \n"
};

const char *c_mesh_frag =  {
    "#version 330                                                                           \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec3(0.1);                                                        \n"
    "}                                                                                      \n"
};

const char *c_dist_comp =  {
    "#version 440							                                                \n"
    "layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;	 \n"
    "readonly buffer a_position { vec4 v[]; } b_position;		             \n"
    "writeonly buffer a_index { uint v[]; } b_index;			                 \n"
    "writeonly buffer a_distance { float v[]; } b_distance;		           \n"
    "uniform mat4 u_viewsceen;                                          \n"
    "uniform mat4 u_worldview;                                          \n"
    "void main()                                                        \n"
    "{                                                                  \n"
    "       uint index = gl_GlobalInvocationID.x;			                    \n"
    "       vec4 p = u_viewsceen*u_worldview*b_position.v[index];       \n"
    "       b_index.v[index] = index;					                              \n"
    "       b_distance.v[index] = p.z;                                  \n"
    "}                                                                  \n"
};
	
const char *c_sort_comp =  {
    "#version 440								                                                       \n"
    "layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;		        \n"
    "uniform usampler2DRect u_texture;                                          \n"
    "uniform int u_pass;                                                        \n"
    "layout (packed) uniform u_index_block{ uint  indices[4096]; };             \n"
    "layout (packed) uniform u_distance_block { float distances[4096]; };       \n"
    "readonly buffer a_input { uint v[]; } b_input;				                         \n"
    "writeonly buffer a_output { uint v[]; } b_output;				                      \n"
    "void main()                                                                \n"
    "{                                                                          \n"
    "       uint id = gl_GlobalInvocationID.x;					                             \n"
    "       uint enc = texelFetch(u_texture, ivec2(id, u_pass)).r;              \n"
    "       if (enc != 0u)                                                      \n"
    "       {                                                                   \n"
    "               int sig1 = ((enc & 0x1u) != 0u)?-1:1;                       \n"
    "               int sig2 = ((enc & 0x2u) != 0u)?-1:1;                       \n"
    "               int offs = sig2 * int(enc >> 2);                            \n"
    "               uint index2 = indices[id+offs];                             \n"
    "               float diff = distances[b_input.v[id]] - distances[index2];  \n"
    "               if (diff*sig1*sig2 < 0)                                     \n"
    "               {                                                           \n"
    "                       b_output.v[id] = index2;                            \n"
    "                       return;                                             \n"
    "               }                                                           \n"
    "       }                                                                   \n"
    "       b_output.v[id] = b_input.v[id];                                     \n"
    "}                                                                          \n"
};
	

const char *c_draw_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "out vec3 g_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_worldview * a_position;                                         \n"
    "       g_color = normalize(vec3(1)-a_position.rgb);                                    \n"
    "}                                                                                      \n"
};

const char *c_draw_geom =  {
    "#version 330                                                                           \n"
    "layout(points) in;                                                                     \n"
    "layout(triangle_strip, max_vertices = 4) out;                                          \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "in vec3 g_color[1];                                                                    \n"
    "out vec3 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_color = g_color[0];                                                           \n"
    "       float s = 0.1;                                                                  \n"
    "       float yo[2] = float[2](-1.0, 1.0);                                              \n"
    "       float xo[2] = float[2](-1.0, 1.0);                                              \n"
    "       for (int j=0;j!=2;++j)                                                          \n"
    "       for (int i=0;i!=2;++i)                                                          \n"
    "       {                                                                               \n"
    "               float xoffs = xo[i]*s;                                                  \n"
    "               float yoffs = yo[j]*s;                                                  \n"
    "               gl_Position = u_viewsceen * vec4(                                       \n"
    "                       gl_in[0].gl_Position.x-xoffs,                                   \n"
    "                       gl_in[0].gl_Position.y-yoffs,                                   \n"
    "                       gl_in[0].gl_Position.z,                                         \n"
    "                       1.0                                                             \n"
    "               );                                                                      \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_draw_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_color;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float a = 1.0/16.0;                                                             \n"
    "       final_color = vec4(f_color, a);                                                 \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	static constexpr uint32_t c_max_count = 4096;  // from u_index_block/DistBlock

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	int32_t u_pass;
	uint32_t u_texture;
	uint32_t m_indexBlock[c_max_count];
	float m_distBlock[c_max_count];

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen", &u_viewsceen},
                        {"u_worldview", &u_worldview},
		        {"u_nodeworld", &u_nodeworld},
                        {"u_pass",      &u_pass     },
		        {"u_texture",   &u_texture  },
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class MeshArray : public shapes::Array {
public:
	explicit MeshArray(const Uniforms &unifs)
	{
		Attrs shader_attrs = {
		        {"frag", c_mesh_frag},
                        {"vert", c_mesh_vert}
                };

		Array::initShader(shader_attrs, Attrs(unifs));
		Array::initArray(shapes::SpiralSphere(), {"position"});
	}
};

class DistArray {
public:
	uint32_t m_positionId;
	uint32_t m_indexId;
	uint32_t m_distanceId;
	uint32_t m_passCount;

	explicit DistArray(Uniforms &unifs)
	{
		genTexcoord(unifs);
		auto pos_data = genPosData(Uniforms::c_max_count);

		// compute
		{
			auto &array = m_compArray;
			auto &shader = array.getShader();

			Attrs attrs = {
			        {"comp", c_dist_comp},
			};
			shapes::loadShader(shader, attrs, Attrs(unifs));

			Attrs attrs0 = {
			        {"shader_id",    shader.id()          },
			        {"a.a_position", 4                    },
			        {"data",         pos_data.data()      },
			        {"nelem",        Uniforms::c_max_count},
			};
			array.aux(attrs0, 0);

			Attrs attrs1 = {
			        {"shader_id", shader.id()          },
			        {"a.a_index", 1                    },
			        {"nelem",     Uniforms::c_max_count},
			};
			array.aux(attrs1, 1);

			Attrs attrs2 = {
			        {"shader_id",    shader.id()          },
			        {"a.a_distance", 1                    },
			        {"nelem",        Uniforms::c_max_count},
			};
			array.aux(attrs2, 2);

			array.getDim().x = Uniforms::c_max_count / 64;  // local_size_z = 64

			array.get("0.buffer_id", &m_positionId);
			array.get("1.buffer_id", &m_indexId);
			array.get("2.buffer_id", &m_distanceId);
		}
	}

	void draw() { m_compArray.compute(); }

private:
	SpuComputeArray m_compArray;
	SpuTexture m_texture;

	std::vector<Vec4f> genPosData(int32_t count)
	{
		RandomGenerator<float> frand = {-1.0, +1.0};
		std::vector<Vec4f> pos_data(count);
		for (auto p = 0; p != count; ++p) {
			pos_data[p].x = frand();
			pos_data[p].y = frand();
			pos_data[p].z = frand();
			pos_data[p].w = 1.0;
		}
		return pos_data;
	}

	void genTexcoord(Uniforms &unifs)
	{
		images::Image image = images::SortNWMap(Uniforms::c_max_count);

		Attrs attrs = {
		        {"target",      GL_TEXTURE_RECTANGLE},
                        {"iformat",     GL_R16UI            },
		        {"data",        image.data()        },
                        {"width",       image.width()       },
		        {"height",      image.height()      },
                        {"wrap_s",      GL_CLAMP_TO_EDGE    },
		        {"wrap_t",      GL_CLAMP_TO_EDGE    },
                        {"min_filter",  GL_NEAREST          },
		        {"mag_filter",  GL_NEAREST          },
                        {"auto_mipmap", 0                   },
		};
		m_passCount = image.height();
		m_texture.init(attrs);
		unifs.u_texture = m_texture.id();
	}
};

class SortArray {
public:
	SortArray(Uniforms &unifs, DistArray &dist_proc) : m_unifs(unifs)
	{
		m_passCount = dist_proc.m_passCount;

		auto &array = m_compArray;
		auto &shader = array.getShader();

		Attrs init_attrs = {
		        {"comp", c_sort_comp},
		};
		shapes::loadShader(shader, init_attrs, Attrs(unifs));

		Attrs set_attrs = {
		        {"u_index_block.buffer_id",    &dist_proc.m_indexId   },
		        {"u_distance_block.buffer_id", &dist_proc.m_distanceId},
		};
		shader.set(set_attrs);

		Attrs attrs0 = {
		        {"shader_id", shader.id()          },
		        {"buffer_id", dist_proc.m_indexId  },
		        {"a.a_input", 1                    },
		        {"nelem",     Uniforms::c_max_count},
		};
		array.init(attrs0);

		Attrs attrs1 = {
		        {"shader_id",  shader.id()          },
		        {"a.a_output", 1                    },
		        {"nelem",      Uniforms::c_max_count},
		};
		array.aux(attrs1, 1);

		array.getDim().x = Uniforms::c_max_count / 64;  // local_size_z = 64
	}

	void draw()
	{
		for (auto p = 0u; p != m_passCount; ++p) {
			m_unifs.u_pass = p;
			m_compArray.compute();
			m_compArray.copy(0, m_compArray, 1);
		}
	}

private:
	SpuComputeArray m_compArray;
	uint32_t m_passCount;
	Uniforms &m_unifs;
};

class DrawArray : public SpuArray {
public:
	DrawArray(const Uniforms &unifs, DistArray &dist_array)
	{
		uint32_t max_count = Uniforms::c_max_count;

		// program
		{
			Attrs attrs = {
			        {"frag", c_draw_frag},
			        {"geom", c_draw_geom},
			        {"vert", c_draw_vert},
			};
			shapes::loadShader(m_shader, attrs, Attrs(unifs));
		}
		// array
		{
			Attrs attrs = {
			        {"shader_id",    m_shader.id()          },
			        {"index_id",     dist_array.m_indexId   },
			        {"buffer_id",    dist_array.m_positionId},
			        {"a.a_position", 4                      },
			        {"nelem",        max_count              },
			};
			SpuArray::init(attrs);
		}
	}

	void draw()
	{
		SpuScopedRenderstate renderstate(true);
		renderstate.flags.blend = true;
		renderstate.use();
		m_shader.use();
		SpuArray::draw(GL_POINTS, 0, Uniforms::c_max_count);
	}

private:
	SpuShader m_shader;
};

class App : public SpuPage {
public:
	Uniforms m_unifs;
	MeshArray m_meshArray;
	DistArray m_distArray;
	SortArray m_sortArray;
	DrawArray m_drawArray;

	App(const char *name)
	        : SpuPage(name, true, {0.3, 0.3, 0.3, 0.0}), m_meshArray(m_unifs), m_distArray(m_unifs),
	          m_sortArray(m_unifs, m_distArray), m_drawArray(m_unifs, m_distArray)
	{
	}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
		renderstate.use();
	}

	void render() override
	{
		auto esec = getSeconds().current();
		m_unifs.u_viewsceen = math::perspective(viewport(0), 60, 1, 60);

		m_unifs.u_nodeworld = Mat4f(Quatf(esec / 13.0 * math::two_pi(), eone()));
		m_unifs.u_worldview = Mat4f::orbiting(ezero(), esec, 5, 1, 11, 0, 23, 0, 80, 17);

		m_meshArray.draw(nullptr);
		m_distArray.draw();
		m_sortArray.draw();
		m_drawArray.draw();
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("029_gpu_sort_tfb");
}  // namespace
}  // namespace spu::oglplus
