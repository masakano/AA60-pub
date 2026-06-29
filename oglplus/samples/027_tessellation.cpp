//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 420                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform float u_step;                                                                  \n"
    "uniform int u_inst_count;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_prev_cp, a_next_cp;                                                          \n"
    "in vec2 a_tess_level;                                                                  \n"
    "out vec4 tc_position;                                                                  \n"
    "out vec3 tc_prev_cp, tc_next_cp;                                                       \n"
    "out vec3 tc_color;                                                                     \n"
    "out vec2 tc_tess_level;                                                                \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float xoffs = gl_InstanceID*u_step-(u_inst_count*u_step)*0.5;                   \n"
    "       tc_color = normalize(normalize(vec3(1,1,1))-normalize(a_position.xyz));         \n"
    "       tc_position = u_worldview * (a_position+vec4(xoffs, 0, 0, 0));                  \n"
    "       tc_prev_cp = mat3(u_worldview) * a_prev_cp;                                     \n"
    "       tc_next_cp = mat3(u_worldview) * a_next_cp;                                     \n"
    "       tc_tess_level = a_tess_level;                                                   \n"
    "}                                                                                      \n"
};

const char *c_tesc =  {
    "#version 440                                                                           \n"
    "#define id gl_InvocationID                                                             \n"
    "layout(vertices = 4) out;                                                              \n"
    "in vec4 tc_position[];                                                                 \n"
    "in vec3 tc_prev_cp[];                                                                  \n"
    "in vec3 tc_next_cp[];                                                                  \n"
    "in vec3 tc_color[];                                                                    \n"
    "in vec2 tc_tess_level[];                                                               \n"
    "patch out vec3 te_position[4];                                                         \n"
    "patch out vec3 te_prev_cp[4];                                                          \n"
    "patch out vec3 te_next_cp[4];                                                          \n"
    "patch out vec3 te_color[4];                                                            \n"
    "float calc_tl(vec3 v)                                                                  \n"
    "{                                                                                      \n"
    "       float d = pow(dot(vec3(0.0, 0.0,-1.0), normalize(v)), 3);                       \n"
    "       return clamp(d/pow(abs(v.z)*0.1+1.5, 1.5), 0.0, 1.0);                           \n"
    "}                                                                                      \n"

    "vec3 mid(int id1, int id2)                                                             \n"
    "{                                                                                      \n"
    "       return mix(tc_position[id1].xyz, tc_position[id2].xyz, 0.5);                    \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       int next_id = (id+2)%4;                                                         \n"
    "       int prev_id = (id+3)%4;                                                         \n"
    "       if (id == 0 || id == 1)                                                         \n"
    "       {                                                                               \n"
    "               gl_TessLevelInner[id] = int(                                            \n"
    "                       (tc_tess_level[id].y+tc_tess_level[next_id].y)*                 \n"
    "                       calc_tl(mix(mid(0,2), mid(1,3), 0.5))                           \n"
    "               );                                                                      \n"
    "       }                                                                               \n"
    "       gl_TessLevelOuter[id] = 1+int(tc_tess_level[prev_id].x*calc_tl(mid(prev_id, id)));\n"
    "       te_position[id] = tc_position[id].xyz;                                          \n"
    "       te_prev_cp[id] = tc_prev_cp[id];                                                \n"
    "       te_next_cp[id] = tc_next_cp[id];                                                \n"
    "       te_color[id] = tc_color[id];                                                    \n"
    "}                                                                                      \n"
};

const char* c_tese = {
    "#version 420                                                                                \n"
    "layout(quads, equal_spacing, ccw) in;                                                       \n"
    "uniform mat4 u_viewsceen;                                                                   \n"
    "patch in vec3 te_position[4];                                                               \n"
    "patch in vec3 te_prev_cp[4];                                                                \n"
    "patch in vec3 te_next_cp[4];                                                                \n"
    "patch in vec3 te_color[4];                                                                  \n"
    "out vec4 g_patch_distance;                                                                  \n"
    "out vec3 g_color;                                                                           \n"
    "vec3 positions[16] = vec3[16](                                                              \n"
    "       te_position[0],                                                                      \n"
    "       te_position[0]+te_next_cp[0],                                                        \n"
    "       te_position[1]+te_prev_cp[1],                                                        \n"
    "       te_position[1],                                                                      \n"
    "       te_position[0]+te_prev_cp[0],                                                        \n"
    "       te_position[0]+te_prev_cp[0]+te_next_cp[0],                                          \n"
    "       te_position[1]+te_prev_cp[1]+te_next_cp[1],                                          \n"
    "       te_position[1]+te_next_cp[1],                                                        \n"
    "       te_position[3]+te_next_cp[3],                                                        \n"
    "       te_position[3]+te_prev_cp[3]+te_next_cp[3],                                          \n"
    "       te_position[2]+te_prev_cp[2]+te_next_cp[2],                                          \n"
    "       te_position[2]+te_prev_cp[2],                                                        \n"
    "       te_position[3],                                                                      \n"
    "       te_position[3]+te_prev_cp[3],                                                        \n"
    "       te_position[2]+te_next_cp[2],                                                        \n"
    "       te_position[2]                                                                       \n"
    ");                                                                                          \n"
    "const mat4 b = mat4(                                                                        \n"
    "       -1, 3,-3, 1,                                                                         \n"
    "        3,-6, 3, 0,                                                                         \n"
    "       -3, 3, 0, 0,                                                                         \n"
    "        1, 0, 0, 0                                                                          \n"
    ");                                                                                          \n"
    "mat4 px, py, pz;                                                                            \n"
    "void main()                                                                                 \n"
    "{                                                                                           \n"
    "       float u = gl_TessCoord.x;                                                            \n"
    "       float v = gl_TessCoord.y;                                                            \n"
    "       for (int j=0; j!=4; ++j)                                                             \n"
    "       for (int i=0; i!=4; ++i)                                                             \n"
    "       {                                                                                    \n"
    "               int k = j*4+i;                                                               \n"
    "               px[j][i] = positions[k].x;                                                   \n"
    "               py[j][i] = positions[k].y;                                                   \n"
    "               pz[j][i] = positions[k].z;                                                   \n"
    "       }                                                                                    \n"
    "       mat4 cx = b * px * b;                                                                \n"
    "       mat4 cy = b * py * b;                                                                \n"
    "       mat4 cz = b * pz * b;                                                                \n"
    "       vec4 up = vec4(u*u*u, u*u, u, 1);                                                    \n"
    "       vec4 vp = vec4(v*v*v, v*v, v, 1);                                                    \n"
    "       vec4 temp_position = vec4(dot(cx * vp, up), dot(cy * vp, up), dot(cz * vp, up), 1.0);\n"
    "       gl_Position = u_viewsceen * temp_position;                                           \n"
    "       g_patch_distance = vec4(u, v, 1.0-u, 1.0-v);                                         \n"
    "       g_color = mix(                                                                       \n"
    "               mix(te_color[0], te_color[1], u),                                            \n"
    "               mix(te_color[3], te_color[2], u),                                            \n"
    "               v                                                                            \n"
    "       );                                                                                   \n"
    "}                                                                                           \n"
};

const char *c_geom =  {
    "#version 420                                                                           \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 3) out;                                          \n"
    "uniform vec2 u_viewport_dimensions;                                                    \n"
    "in vec4 g_patch_distance[3];                                                           \n"
    "in vec3 g_color[3];                                                                    \n"
    "out vec3 f_edge;                                                                       \n"
    "out vec3 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec2 screen_pos[3];                                                             \n"
    "       vec4 patch_dist[3];                                                             \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "       {                                                                               \n"
    "               screen_pos[v] =                                                         \n"
    "                       u_viewport_dimensions*                                          \n"
    "                       gl_in[v].gl_Position.xy/                                        \n"
    "                       gl_in[v].gl_Position.w;                                         \n"
    "               patch_dist[v] = vec4(                                                   \n"
    "                       g_patch_distance[v].x < 1.0 ? 0.0 : 1.0,                        \n"
    "                       g_patch_distance[v].y < 1.0 ? 0.0 : 1.0,                        \n"
    "                       g_patch_distance[v].z < 1.0 ? 0.0 : 1.0,                        \n"
    "                       g_patch_distance[v].w < 1.0 ? 0.0 : 1.0                         \n"
    "               );                                                                      \n"
    "       }                                                                               \n"
    "       vec2 temp_vect[3];                                                              \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "       {                                                                               \n"
    "               temp_vect[v] =                                                          \n"
    "                       screen_pos[(v+2)%3]-                                            \n"
    "                       screen_pos[(v+1)%3];                                            \n"
    "       }                                                                               \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "       {                                                                               \n"
    "               float dist = abs(                                                       \n"
    "                       temp_vect[(v+1)%3].x * temp_vect[(v+2)%3].y-                    \n"
    "                       temp_vect[(v+1)%3].y * temp_vect[(v+2)%3].x                     \n"
    "               ) / length(temp_vect[v]);                                               \n"
    "               float width = 0.6+dot(patch_dist[(v+1)%3], patch_dist[(v+2)%3])*1.4;    \n"
    "               f_edge = vec3(0, 0, 0);                                                 \n"
    "               f_edge[v] = dist/width;                                                 \n"
    "               f_color = g_color[v];                                                   \n"
    "               gl_Position = gl_in[v].gl_Position;                                     \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 420                                                                           \n"
    "in vec3 f_edge;                                                                        \n"
    "in vec3 f_color;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float edge_dist = min(min(f_edge.x, f_edge.y), f_edge.z);                       \n"
    "       float edge_alpha = exp2(-pow(edge_dist, 2.0));                                  \n"
    "       vec3 edge_color = vec3(0.0, 0.0, 0.0);                                          \n"
    "       final_color.rgb = mix(f_color, edge_color, edge_alpha);                         \n"
    "       final_color.a = 1.0;                                                            \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Vec2f u_viewport_dimensions;
	float u_step;
	uint32_t u_inst_count;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen",           &u_viewsceen          },
		        {"u_worldview",           &u_worldview          },
		        {"u_viewport_dimensions", &u_viewport_dimensions},
		        {"u_step",                &u_step               },
		        {"u_inst_count",          &u_inst_count         },
		};
	}
	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;
	Uniforms m_unifs;
	uint32_t m_segments = 8;
	uint32_t m_instances = 24;
	float m_step = 10.0;

	App(const char *name) : SpuPage(name, true, {0.9, 0.9, 0.9, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"vert", c_vert},
                        {"tese", c_tese},
                        {"tesc", c_tesc},
		        {"geom", c_geom},
                        {"frag", c_frag},
		};
		shapes::loadShader(m_shader, shader_attrs, Attrs(m_unifs));

		auto r_min = 2.0f;
		auto r_max = 4.5f;

		auto seg_step = 1.0f / m_segments;

		std::vector<vec3f_t> pos_data(size_t(m_segments) * 4, vec3f_t(0));
		std::vector<vec3f_t> pcp_data(size_t(m_segments) * 4, vec3f_t(0));
		std::vector<vec3f_t> ncp_data(size_t(m_segments) * 4, vec3f_t(0));
		std::vector<vec2f_t> ptl_data(size_t(m_segments) * 4, vec2f_t(0));

		float kpa = (4.0 / m_segments) * (4.0 / 3.0) * (sqrt(2.0) - 1.0) / sqrt(2.0);

		for (auto i = 0u; i < m_segments * 4; i += 4) {
			auto s = i / 4;  // need organize
			auto a0 = ((s - 1) * seg_step) * math::two_pi();
			auto a1 = ((s + 0) * seg_step) * math::two_pi();
			auto a2 = ((s + 1) * seg_step) * math::two_pi();
			auto a3 = ((s + 2) * seg_step) * math::two_pi();

			auto min_sa0 = r_min * sinf(a0);
			auto min_sa1 = r_min * sinf(a1);
			auto min_sa2 = r_min * sinf(a2);
			auto min_sa3 = r_min * sinf(a3);

			auto min_ca0 = r_min * cosf(a0);
			auto min_ca1 = r_min * cosf(a1);
			auto min_ca2 = r_min * cosf(a2);
			auto min_ca3 = r_min * cosf(a3);

			auto max_sa0 = r_max * sinf(a0);
			auto max_sa1 = r_max * sinf(a1);
			auto max_sa2 = r_max * sinf(a2);
			auto max_sa3 = r_max * sinf(a3);

			auto max_ca0 = r_max * cosf(a0);
			auto max_ca1 = r_max * cosf(a1);
			auto max_ca2 = r_max * cosf(a2);
			auto max_ca3 = r_max * cosf(a3);

			pos_data[i + 0] = {+min_ca1, 0.0, -min_sa1};
			pos_data[i + 2] = {+max_ca2, 0.0, -max_sa2};
			pos_data[i + 1] = {+max_ca1, 0.0, -max_sa1};
			pos_data[i + 3] = {+min_ca2, 0.0, -min_sa2};

			pcp_data[i + 0] = {(+min_ca2 - min_ca0) * kpa, 0.0f, (-min_sa2 + min_sa0) * kpa};
			pcp_data[i + 2] = {(+max_ca1 - max_ca3) * kpa, 0.0f, (-max_sa1 + max_sa3) * kpa};
			pcp_data[i + 1] = {(+min_ca1 - max_ca1) / 3.0f, 0.0f, (-min_sa1 + max_sa1) / 3.0f};
			pcp_data[i + 3] = {(+max_ca2 - min_ca2) / 3.0f, 0.0f, (-max_sa2 + min_sa2) / 3.0f};

			ncp_data[i + 0] = {(+max_ca1 - min_ca1) / 3.0f, 0.0f, (-max_sa1 + min_sa1) / 3.0f};
			ncp_data[i + 2] = {(+min_ca2 - max_ca2) / 3.0f, 0.0f, (-min_sa2 + max_sa2) / 3.0f};
			ncp_data[i + 1] = {(+max_ca2 - max_ca0) * kpa, 0.0f, (-max_sa2 + max_sa0) * kpa};
			ncp_data[i + 3] = {(+min_ca1 - min_ca3) * kpa, 0.0f, (-min_sa1 + min_sa3) * kpa};

			ptl_data[i + 0] = {3.0f, 3.0f};
			ptl_data[i + 2] = {3.0f, 3.0f};
			ptl_data[i + 1] = {20.0f * r_max / r_min, 10.0f * r_max / r_min};
			ptl_data[i + 3] = {20.0f, 10.0f};
		}

		Attrs array_attrs0 = {
		        {"shader_id",    m_shader.id()  },
		        {"a.a_position", 3              },
		        {"data",         pos_data.data()},
		        {"nelem",        pos_data.size()},
		};
		Attrs array_attrs1 = {
		        {"a.a_prev_cp", 3              },
		        {"data",        pcp_data.data()},
		        {"nelem",       pcp_data.size()},
		};
		Attrs array_attrs2 = {
		        {"a.a_next_cp", 3              },
		        {"data",        ncp_data.data()},
		        {"nelem",       ncp_data.size()},
		};
		Attrs array_attrs3 = {
		        {"a.a_tess_level", 2              },
		        {"data",           ptl_data.data()},
		        {"nelem",          ptl_data.size()},
		};

		m_array.init(array_attrs0);
		m_array.aux(array_attrs1, 1);
		m_array.aux(array_attrs2, 2);
		m_array.aux(array_attrs3, 3);

		m_unifs.u_step = m_step;
		m_unifs.u_inst_count = m_instances;

		m_array.set("patch_vertices", 4);
	}

	void render() override
	{
		m_unifs.u_viewsceen = math::perspective(viewport(0), 70, 1, 200);
		m_unifs.u_viewport_dimensions = Vec2f(viewport(0).sx, viewport(0).sy);

		auto esec = getSeconds().current();
		auto a = pow(sin(esec / 51.7 * math::two_pi()), 3.0);
		auto b = sin(esec / 31.1 * math::two_pi());

		m_unifs.u_worldview = math::lookat(
		        Vec3f(0.55 * a * m_instances * m_step, m_step * 0.5 + pow(b, 4) * 80.0,
		              sin(esec / 7.1 * math::two_pi()) * m_step * (1.0 - pow(b, 2) * 0.9)),
		        Vec3f(0.45 * a * m_instances * m_step, -m_step + pow(b, 2) * 30.0, 0.0), ey());

		m_shader.use();
		m_array.draw(GL_PATCHES, 0, m_segments * 4, m_instances);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("027_tessellation");
}  // namespace
}  // namespace spu::oglplus
