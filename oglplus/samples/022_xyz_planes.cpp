//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/plane.hpp>
#include <shapes/torus.hpp>
#include <shapes/torus.hpp>
#include "replace_text.h"

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_torus_vert_tail = {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform float u_clip_sign[plane_count];                                                \n"
    "uniform vec4 u_clip_plane[plane_count];                                                \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "       gl_Position =                                                                   \n"
    "               u_nodeworld *                                                           \n"
    "               a_position;                                                             \n"
    "       for (int p=0; p!=plane_count; ++p) {                                            \n"
    "               gl_ClipDistance[p] =                                                    \n"
    "                       u_clip_sign[p]*                                                 \n"
    "                       dot(u_clip_plane[p], gl_Position);                              \n"
    "       }                                                                               \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               gl_Position;                                                            \n"
    "}                                                                                      \n"
};

const char *c_torus_frag =  {
    "#version 330                                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float i = (                                                                     \n"
    "               int(f_texcoord.x*36) % 2+                                               \n"
    "               int(f_texcoord.y*24) % 2                                                \n"
    "       ) % 2;                                                                          \n"
    "       final_color = vec4(1-i/2, 1-i/2, 1-i/2, 1.0);                                   \n"
    "}                                                                                      \n"
};

const char *c_plane_vert_tail = {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform float u_clip_sign[plane_count];                                                \n"
    "uniform vec4 u_clip_plane[plane_count];                                                \n"
    "uniform vec3 u_normal;                                                                 \n"
    "in vec4 a_position;                                                                    \n"
    "out vec3 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       for (int p=0; p!=plane_count; ++p) {                                            \n"
    "               gl_ClipDistance[p] =                                                    \n"
    "                       u_clip_sign[p] *                                                \n"
    "                       dot(u_clip_plane[p], gl_Position);                              \n"
    "       }                                                                               \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen * u_worldview * gl_Position;                                \n"
    "       f_color = normalize(                                                            \n"
    "               abs(u_normal) +                                                         \n"
    "               0.4*a_position.xyz                                                      \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_plane_frag = {
    "#version 330                                                                           \n"
    "in vec3 f_color;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(f_color, 0.7);                                               \n"
    "}                                                                                      \n"
};

class App : public SpuPage {
public:
	shapes::Torus m_torusBuilder;
	std::vector<shapes::Plane> m_planeBuilder;
	const int32_t c_plane_size = 3;
	const Attrs replace_attrs = {
		{"plane_count", 3},
	};

	std::vector<shapes::Plane> makePlaneBuilders()
	{
		std::vector<shapes::Plane> result;
		result.emplace_back(Vec3f(0, 2, 0), Vec3f(0, 0, 2));
		result.emplace_back(Vec3f(2, 0, 0), Vec3f(0, 0, 2));
		result.emplace_back(Vec3f(2, 0, 0), Vec3f(0, 2, 0));
		return result;
	}

	shapes::Array m_torusArray;

	SpuShader m_planeShader;
	SpuArray m_planeArrays[8];  // must be larger than c_plane_size!!

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec4f u_clip_plane[8];
	float u_clip_sign[8];
	Vec3f u_normal;

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_torusBuilder = shapes::Torus(1.0, 0.5, 36, 24);
		m_planeBuilder = makePlaneBuilders();

		{
			Attrs shader_attrs = {
			        {"vert", replaceText(c_torus_vert_tail, replace_attrs)},
			        {"frag", c_torus_frag},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen",  &u_viewsceen    },
                                {"u_worldview",  &u_worldview    },
			        {"u_nodeworld",  &u_nodeworld    },
                                {"u_clip_sign",  &u_clip_sign[0] },
			        {"u_clip_plane", &u_clip_plane[0]},
			};
			m_torusArray.initShader(shader_attrs, unif_attrs);
			m_torusArray.initArray(m_torusBuilder, {"position", "texcoord"});
		}

		{
			Attrs shader_attrs = {
			        {"vert", replaceText(c_plane_vert_tail, replace_attrs)},
			        {"frag", c_plane_frag},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen",  &u_viewsceen    },
                                {"u_worldview",  &u_worldview    },
			        {"u_clip_sign",  &u_clip_sign[0] },
                                {"u_clip_plane", &u_clip_plane[0]},
			        {"u_normal",     &u_normal       },
			};
			shapes::loadShader(m_planeShader, shader_attrs, unif_attrs);
		}
		{
			for (auto p = 0; p != c_plane_size; ++p) {
				std::vector<float> data;
				uint32_t n = m_planeBuilder[p].positions(data);

				Attrs attrs = {
				        {"shader_id",    m_planeShader.id()},
				        {"data",         &data[0]          },
				        {"nelem",        data.size() / n   },
				        {"a.a_position", n                 },
				};
				auto index = m_planeBuilder[p].indices();
				m_planeArrays[p].init(attrs);
				m_planeArrays[p].send(index, -1, sizeof(index[0]));

				Attrs array_attrs = {
				        {"restart", m_planeBuilder[p].restartIndex()},
				};
				m_planeArrays[p].set(array_attrs);

				auto eq = m_planeBuilder[p].equation();

				// eq.report("eq");
				u_clip_plane[p] = eq;
			}
			// printf("\n");
		}

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.blend = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
		// renderstate.use();
	}

	void renderTorus() { m_torusArray.draw(nullptr); }

	void renderPlane(size_t p)
	{
		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.blend = true;
		renderstate.use();

		u_normal = m_planeBuilder[p].normal();
		m_planeShader.use();
		m_planeArrays[p].draw(GL_TRIANGLE_STRIP);

		renderstate.flags.blend = false;
		renderstate.use();
	}

	void setClipDistance(int32_t p, bool value)
	{
		auto &renderstate = SpuPage::getRenderstate();

		switch (p) {
		case 0: renderstate.flags.clip_distance0 = value; break;
		case 1: renderstate.flags.clip_distance1 = value; break;
		case 2: renderstate.flags.clip_distance2 = value; break;
		default: assert(0);
		};
		renderstate.use();
	}

	void bsp(const Mat4f &worldview, int32_t p)
	{
		Vec4f normal = {m_planeBuilder[p].normal(), 0.0};
		float sign = ((worldview * normal).z >= 0.0) ? 1.0 : -1.0;
		bool at_leaf = p + 1 == c_plane_size;

		setClipDistance(p, true);

		u_clip_sign[p] = -sign;

		if (at_leaf) {
			renderTorus();
		}
		else {
			bsp(worldview, p + 1);
		}

		setClipDistance(p, false);

		renderPlane(p);

		setClipDistance(p, true);

		u_clip_sign[p] = +sign;

		if (at_leaf) {
			renderTorus();
		}
		else {
			bsp(worldview, p + 1);
		}

		setClipDistance(p, false);
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 30);
		u_worldview = Mat4f::orbiting(ezero(), esec, 6, 0, 0, 0, 10, 45, 30, 7);
		u_nodeworld = math::unit().rot("x", -esec / 12.0 * math::two_pi());

		bsp(u_worldview, 0);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("022_xyz_planes");
}  // namespace
}  // namespace spu::oglplus
