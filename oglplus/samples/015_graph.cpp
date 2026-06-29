//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_viewsceen * u_worldview * a_position;                           \n"
    "       gl_PointSize = 4.0 * gl_Position.w / gl_Position.z;                             \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(0.0, 0.0, 0.0, 1.0);                                         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	static constexpr auto c_node_count = size_t(512);

	CubicBezierLoop<Vec3f, double> m_camPath;
	CubicBezierLoop<Vec3f, double> m_tgtPath;

	SpuShader m_shader;
	SpuArray m_array;
	uint32_t m_edgeCount = 0;

	Mat4f u_viewsceen;
	Mat4f u_worldview;

	float nrand() { return 2.0 * (frand() - 0.5); }

	std::vector<Vec3f> makeCamPathCps()
	{
		return {
		        {-40.0, -50.0, -50.0},
                        {40.0,  0.0,   -60.0},
                        {60.0,  30.0,  50.0 },
		        {-20.0, 50.0,  55.0 },
                        {-30.0, 30.0,  0.0  },
                        {-60.0, 4.0,   -30.0}
                };
	}

	std::vector<Vec3f> makeTgtPathCps()
	{
		return {
		        {-10.0, 0.0,  -10.0},
                        {10.0,  10.0, -10.0},
                        {10.0,  0.0,  10.0 },
		        {-10.0, -5.0, 15.0 },
                        {-10.0, -3.0, 0.0  },
                        {-10.0, 0.0,  -10.0}
                };
	}

	App(const char *name) : SpuPage(name, true, {0.9, 0.9, 0.8, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_camPath.init(makeCamPathCps());
		m_tgtPath.init(makeTgtPathCps());

		srand(0);
		{
			Attrs shader_attrs = {
			        {"frag", c_frag},
			        {"vert", c_vert},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen},
			        {"u_worldview", &u_worldview},
			};
			shapes::loadShader(m_shader, shader_attrs, unif_attrs);
		}

		std::vector<vec3f_t> positions(c_node_count);
		{
			for (auto p = 0u; p != c_node_count; ++p) {
				positions[p].x = nrand() * 120.0;
				positions[p].y = nrand() * 5.0;
				positions[p].z = nrand() * 120.0;
			}

			Attrs vert_attrs = {
			        {"shader_id",    m_shader.id()   },
			        {"data",         positions.data()},
			        {"nelem",        positions.size()},
			        {"a.a_position", 3               },
			};
			m_array.init(vert_attrs);
		}
		{
			std::vector<uint32_t> edge_data;
			edge_data.reserve(c_node_count * 6);
			for (auto i = 0u; i != c_node_count; ++i) {
				auto pi = Vec3f(positions[i]);
				auto min_dist = 1000.0f;
				auto m = i;
				for (auto j = i + 1; j != c_node_count; ++j) {
					auto pj = Vec3f(positions[j]);
					auto dist = length(pi - pj);

					if (min_dist > 1.0 && min_dist > dist) {
						min_dist = dist;
						m = j;
					}
				}
				min_dist *= 2.0;
				auto done = 0u;
				for (auto j = i + 1; j != c_node_count; ++j) {
					auto pj = Vec3f(positions[j]);
					auto dist = length(pi - pj);

					if (min_dist > dist) {
						float x = dist / min_dist;
						if (powf(nrand(), 2.0f) >= x) {
							edge_data.push_back(i);
							edge_data.push_back(j);
							++done;
						}
					}
				}
				if (done == 0) {
					if (i != m) {
						edge_data.push_back(i);
						edge_data.push_back(m);
					}
				}
			}
			m_edgeCount = edge_data.size();
			m_array.send(edge_data, -1, 4);
		}
		positions.clear();

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		// renderstate.flags.line_smooth = true;
		renderstate.flags.program_point_size = true;
		renderstate.flags.blend = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto eye = Vec3f(m_camPath.position(esec / 9.0));
		auto dir = Vec3f(m_tgtPath.position(esec / 7.0) - eye);
		auto up = ey();

		u_viewsceen = math::perspective(viewport(0), 78, 1, 200);
		u_worldview = math::worldview(eye, dir, up);

		m_shader.use();
		m_array.draw(GL_POINTS, 0, c_node_count, 1, GL_ARRAY_BUFFER);
		m_array.draw(GL_LINES, 0, m_edgeCount, 1, GL_ELEMENT_ARRAY_BUFFER);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("015_graph");
}  // namespace
}  // namespace spu::oglplus
