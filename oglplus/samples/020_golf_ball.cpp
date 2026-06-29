//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <cmath>
#include <shapes/sphere.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_comp = {
    "#version 440										                                                                    \n"
    "layout(local_size_x = 10, local_size_y = 1, local_size_z = 1) in;				                     \n"
    "readonly buffer a_input { vec4 v[]; } b_input;						                                      \n"
    "writeonly buffer a_output { vec4 v[]; } b_output;						                                   \n"
    "uniform mat4 u_worldview;                                                                 \n"
    "uniform mat4 u_nodeworld;                                                                 \n"
    "uniform float u_diameter;									                                                        \n"
    "void main()										                                                                     \n"
    "{												                                                                             \n"
    "       uint index = gl_GlobalInvocationID.x;						                                        \n"
    "       vec4 v0 = b_input.v[index];								                                                \n"
    "       b_output.v[index] =	(u_worldview * u_nodeworld) * (v0 * (1.0 + 0.5 * u_diameter));	\n"
    "}												                                                                             \n"
};
	
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen,                                                              \n"
    "u_worldview, u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "const vec3 u_light_pos = vec3(2.0, 3.0, 3.0);                                          \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_normal = mat3(u_nodeworld)*a_normal;                                          \n"
    "       f_light = u_light_pos-gl_Position.xyz;                                          \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 440						                                         \n"
    "in vec3 f_normal;                                          \n"
    "in vec3 f_light;                                           \n"
    "out vec4 final_color;                                      \n"
    "const int hole_count = 50;                                 \n"
    "uniform vec4 u_holes[50];                                  \n"
    "layout (std140) uniform UB_HOLES { vec4 v[50]; } ub_holes; \n"
    "uniform float u_diameter;                                  \n"
    "void main()                                                \n"
    "{                                                          \n"
    "       int imax = 0;                                       \n"
    "       float dmax = -1.0;                                  \n"
    "       for (int i=0; i!=hole_count; ++i)                   \n"
    "       {                                                   \n"
    "               float d = dot(f_normal, ub_holes.v[i].xyz); \n"
    "               if (dmax < d)                               \n"
    "               {                                           \n"
    "                       dmax = d;                           \n"
    "                       imax = i;                           \n"
    "               }                                           \n"
    "       }                                                   \n"
    "       float l = length(f_light);                          \n"
    "       vec3 diff = ub_holes.v[imax].xyz - f_normal;        \n"
    "       vec3 normal =                                       \n"
    "               length(diff) > u_diameter?                  \n"
    "               f_normal:                                   \n"
    "               normalize(diff+f_normal*u_diameter);        \n"
    "       float i = (l > 0.0) ? dot(                          \n"
    "               normal,                                     \n"
    "               normalize(f_light)                          \n"
    "       ) / l : 0.0;                                        \n"
    "       i = 0.2+max(i*2.5, 0.0);                            \n"
    "       final_color = vec4(i, i, i, 1.0);                   \n"
    "}                                                          \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	static constexpr auto c_hole_count = 50u;
	static constexpr auto c_hole_diameter = 0.30;

	SpuComputeArray m_compArray;
	shapes::Array m_array;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec4f u_holes[50];
	Vec4f ub_holes[50];
	float u_diameter;

	void makeHoles(std::vector<Vec4f> &holes, uint32_t hole_count)
	{
		const auto ne = 5u;
		const float el[ne] = {0.50, 0.33, 0.21, 0.11, 0.07};
		const uint32_t ea[ne] = {1, 6, 6, 6, 6};
		const float ao[ne] = {0.00, 0.00, 0.50, -0.08, 0.42};
		const float si[2] = {1.0, -1.0};
		auto k = 0u;

		if (ne != 0) {
			auto hn = size_t(0);
			for (auto e: ea) {
				hn += e;
			}
			assert(hn * 2 == hole_count);
			holes.resize(hn * 2);
		}
		for (auto s: si) {
			for (auto e = 0; e != ne; ++e) {
				auto na = ea[e];
				if (na == 1) {
					holes[k].x = 0.0;
					holes[k].y = s;
					holes[k].z = 0.0;
					k++;
				}
				else if (na > 1) {
					auto elev = el[e] * pi();
					auto a_step = 1.0 / na;
					for (auto a = 0u; a != na; ++a) {
						float azim = s * ao[e] + a * a_step * pi() * 2.0f;
						holes[k].x = cosf(elev) * cosf(azim);
						holes[k].y = sinf(elev * s);
						holes[k].z = cosf(elev) * sinf(azim);
						holes[k].w = 0;  // not '1'
						k++;
					}
				}
			}
		}
	}

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.8, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		std::vector<Vec4f> holes;
		makeHoles(holes, c_hole_count);

		// draw
		uint32_t ubo_holes_buffer_id = 0;
		{
			Attrs shader_attrs = {
			        {"frag", c_frag},
			        {"vert", c_vert},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen},
                                {"u_worldview", &u_worldview},
			        {"u_nodeworld", &u_nodeworld},
                                {"u_diameter",  &u_diameter },
			        {"u_holes",     &u_holes    },
                                {"ub_holes",    &ub_holes   },
			};
			m_array.initShader(shader_attrs, unif_attrs);
			u_diameter = c_hole_diameter;

			shapes::Sphere make_sphere;
			m_array.initArray(make_sphere, {"position", "normal"});

			m_array.getAShader().get("UB_HOLES.buffer_id", &ubo_holes_buffer_id);
		}

		// compute
		{
			Attrs shader_attrs = {
			        {"comp", c_comp},
			};

			Attrs unif_attrs = {
			        {"u_worldview", &u_worldview},
			        {"u_nodeworld", &u_nodeworld},
			        {"u_diameter",  &u_diameter },
			};
			auto &shader = m_compArray.getShader();
			shapes::loadShader(shader, shader_attrs, unif_attrs);
			u_diameter = c_hole_diameter;

			Attrs attrs0 = {
			        {"shader_id", shader.id()         },
			        {"data",      holes.data()        },
			        {"nelem",     holes.size()        },
			        {"a.a_input", sizeof(holes[0]) / 4},
			};
			m_compArray.aux(attrs0, 0);

			Attrs attrs1 = {
			        {"shader_id",  shader.id()         },
			        {"buffer_id",  ubo_holes_buffer_id },
			        {"data",       holes.data()        },
			        {"nelem",      holes.size()        },
			        {"a.a_output", sizeof(holes[0]) / 4},
			};
			m_compArray.aux(attrs1, 1);
			m_compArray.getDim().x = c_hole_count / 10;  // assume local_x = 10
		}

		{
			auto bgcolor = Vec4f(0.10, 0.05, 0.10, 0.0);
			this->set(Attrs({Attr("bgcolor0", bgcolor)}));
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 75, 1, 20);
		u_worldview = Mat4f::orbiting(ezero(), esec, 4.5, 0, 0, 0, 7.2, 0, 70, 15);
		u_nodeworld = math::unit().trans({0.0f, sqrtf(1.0 + sinf(esec / 2.0f * math::two_pi())), 0.0f})
		            * math::unit().rot("x", -esec * math::two_pi() * 0.5);

		m_compArray.compute();
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("020_golf_ball");
}  // namespace
}  // namespace spu::oglplus
